#include "hls_recurse/examples/miser.hpp"

#include "hls_recurse/utility.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    
    uint64_t t1=time_now();
    if(!test_miser(r_miser)){
        fprintf(stderr, "MISER/recursive failed.\n");
    }
    uint64_t t2=time_now();
    printf("MISER/r : %lg s\n", time_delta(t1,t2));
    
    uint64_t t3=time_now();
    if(!test_miser(f2_miser)){
        fprintf(stderr, "MISER/f failed.\n");
    }
    uint64_t t4=time_now();
    printf("MISER/f : %lg s\n", time_delta(t3,t4));

    return 0;
};
