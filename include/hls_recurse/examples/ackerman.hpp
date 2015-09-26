#ifndef ackerman_hpp
#define ackerman_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

namespace hls_recurse
{

uint32_t r_ackerman(uint32_t m, uint32_t n)
{
    if(m==0){
        return n+1;
    }else if(m>0 && n==0){
        return r_ackerman(m-1, 1);
    }else{
        n=r_ackerman(m,n-1);
        return r_ackerman(m-1,n);
    }
}

class Ackerman
    : public Function<Ackerman,uint32_t, uint32_t,uint32_t>
    {
    private:
        uint32_t m, n;
    public:
        Ackerman(stack_entry_t *stack)
            : function_base_t(*this, stack, m, n)
        {}

        template<class TVisitor>
        void VisitBody(TVisitor visitor)
        {
            visitor(
                //[&](){ printf("  A(%d,%d)\n", m,n); },
                IfElse([&](){ return m==0; },
                    Return([&](){ return n+1; }),
                    IfElse([&](){ return m>0 && n==0; },
                        RecurseAndReturn([&](){ return state_t(m-1,1); }),
                        Sequence(
                            RecurseWithResult(n, [&](){ return state_t(m,n-1); }),
                            RecurseAndReturn([&](){ return state_t(m-1,n); })
                        )
                    )
                )
            );
        }


    };

uint32_t f_ackerman(uint32_t m, uint32_t n)
{
    Ackerman::stack_entry_t *stack=new Ackerman::stack_entry_t[65536];
    Ackerman ackerman(stack);

    return ackerman(m,n);
}

uint32_t f2_ackerman(uint32_t _m, uint32_t _n)
{
    uint32_t m=_m, n=_n;

    return run_function_old<uint32_t>(
        Sequence(
            //[&](){ printf("A(%d,%d)\n", m,n); },
            IfElse([&](){ return m==0; },
                Return([&](){ return n+1; }),
                IfElse([&](){ return m>0 && n==0; },
                    RecurseAndReturn([&](){ return make_hls_state_tuple(m-1,1u); }),
                    Sequence(
                        RecurseWithResult(n, [&](){ return make_hls_state_tuple(m,n-1); }),
                        RecurseAndReturn([&](){ return make_hls_state_tuple(m-1,n); })
                    )
                )
            )
        ),
		m, n
    );
}

template<class T>
bool test_ackerman(T ackerman, bool logEvents=false)
{
    struct{
        int m;
        int n;
        int ref;
    }aTests[]={
        {0,0,1},
        {1,0,2},
        {2,0,3},
        {3,0,5},
        {4,0,13},
        //{5,0,65533},
        {0,1,2},
        {1,1,3},
        {2,1,5},
        {3,1,13},
        //{4,1,65533},
        {0,2,3},
        {1,2,4},
        {2,2,7},
        {3,2,29},
        {0,3,4},
        {1,3,5},
        {2,3,9},
        {3,3,61},
        {0,4,5},
        {1,4,6},
        {2,4,11},
        {3,4,125}
    };
    int nTests=sizeof(aTests)/sizeof(aTests[0]);

    int failed=0;
    for(int i=0; i<nTests; i++){
        int n=aTests[i].n;
        int m=aTests[i].m;
        int ref=aTests[i].ref;

        if(logEvents){
            printf("ackerman, n=%u, start\n", i);
        }
        int got=ackerman(m,n);
        if(logEvents){
            printf("ackerman, n=%u, finish\n", i);
        }
        //printf("ack(%d,%d)=%d, got=%d\n", m,n,ref, got);
        if(got!=ref){
            //printf("FAIL : ack(%d,%d)=%d, got=%d\n", m,n,ref, got);
            failed++;
        }
    }

    return failed==0;
}

}; // hls_recurse

#endif
