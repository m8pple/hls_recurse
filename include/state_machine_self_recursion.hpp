#ifndef state_machine_self_recursion_hpp
#define state_machine_self_recursion_hpp

#include "state_machine_steps.hpp"
#include "state_machine_builder.hpp"

#include <var.hpp>

//#include <utility>
//#include <stack>
#include <stdio.h>
#include <iostream>

#include "hls_tuple.hpp"

template<class TTuple>
struct generic_context_t{
    unsigned first;
    TTuple second;

    HLS_INLINE_STEP generic_context_t()
    {}

    HLS_INLINE_STEP generic_context_t(unsigned _first, const TTuple &_second)
        : first(_first)
        , second(_second)
    {}

};


template<class TRet>
struct return_holder
{
    TRet m_value;

    HLS_INLINE_STEP TRet get()
    {
        return m_value;
    }

    HLS_INLINE_STEP void set(const TRet &val)
    {
        m_value=val;
    }
};

template<>
struct return_holder<void>
{
	HLS_INLINE_STEP void get()
    {}
};


template<class TRet, class ...TStateVars>
struct CallStack
{
    typedef hls_state_tuple<TStateVars...> state_tuple_t;
  typedef generic_context_t<state_tuple_t> context_t;

    typedef generic_context_t<state_tuple_t> stack_entry_t;


    context_t *m_stack;
    unsigned m_stackPtr;

    hls_binding_tuple<TStateVars...> m_state;

    return_holder<TRet> m_return;
public:
  CallStack(context_t *stack, TStateVars &...stateVars)
    : m_stack(stack)
    , m_stackPtr(0)
    , m_state(stateVars...)
  {}

  CallStack(context_t *stack, const hls_binding_tuple<TStateVars...> &stateVars)
    : m_stack(stack)
    , m_stackPtr(0)
    , m_state(stateVars)
  {}

    HLS_INLINE_STEP bool IsEmpty() const
    { return m_stackPtr==0; }

  HLS_INLINE_STEP unsigned Depth() const
  {return m_stackPtr; }

    HLS_INLINE_STEP void PushContext(unsigned state)
    {
        state_tuple_t current=m_state; // Capture values
        m_stack[m_stackPtr]=context_t(state, current);
        ++m_stackPtr;
    }

    template<class TSrc>
    HLS_INLINE_STEP void SetContext(const TSrc &src)
    {
        m_state=src;
    }

    HLS_INLINE_STEP unsigned PopContext()
    {
        assert(!IsEmpty());

        --m_stackPtr;
        m_state=m_stack[m_stackPtr].second;
        unsigned next=m_stack[m_stackPtr].first;
        return next;
    }

    /* This is returned by reference due to having to deal with
        void and non void. Makes it easier to avoid...
    */
    HLS_INLINE_STEP return_holder<TRet> &GetReturnHolder()
    {
        return m_return;
    }
    
    std::ostream &dump(std::ostream &dst)
    {
        dst<<"Stack ptr = "<<m_stackPtr<<"\n";
        dst<<" state    = "; m_state.dump(dst)<<"\n";
        return dst;
    }
};

template<class TRet,class TStateTyple>
class call_stack_for_state_tuple;

template<class TRet,class ...TState>
struct call_stack_for_state_tuple<TRet,hls_state_tuple<TState...> >
{
    typedef CallStack<TRet,TState...> type;
};

template<class TRet,class TStateTyple>
class call_stack_for_binding_tuple;

template<class TRet,class ...TState>
struct call_stack_for_binding_tuple<TRet,hls_binding_tuple<TState...> >
{
    typedef CallStack<TRet,TState...> type;
};



template<class TArgSrc>
struct RecurseStep
    : Step
{
    TArgSrc m_argSrc;

    HLS_INLINE_STEP RecurseStep(const TArgSrc &args)
        : m_argSrc(args)
    {}

    enum{ total_state_count = 1 };
    
    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(curr==TBase);

        auto args=m_argSrc();

        if(TTraits & InheritedTrait_ReturnPosition){
            // Tail recursion position, we simply overwrite the current context,
            // we don't need to put anything on the stack

            // This should be statically visible to the compiler and fully inlined,
            // so either this path or the next will be in the code
            //printf("Tail recursion\n");
        }else{
            //printf("True recursion\n");
            ctxt.PushContext(TBase+1);
        }
        ctxt.SetContext(args);
        return 0; // Back to the beginning of the function
    }
};

