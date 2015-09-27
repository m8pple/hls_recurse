#include "hls_recurse/examples/sum.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    if(!test_sum(r_sum)){
        fprintf(stderr, "sum/recursive failed.\n");
    }
    
    if(!test_sum(man_sum)){
        fprintf(stderr, "sum/manual failed.\n");
    }

    if(!test_sum(f2_sum)){
        fprintf(stderr, "sum/state2 failed.\n");
    }

    return 0;
};
