#ifndef sum_hpp
#define sum_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

namespace hls_recurse
{

class SumV1
    : public Function<SumV1, void, uint32_t,float*>
{
private:
    uint32_t n;
    float *f;
public:
    SumV1(stack_entry_t *stack)
        : function_base_t(*this, stack, n, f)
    {}

    template<class TVisitor>
    void VisitBody(TVisitor visitor)
    {
        visitor(
            IfElse([&](){ return n<=1; },
                Return(),
                Sequence(
                    Recurse([&](){ return make_hls_state_tuple(n/2, f); }),
                    Recurse([&](){ return make_hls_state_tuple(n-(n/2), f+n/2); }),
                    [&](){  f[0] += f[n/2];  }
                )
            )
        );
    }
};

void f_sum(uint32_t n, float *f)
{
    SumV1::stack_entry_t stack[64];
    SumV1 sum(stack);

    sum(n,f);
}

void f2_sum(uint32_t n, float *f)
{
    run_function_old<void>(
        IfElse([&](){ return n<=1; },
            Return(),
            Sequence(
                Recurse([&](){ return make_hls_state_tuple(n/2, f); }),
                Recurse([&](){ return make_hls_state_tuple(n-(n/2), f+n/2); }),
                [&](){  f[0] += f[n/2];  }
            )
        ),
        n, f
    );
}

template<class T>
bool test_sum(T sum)
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

    f_sum(n,x);

    return x[0]==n*(n-1)/2;
}

}; // hls_recurse

#endif
