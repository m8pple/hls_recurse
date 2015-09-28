#include "hls_recurse/examples/miser_indexed.hpp"

#include "hls_recurse/utility.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{

    uint64_t t1=time_now();
    if(!test_miser_indexed(r_miser_indexed)){
        fprintf(stderr, "MISER_INDEXED/recursive failed.\n");
    }
    uint64_t t2=time_now();
    printf("MISER_INDEXED/r : %lg s\n", time_delta(t1,t2));
    
    t1=time_now();
    if(!test_miser_indexed(man_miser_indexed)){
        fprintf(stderr, "MISER_INDEXED/manual failed.\n");
    }
    t2=time_now();
    printf("MISER_INDEXED/manual : %lg s\n", time_delta(t1,t2));

    uint64_t t3=time_now();
    if(!test_miser_indexed(f2_miser_indexed)){
        fprintf(stderr, "MISER_INDEXED/f2 failed.\n");
    }
    uint64_t t4=time_now();
    printf("MISER_INDEXED/f2 : %lg s\n", time_delta(t3,t4));

    return 0;
};
