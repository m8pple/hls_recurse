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

uint32_t man_ackerman(uint32_t m, uint32_t n)
{
    const unsigned DEPTH=512;

    unsigned sp=0;
    uint32_t stack_n[DEPTH];
    uint32_t stack_m[DEPTH];
    int stack_state[DEPTH];
    uint32_t retval;

    stack_n[sp]=n;
    stack_m[sp]=m;
    stack_state[sp]=0;

    while(1){
        int state=stack_state[sp];
        n=stack_n[sp];
        m=stack_m[sp];

        /*
        for(int i=0;i<sp;i++){
            printf("  ");
        }
        printf("sp=%d, n=%d, m=%d ", sp, n,m);
        */

        if(state==0){
            if(m==0){
                //printf("leaf\n");

                retval=n+1;
                if(sp==0){
                    break;
                }else{
                    sp--;
                }
            }else if(m>0 && n==0){
                //printf("branchA-0\n");

                // return r_ackerman(m-1, 1);
                // call r_ackerman(m-1, 1);
                stack_state[sp]=1;
                sp++;
                stack_state[sp]=0;
                stack_m[sp]=m-1;
                stack_n[sp]=1;
            }else{
                //printf("branchB-0\n");

                //n=r_ackerman(m,n-1);
                //return r_ackerman(m-1,n);
                // call ackerman(m,n-1)
                stack_state[sp]=2;
                sp++;
                stack_state[sp]=0;
                stack_m[sp]=m;
                stack_n[sp]=n-1;
            }
        }else if(state==1){
            //printf("branchA-1\n");
            // complete r_ackerman(m-1,1)
            // propagate retval back
            if(sp==0){
                break;
            }else{
                sp--;
            }
        }else if(state==2){
            //printf("branchB-2\n");
            // complete n=ackerman(m,n-1)
            n=retval;
            stack_n[sp]=n;
            // call ackerman(m,n-1)
            stack_state[sp]=3;
            sp++;
            stack_state[sp]=0;
            stack_m[sp]=m-1;
            stack_n[sp]=n;
        }else if(state==3){
            //printf("branchB-3\n");
            // complete ackerman(m-1,n)
            if(sp==0){
                break;
            }else{
                sp--;
            }
        }
    }
    return retval;
}


/*
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
}*/

uint32_t f2_ackerman(uint32_t m, uint32_t n)
{
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
                        // LEGUP-HACK:
                        //RecurseWithResult(n, [&](){ return make_hls_state_tuple(m-1,n); }),
                        //Return([&](){ return n; })
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
        int _pad;
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
            printf("FAIL : ack(%d,%d)=%d, got=%d\n", m,n,ref, got);
            failed++;
        }
    }

    return failed==0;
}

template<class T>
int harness_ackerman(T ackerman)
{
    int acc=0;
    for(int n=0;n<4;n++){
        for(int m=0;m<4;m++){
            acc+=ackerman(m,n);
        }
    }
    return acc;
}

}; // hls_recurse

#endif