template<class TArgSrc>
struct RecurseAndReturnStep
    : Step
{
    TArgSrc m_argSrc;

    HLS_INLINE_STEP RecurseAndReturnStep(const TArgSrc &args)
        : m_argSrc(args)
    {}

    enum{ total_state_count = 1 };
    
    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(curr==TBase);

        auto args=m_argSrc();

        // Recurse and return is explicitly tail recursion, so
        // we simply overwrite the current context, and we don't
        // need to put anything on the stack
        ctxt.SetContext(args);

        return 0; // Back to the beginning of the function
    }
};

template<class TRetDst, class TArgSrc>
struct RecurseWithResultStep
    : Step
{
    TRetDst &m_retDst;
    TArgSrc m_argSrc;

    HLS_INLINE_STEP RecurseWithResultStep(TRetDst &retDst, const TArgSrc &args)
        : m_retDst(retDst)
        , m_argSrc(args)
    {}

    enum{ total_state_count = 2 };
    
    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert( (curr==TBase) || (curr==TBase+1) );

        if(curr==TBase){
            // Kick of the recursive call
            auto args=m_argSrc();
            ctxt.PushContext(TBase+1); // We will resume with the assignment from return
            ctxt.SetContext(args);
            return 0; // Back to the beginning of the function
        }else{
            // Assign the return value to the correct place
            m_retDst=ctxt.GetReturnHolder().get();
            return TBase+2; // And carry on
        }
    }
};

namespace detail{
    template<class TF>
    struct LiftSingleStep<RecurseStep<TF>  >
    {
        typedef RecurseStep<TF> type;

        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };

    template<class TF>
    struct LiftSingleStep<RecurseAndReturnStep<TF>  >
    {
        typedef RecurseAndReturnStep<TF> type;

        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };

    template<class TR, class TF>
    struct LiftSingleStep<RecurseWithResultStep<TR,TF>  >
    {
        typedef RecurseWithResultStep<TR,TF> type;

        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };
};

template<class TArgSrc>
HLS_INLINE_STEP RecurseStep<TArgSrc> Recurse(const TArgSrc &src)
{ return RecurseStep<TArgSrc>(src); }

template<class TArgSrc>
HLS_INLINE_STEP RecurseAndReturnStep<TArgSrc> RecurseAndReturn(const TArgSrc &src)
{ return RecurseAndReturnStep<TArgSrc>(src); }

template<class TRetDst, class TArgSrc>
HLS_INLINE_STEP RecurseWithResultStep<TRetDst, TArgSrc> RecurseWithResult(TRetDst &dst, const TArgSrc &src)
{ return RecurseWithResultStep<TRetDst,TArgSrc>(dst,src); }

/*

template<class TFunction, class TArgs>
void run(
    const TFunction &function,
    const typename TFunction::args_t &args
){
    CallStack<std::pair<unsigned, state_t> > stack;

    unsigned state=0;
    function.bind_call(args); // Set up state from arguments

    while(state<step.total_state_count){
        state=step.step(state);
        if(state==State_Return){
            if(stack.empty())
                break;
            function.bind_state(stack.top().second);

        }
    }
}


uint32_t n;
float *f;

struct F {};
struct N {};

typedef FunctionSig<F,N> sum_sqr_sig_t;

auto x=Function<sum_sqr_sig_t>(



);
*/


template<class TImpl, class TRet, class ...TState>
class Function
{
public:
    typedef hls_state_tuple<TState...> state_t;

    typedef generic_context_t< state_t > stack_entry_t;

    typedef Function function_base_t;
private:
    typedef CallStack<TRet,TState...> stack_t;

    // Is the function currently in the middle of a function call?
    bool m_active;

