#ifndef sequence_perf_hpp
#define sequence_perf_hpp

// Get step
#include "while_perf.hpp"

namespace hls_recurse
{

uint32_t sequence_perf_native(uint32_t x)
{
    uint32_t i=0xCCCCCCCCul;
    
    while(x>1000){
        x=step(x);
        i=i+1;
        i=i^0x12345678;
        i=i+2;
        i=i^0x12345678;
        i=i+3;
        i=i^0x12345678;
        i=i+4;
        i=i^0x12345678;
        
    }
    return i;
}

uint32_t sequence_perf_sm(uint32_t x)
{
    uint32_t i=0xCCCCCCCCul;
    
    auto body=Sequence(
        [&](){ i=i+1; },
        [&](){ i=i^0x12345678ul; },
        [&](){ i=i+2; },
        [&](){ i=i^0x12345678ul; },
        [&](){ i=i+3; },
        [&](){ i=i^0x12345678ul; },
        [&](){ i=i+4; },
        [&](){ i=i^0x12345678ul; }
    );
    
    while(x>1000){
        x=step(x);
        run_state_machine(body);
    }
    return i;
}

template<class T>
bool test_sequence_perf(T sequence_perf)
{
    uint32_t n=0xCCCCCCCCul; // Requires ~ 2000000 iterations
    uint32_t res=sequence_perf(n);
    printf("n=%u, res=%u\n", n, res);
    return res==4060911068;
}

}; // hls_recurse

#endif
