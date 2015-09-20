#include "hls_recurse/examples/tiled_mmm.hpp"

#include "hls_recurse/utility.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{

    uint64_t t1=time_now();
    if(!test_tiled_mmm(r_tiled_mmm)){
        fprintf(stderr, "tiled_mmm/recursive failed.\n");
    }
    uint64_t t2=time_now();
    printf("tiled_mmm/r : %lg s\n", time_delta(t1,t2));

    uint64_t t3=time_now();
    if(!test_tiled_mmm(f2_tiled_mmm)){
        fprintf(stderr, "tiled_mmm/f2 failed.\n");
    }
    uint64_t t4=time_now();
    printf("tiled_mmm/f2 : %lg s\n", time_delta(t3,t4));

    return 0;
};
