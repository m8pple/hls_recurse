#ifndef while_perf_hpp
#define while_perf_hpp

#include <stdint.h>
#include <stdio.h>

namespace hls_recurse
{

uint32_t step(uint32_t x)
{
    x^=x<<1;
    x^=x>>3;
    x^=x<<10;
    return x;
}

uint32_t while_perf_native(uint32_t x)
{
    uint32_t i=0;
    while(x>1000){
        x=step(x);
        i++;
    }
    return i;
}

uint32_t while_perf_sm(uint32_t x)
{
    uint32_t i=0;
    run_state_machine(
        While([&](){ return x > 1000; },
            [&](){
                x=step(x);
                i++;
            }
        )
    );
    return i;
}

template<class T>
bool test_while_perf(T while_perf)
{
    uint32_t n=0xCCCCCCCCul; // Requires ~ 2000000 iterations
    uint32_t res=while_perf(n);
    printf("n=%u, res=%u\n", n, res);
    return res==2098296;
}

}; // hls_recurse

#endif
