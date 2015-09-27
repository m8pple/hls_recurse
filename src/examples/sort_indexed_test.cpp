#include "hls_recurse/examples/sort_indexed.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    if(!test_sort_indexed(man_sort_indexed)){
        fprintf(stderr, "SORT_INDEXED/manual failed.\n");
    }


    if(!test_sort_indexed(f2_sort_indexed)){
        fprintf(stderr, "SORT_INDEXED/state2 failed.\n");
    }

    fprintf(stderr, "Pass\n");

    return 0;
};
