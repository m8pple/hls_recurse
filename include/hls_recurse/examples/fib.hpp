#ifndef fib_hpp
#define fib_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

namespace hls_recurse
{

uint32_t r_fib(uint32_t n)
{
    if(n<=2){
        return 1;
    }else{
        return r_fib(n-1)+r_fib(n-2);
    }
}

uint32_t man_fib(uint32_t n)
{
    const unsigned DEPTH=512;

    unsigned sp=0;
    uint32_t stack_n[DEPTH];
    uint32_t stack_r1[DEPTH];
    int stack_state[DEPTH];
    uint32_t retval;

    stack_n[sp]=n;
    stack_state[sp]=0;

    while(1){
        int state=stack_state[sp];
        uint32_t n=stack_n[sp];

        if(state==0){
            if(n<=2){
                //  return 1;
                retval=1;
                if(sp==0){
                    break;
                }else{
                    sp--;
                }
            }else{
                // call fib(n-1);
                stack_state[sp]=1;
                sp++;
                stack_state[sp]=0;
                stack_n[sp]=n-1;
            }
        }else if(state==1){
            // completed r_fib(n-1)
            stack_r1[sp]=retval;
            // call fib(n-2);
            stack_state[sp]=2;
            sp++;
            stack_state[sp]=0;
            stack_n[sp]=n-2;
        }else if(state==2){
            // completed r_fib(n-2)
            retval=stack_r1[sp]+retval;
            // return
            if(sp==0){
                break;
            }else{
                sp--;
            }
        }
    }
    return retval;
}


class Fib
    : public Function<Fib,uint32_t, uint32_t,uint32_t,uint32_t>
{
private:
    uint32_t n;
    uint32_t f_n_1, f_n_2;
public:
    Fib(stack_entry_t *stack)
        : function_base_t(*this, stack, n, f_n_1, f_n_2)
    {}

    template<class TVisitor>
    void VisitBody(TVisitor visitor)
    {
        visitor(
            IfElse([&](){ return n<=2; },
                Return([](){ return 1; }),
                Sequence(
                    RecurseWithResult(f_n_1, [&](){ return state_t(n-1,0,0); }),
                    RecurseWithResult(f_n_2, [&](){ return state_t(n-2,0,0); }),
                    Return([&](){ return f_n_1+f_n_2; })
                )
            )
        );
    }


};

uint32_t f_fib(uint32_t n)
{
    Fib::stack_entry_t stack[1024];
    Fib fib(stack);

    return fib(n,0,0);
}

extern "C" uint32_t __attribute__((noinline)) f2_fib(uint32_t n)
{
    uint32_t f_n_1, f_n_2;

    return run_function_old<uint32_t>(
        IfElse([&](){ return n<=2; },
            Return([](){ return 1; }),
            Sequence(
                RecurseWithResult(f_n_1, [&](){ return make_hls_state_tuple(n-1); }),
                RecurseWithResult(f_n_2, [&](){ return make_hls_state_tuple(n-2); }),
                Return([&](){ return f_n_1+f_n_2; })
            )
        ),
        n, f_n_1, f_n_2
    );
}

template<class T>
bool test_fib(T fib, bool logEvents=false)
{
    uint32_t aRef[]={0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584};
    uint32_t nRef=sizeof(aRef)/sizeof(aRef[0]);

    int failed=0;
    for(unsigned i=1; i<nRef; i++){
        if(logEvents){
            printf("fib, n=%u, start\n", i);
        }
        uint32_t got=fib(i);
        if(logEvents){
            printf("fib, n=%u, finish\n", i);
        }
        if(got!=aRef[i]){
            failed++;
        }
    }

    return failed==0;
}

template<class T>
int harness_fib(T fib)
{
    return fib(64);
}

}; // hls_recurse

#endif
