#ifndef for_perf_hpp
#define for_perf_hpp

#include <stdint.h>
#include <stdio.h>

namespace hls_recurse
{


uint32_t for_perf_native(uint32_t x)
{
    uint32_t i;
    
    uint32_t xx;
    for(xx=x; xx>1000; xx=step(xx)){
        i++;
    }
    return i;
}

uint32_t for_perf_sm(uint32_t x)
{
    uint32_t i=0;
    uint32_t xx;
    run_state_machine(
        For(
            [&](){ xx=x; },
            [&](){ return xx>1000; },
            [&](){ xx=step(xx); },
            [&](){ i++; }
        )
    );
    return i;
}

template<class T>
bool test_for_perf(T for_perf)
{
    uint32_t n=0xCCCCCCCCul; // Requires ~ 2000000 iterations
    uint32_t res=for_perf(n);
    printf("n=%u, res=%u\n", n, res);
    return res==2098296;
}

}; // hls_recurse

#endif
