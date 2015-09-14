#ifndef mutual_recursion_v0_hpp
#define mutual_recursion_v0_hpp

#include "hls_recurse/self_recursion_steps.hpp"

namespace v0
{

template<unsigned TId, class TArgSrc>
struct CallStep
    : Step
{
    TArgSrc m_argSrc;

    HLS_INLINE_STEP CallStep(const TArgSrc &args)
        : m_argSrc(args)
    {}

    enum{ total_state_count = 1 };
    
    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        // TContext is actually a multi_stack<...> for the set of
        // active functions
        
        assert(curr==TBase);

        auto args=m_argSrc();
        
        ctxt.PushReturnPoint(curr+1);
        ctxt.PushArguments(args);

        return ctxt.template StartStateOf<TId>();
    }
};




template<class TAllFuncs, class TDoneStacks, class TCallId, class TCallArgSrc>
function_def_by_id<TCallId,TAllFuncs>::return_type
    run_step_impl(
    const TAllFuncs &allDefs,
    const function_def_list<> &todoFuncs,
    TDoneStacks &doneStack,
    const CallStep<TCallId,TCallArgSrc> &call
)
{
    // Base case: all data stacks have  been created, we actually
    // now run the function
    
    const traits_t Traits = (traits_t)(InheritedTrait_SequenceEnd|InheritedTrait_ReturnPosition);
    
    // This is the state after the end of the body, which indicates the
    // entire thing has returned
    const unsigned top_level_return=allDefs.total_state_count;    
    
    // Push the initial call context and set the state to the start
    // of that function
    auto args=call.m_argSrc();
    stack_by_id<TCallId>::PushContext(top_level_return, args);
    unsigned s=allDefs.template StartStateOf<TCallId>();
    
	while(s != top_level_return){
        assert( (s&StateCode_SpecialMask) || (s < top_level_return));
        
  		if(s==StateCode_Return){
            
			if(call_stack.IsEmpty()){
				// end of the state machine, no more context. Return
				break;
			}else{
				// end of the state machine, more context. pop to previous frame
				s=call_stack.PopContext();
			}
        }else if(s&StateCode_SpecialMask){
            // No special codes should bubble up to us here, we only do returns
            // and normal states
            logic_error_if_reachable();
		}else{
            // All function defs have an explicit return in the trailing position,
            // there shouldn't be any implicit returns.
            
			const traits_t Traits = (traits_t)(InheritedTrait_SequenceEnd|InheritedTrait_ReturnPosition);

            s=body.template step<0,Traits,call_stack_t>(s, call_stack);
		}
	}
    
    
    return stack_by_id<TCallId>::GetReturnHolder(doneStack).get();
}

template<class TAllFuncs, class TTodoFuncs, class TDoneStacks, class TCallId, class TCallArgSrc>
function_def_by_id<TCallId,TAllFuncs>::return_type run_step_impl(
    const TAllFuncs &allDefs,
    const TTodoFuncs &todoFuncs,
    TDoneStacks &doneStack,
    const CallStep<TCallId,TCallArgSrc> &call
)
{
    stack_entry_t stackStorage[1024];
    
    multi_stack call_stack(stackStorage, defs.head, tailStack);
    
    return run_step_impl(allDefs, todoFuncs.tail, call_stack, call);
}



template<class TAllFuncs, class TCallId, class TCallArgSrc>
function_def_by_id<TCallId,TAllFuncs>::return_type run_mutual(
    const TAllFuncs &allDefs,
    const CallStep<TCallId,TCallArgSrc> &call
){
    multi_stack<> baseStack;
    
    return run_step_impl(allDefs, allDefs, baseStack, call);
}


#error "None of this works, just sketching out ideas."


#endif
