#include "hls_recurse/examples/strassen.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    if(!test_strassen(r_strassen)){
        fprintf(stderr, "STRASSEN/r failed.\n");
    }
    
    if(!test_strassen(man_strassen)){
        fprintf(stderr, "STRASSEN/manual failed.\n");
    }

    /*
    if(!test_strassen(f_strassen)){
        fprintf(stderr, "STRASSEN/sm failed.\n");
    }
    */
    
    if(!test_strassen(f2_strassen)){
        fprintf(stderr, "STRASSEN/sm2 failed.\n");
    }

    return 0;
};
