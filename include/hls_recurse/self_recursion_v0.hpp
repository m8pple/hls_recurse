#ifndef state_machine_self_recursion_v0_hpp
#define state_machine_self_recursion_v0_hpp

#include "hls_recurse/self_recursion_steps.hpp"

namespace hls_recurse
{

namespace v0
{

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

#ifndef __SYNTHESIS__
    std::ostream &dump(std::ostream &dst)
    {
        dst<<"Stack ptr = "<<m_stackPtr<<"\n";
        dst<<" state    = "; m_state.dump(dst)<<"\n";
        return dst;
    }
#endif
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

}; // v0

template<class TImpl, class TRet, class ...TState>
class Function
{
    public:
    typedef hls_state_tuple<TState...> state_t;

    typedef generic_context_t< state_t > stack_entry_t;

    typedef Function function_base_t;
private:
    typedef v0::CallStack<TRet,TState...> stack_t;

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

}; // hls_recurse

#endif
