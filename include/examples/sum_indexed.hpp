#ifndef sum_indexed_hpp
#define sum_indexed_hpp

#include "state_machine_self_recursion.hpp"
#include <stdint.h>

void r_sum_indexed(uint32_t n, float *array)
{
	#pragma HLS INTERFACE m_axi depth=1024 port=array

	// TODO : This isn't really the same...

	if( n<=1 ){
		return;
	}else{
		r_sum_indexed(n/2, array);
		r_sum_indexed(n-(n/2), array+n/2);
		array[0] += array[n/2];
	}
}

void f_sum_indexed(uint32_t _n, float *array)
{
	#pragma HLS INTERFACE m_axi depth=1024 port=array

	uint32_t n;
	int index;

	auto body = IfElse([&](){ return n<=1; },
		Return(),
		Sequence(
			Recurse([&](){ return make_hls_state_tuple<uint32_t,int>(n/2, index); }),
			Recurse([&](){ return make_hls_state_tuple<uint32_t,int>(n-(n/2), index+n/2); }),
			[&](){  array[index] += array[index+n/2];  }
		)
	);

	typedef CallStack<void, uint32_t, int> call_stack_t;

	call_stack_t::stack_entry_t stack[64];

	call_stack_t call_stack(stack, n, index);

	unsigned length=body.total_state_count;

	call_stack.SetContext(make_hls_state_tuple<uint32_t,int>(_n, 0));

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

}


void f2_sum_indexed(uint32_t _n, float *array)
{
	#pragma HLS INTERFACE m_axi depth=1024 port=array

	uint32_t n=_n;
	int index=0;

	run_function_old<void>(
		IfElse([&](){ return n<=1; },
			Return(),
			Sequence(
				Recurse([&](){ return make_hls_state_tuple<uint32_t,int>(n/2, index); }),
				Recurse([&](){ return make_hls_state_tuple<uint32_t,int>(n-(n/2), index+n/2); }),
				[&](){  array[index] += array[index+n/2];  }
			)
		),
		n, index
	);
}


template<class T>
bool test_sum_indexed(T sum_indexed)
{
    const uint32_t n=100;
    float x[n];

    // Using the floating-point counter to avoid a problem where
    // legup 4.0 says:
    //   Unrecognized Instruction:   %3 = uitofp i32 %2 to float
    float fx=0;
    for(unsigned i=0;i<n;i++){
        x[i]=fx;
        fx=fx+1;
    }

    sum_indexed(n,x);

    return x[0]==n*(n-1)/2;
}

#endif
