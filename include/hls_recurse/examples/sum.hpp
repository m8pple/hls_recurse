#ifndef sum_hpp
#define sum_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

namespace hls_recurse
{
    
void r_sum(uint32_t n, int32_t *f)
{
    if(n<=1){
        return;
    }else{
        r_sum(n/2, f);
        r_sum(n-(n/2), f+n/2);
        f[0] += f[n/2];
    }
};


void man_sum(uint32_t n, int32_t *f)
{
    const unsigned DEPTH=512;
    
    int sp=0;
    int stack_n[DEPTH];
    int32_t *stack_f[DEPTH];
    int stack_state[DEPTH];
    
    stack_n[0]=n;
    stack_f[0]=f;
    stack_state[0]=0;
    
    while(1){
        n=stack_n[sp];
        f=stack_f[sp];
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
                stack_f[sp]=f;
            }
        }else if(state==1){
            stack_state[sp]=2;
            sp++;
            stack_state[sp]=0;
            stack_n[sp]=n-n/2;
            stack_f[sp]=f+n/2;
        }else if(state==2){
            f[0]+=f[n/2];
            if(sp==0){
                break;
            }else{
                sp--;
            }
        }
    }
};



class SumV1
    : public Function<SumV1, void, uint32_t,int32_t*>
{
private:
    uint32_t n;
    int32_t *f;
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

void f_sum(uint32_t n, int32_t *f)
{
    SumV1::stack_entry_t stack[64];
    SumV1 sum(stack);

    sum(n,f);
}

void f2_sum(uint32_t n, int32_t *f)
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
bool test_sum(T sum, bool logEvents=false)
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
        sum(n,x);
        if(logEvents){
            printf("sum, n=%u, start\n", n);
        }

        ok = ok && ( x[0]==n*(n-1)/2 );
    }

    return ok;
}

template<class T>
bool harness_sum(T sum)
{
    int32_t x[1024];

    for(int i=0;i<1024;i++){
        x[i]=i;
    }

    sum(1024,x);

    return x[0];
}

}; // hls_recurse

#endif
