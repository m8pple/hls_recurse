
template<class TBindings...>
hls_bindings_tuple<TBindings...> Inputs(TBindings &...bindings)
{
    return hls_bindings_tuple<TBindings...>(...bindings);
}

template<class TBindings...>
hls_bindings_tuple<TBindings...> Locals(TBindings &...locals)
{
    return hls_bindings_tuple<TBindings...>(...locals);
}

template<class TRet, class TImpl, class ...TState>
TRet run_function(
    const TParams &params,
    const TLocals &locals
    const TImpl &...body
)
{
    typedef CallStack<TRet, TState...> call_stack_t;

	typename call_stack_t::stack_entry_t stack[1024];

	call_stack_t call_stack(stack, state...);

	unsigned length=body.total_state_count;

    call_stack.SetContext(make_hls_state_tuple(decay_val(state)...));

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
