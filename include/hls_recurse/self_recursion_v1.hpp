#ifndef state_machine_self_recursion_v1_hpp
#define state_machine_self_recursion_v1_hpp

#include "hls_recurse/self_recursion_steps.hpp"

namespace hls_recurse
{

namespace v1
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
  HLS_INLINE_STEP CallStack(context_t *stack, TStateVars &...stateVars)
    : m_stack(stack)
    , m_stackPtr(0)
    , m_state(stateVars...)
  {}

  HLS_INLINE_STEP CallStack(context_t *stack, const hls_binding_tuple<TStateVars...> &stateVars)
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
#ifndef NDEBUG
        /*
        std::cerr<<"SetContext(";
        src.dump(std::cerr);
        std::cerr<<")\n";
        */
#endif
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
    std::ostream &dump(std::ostream &dst) const
    {
        dst<<"Stack ptr = "<<m_stackPtr<<", ";
        dst<<" state = ";
        m_state.dump(dst);
        dst<<"\n";
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

}; // v1

template<class TRet, class TImpl, class ...TState>
HLS_INLINE_STEP TRet run_function_old(const TImpl &body, TState &...state)
{
    typedef v1::CallStack<TRet, TState...> call_stack_t;

	typename call_stack_t::stack_entry_t stack[512];

	call_stack_t call_stack(stack, state...);

	unsigned length=body.total_state_count;

    call_stack.SetContext(make_hls_state_tuple(state...));

	unsigned s=0;
	run_function_old : while(1){
        /*
        std::cerr<<"FunFunction: Pre-step, s="<<s<<", ";
        call_stack.dump(std::cerr);
        std::cerr<<"\n";
        */

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

}; // hls_recurse

#endif
