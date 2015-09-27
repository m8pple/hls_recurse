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
