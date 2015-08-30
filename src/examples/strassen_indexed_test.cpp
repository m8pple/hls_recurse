#include "examples/strassen_indexed.hpp"

#include <stdlib.h>

int main()
{
    if(!test_strassen_indexed(r_strassen_indexed)){
        fprintf(stderr, "STRASSEN/r failed.\n");
    }

    if(!test_strassen_indexed(f2_strassen_indexed)){
        fprintf(stderr, "STRASSEN/sm failed.\n");
    }

    return 0;
};
