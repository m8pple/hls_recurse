#include "hls_recurse/examples/sort.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    if(!test_sort(man_sort)){
        fprintf(stderr, "SORT/manual failed.\n");
    }


    if(!test_sort(f2_sort)){
        fprintf(stderr, "SORT/state2 failed.\n");
    }

    fprintf(stderr, "Pass\n");

    return 0;
};
