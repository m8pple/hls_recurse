#ifndef sum_indexed_hpp
#define sum_indexed_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

#include "hls_recurse/examples/sum.hpp"

namespace hls_recurse
{

void r_sum_indexed(uint32_t n, int32_t *array)
{
	// TODO : This isn't really the same...

	if( n<=1 ){
		return;
	}else{
		r_sum_indexed(n/2, array);
		r_sum_indexed(n-(n/2), array+n/2);
		array[0] += array[n/2];
	}
}

void man_sum_indexed(uint32_t n, int32_t *f)
{
    const unsigned DEPTH=512;

    int sp=0;
    int stack_n[DEPTH];
    int32_t stack_index[DEPTH];
    int stack_state[DEPTH];

    stack_n[0]=n;
    stack_index[0]=0;
    stack_state[0]=0;

    while(1){
        n=stack_n[sp];
        int index=stack_index[sp];
        int state=stack_state[sp];

        if(state==0){
            if(n<=1){
                if(sp==0){
                    break;
                }else{
                    sp--;
                }
            }else{
                stack_state[sp]=1;
                sp++;
                stack_state[sp]=0;
                stack_n[sp]=n/2;
                stack_index[sp]=index;
            }
        }else if(state==1){
            stack_state[sp]=2;
            sp++;
            stack_state[sp]=0;
            stack_n[sp]=n-n/2;
            stack_index[sp]=index+n/2;
        }else if(state==2){
            f[index]+=f[index+n/2];
            if(sp==0){
                break;
            }else{
                sp--;
            }
        }
    }
};

void f_sum_indexed(uint32_t _n, int32_t *array)
{

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

	typedef v0::CallStack<void, uint32_t, int> call_stack_t;

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


void f2_sum_indexed(uint32_t _n, int32_t *array)
{
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

int32_t *g_array;

void f3_sum_indexed(uint32_t _index, uint32_t _n)
{
	uint32_t n=_n;
	uint32_t index=_index;

	run_function_old<void>(
		IfElse([&](){ return n<=1; },
			Return(),
			Sequence(
				Recurse([&](){ return make_hls_state_tuple<uint32_t,uint32_t>(n/2, index); }),
				Recurse([&](){ return make_hls_state_tuple<uint32_t,uint32_t>(n-(n/2), index+n/2); }),
				[&](){  g_array[index] += g_array[index+n/2];  }
			)
		),
		n, index
	);
}


template<class T>
bool test_sum_indexed(T sum_indexed, bool logEvents=false)
{
    int32_t x[4096];

    bool ok=true;

    for(unsigned n=2; n<=4096; n*=2){

        int32_t fx=0;
        for(unsigned i=0;i<n;i++){
            x[i]=fx;
            fx=fx+1;
        }

        if(logEvents){
            printf("sum, n=%u, start\n", n);
        }
        sum_indexed(n,x);
        if(logEvents){
            printf("sum, n=%u, start\n", n);
        }

        ok = ok && ( x[0]==n*(n-1)/2 );
    }

    return ok;
}

template<class T>
bool harness_sum_indexed(T sum)
{
    return harness_sum(sum);
}

}; // namespace hls_recurse

#endif
