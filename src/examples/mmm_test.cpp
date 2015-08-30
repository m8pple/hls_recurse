#include "examples/mmm.hpp"

#include "utility.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    
    uint64_t t1=time_now();
    if(!test_mmm(r_mmm)){
        fprintf(stderr, "MMM/recursive failed.\n");
    }
    uint64_t t2=time_now();
    printf("MMM/r : %lg s\n", time_delta(t1,t2));
    
    uint64_t t3=time_now();
    if(!test_mmm(f2_mmm)){
        fprintf(stderr, "MMM/f2 failed.\n");
    }
    uint64_t t4=time_now();
    printf("MMM/f2 : %lg s\n", time_delta(t3,t4));

    return 0;
};
