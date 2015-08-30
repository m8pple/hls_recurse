#include "examples/strassen.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    if(!test_strassen(r_strassen)){
        fprintf(stderr, "STRASSEN/r failed.\n");
    }

    if(!test_strassen(f_strassen)){
        fprintf(stderr, "STRASSEN/sm failed.\n");
    }

    return 0;
};
