#ifndef if_else_perf_hpp
#define if_else_perf_hpp

// Get step
#include "while_perf.hpp"

namespace hls_recurse
{

uint32_t if_else_perf_native(uint32_t x)
{
    uint32_t i=0;
    
    while(x>1000){
        x=step(x);
        if(x&1){
            i=i+7;
        }else{
            i=i^0x1234567;
        }
    }
    return i;
}

uint32_t if_else_perf_sm(uint32_t x)
{
    uint32_t i=0;
    
    auto body=IfElse([&](){ return x&1; },
        [&](){ i=i+7; },
        [&](){ i=i^0x1234567; }
    );
    
    while(x>1000){
        x=step(x);
        run_state_machine(body);
    }
    return i;
}

template<class T>
bool test_if_else_perf(T while_perf)
{
    uint32_t n=0xCCCCCCCCul; // Requires ~ 2000000 iterations
    uint32_t res=while_perf(n);
    printf("n=%u, res=%u\n", n, res);
    return res==17987606;
}

}; // hls_recurse

#endif