    TImpl &m_impl;
    stack_t m_stack;
    return_holder<TRet> m_ret;


protected:
    HLS_INLINE_STEP Function(TImpl &impl,
			      stack_entry_t *stack,
			     TState & ...state)
        : m_active(false)
        , m_impl(impl)
        , m_stack(stack, state...)
    {}

    /* Passing the body down to the base class is hard. With
        c++14 auto return types it could be easier, as there could
        be something like GetBody(), then the base class could
        use decltype(TImpl::GetBody()) to declare an instance.

        The entire static type of the body must be visible, and it
        is very difficult/impossible to name because:
        - the templates are deeply nested
        - the lambdas are unnamed

        The body of the expression has no state, and the object
        graph itself is irrelevant and should be compiled away.
        So we isntantiate it locally each time within a function.
        Because we want to do lots of things with it, we pass in
        a templated visitor, which can then access the type or
        the isntance of the body in whatever way it wants.

    */

    struct get_body_length
    {
        unsigned &length;

        HLS_INLINE_STEP get_body_length(unsigned &_length)
            : length(_length)
        {}

        template<class TBody>
        HLS_INLINE_STEP void doIt(TBody &)
        {
            length = TBody::total_state_count;
        }

        /*template<class ...TBody>
        HLS_INLINE_STEP void operator()(const TBody & ...args)
        {
            doIt( detail::LiftSequence<TBody...>::lift(args...) );
        }*/

        template<class ...TBody>
        HLS_INLINE_STEP void operator()(const TBody & ...args)
        {
            length = Sequence( args... ).total_state_count;
        }
    };

    struct step_body
    {
        unsigned &state;
        stack_t &stack;

        HLS_INLINE_STEP step_body(unsigned &_state, stack_t &_stack)
            : state(_state)
            , stack(_stack)
        {}

        template<class ...TBody>
        HLS_INLINE_STEP void operator()(const TBody & ...body)
        {
            const traits_t Traits = (traits_t)(InheritedTrait_SequenceEnd|InheritedTrait_ReturnPosition);

            auto seq=Sequence(body...);

            state=seq.template step<0,Traits,stack_t>(state, stack);
        }
    };
public:
    HLS_INLINE_STEP TRet operator()(const TState &...state)
    {
      //printf("Function::start\n");

        assert(!m_active);
        assert(m_stack.IsEmpty());

        m_active=true;

        unsigned length;
        get_body_length gbl(length);
        m_impl.VisitBody(gbl);

	//printf("Setting initial stack context.\n");
        m_stack.SetContext(make_hls_state_tuple(state...));

	//printf("Beginning loop\n");
        unsigned s=0;
        while(1){
	  //printf("Stack depth : %d\n", m_stack.Depth());
            if(
                (s==length) // implicit return
                ||
                (s==StateCode_Return) // explicit return
            ){
                if(m_stack.IsEmpty()){
                    // end of the state machine, no more context. Return
                    break;
                }else{
                    // end of the state machine, more context. pop to previous frame
                    s=m_stack.PopContext();
                }
            }else{
                step_body sb(s, m_stack);
                m_impl.VisitBody(sb);
            }
        }

        m_active=false;

	//printf("Function::finish\n");
        return m_stack.GetReturnHolder().get();
    }
};



template<class TRet, class TImpl, class ...TState>
TRet run_function_old(const TImpl &body, TState &...state)
{
    typedef CallStack<TRet, TState...> call_stack_t;

	typename call_stack_t::stack_entry_t stack[1024];

	call_stack_t call_stack(stack, state...);

	unsigned length=body.total_state_count;

    call_stack.SetContext(make_hls_state_tuple(state...));

	unsigned s=0;
	while(1){
  		if(
			(s==length) // implicit return
			||
			(s==StateCode_Return) // explicit return
		){
			if(call_stack.IsEmpty()){
				// end of the state machine, no more context. Return
				break;
			}else{
				// end of the state machine, more context. pop to previous frame
				s=call_stack.PopContext();
			}
		}else{
			const traits_t Traits = (traits_t)(InheritedTrait_SequenceEnd|InheritedTrait_ReturnPosition);

            s=body.template step<0,Traits,call_stack_t>(s, call_stack);
		}
	}

    return call_stack.GetReturnHolder().get();
}


#endif
