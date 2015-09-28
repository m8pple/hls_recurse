#include "hls_recurse/examples/strassen_indexed_v2.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    if(!test_strassen_indexed_v2(r_strassen_indexed_v2)){
        fprintf(stderr, "STRASSEN_INDEXED_V2/r failed.\n");
    }

    if(!test_strassen_indexed_v2(f2_strassen_indexed_v2)){
        fprintf(stderr, "STRASSEN_INDEXED_V2/sm failed.\n");
    }
    
    if(!test_strassen_indexed_v2(man_strassen_indexed_v2)){
        fprintf(stderr, "STRASSEN_INDEXED_V2/manual failed.\n");
    }

    fprintf(stderr, "Pass\n");

    return 0;
};
