#include "hls_recurse/examples/${DESIGN_NAME}.hpp"

#include "hls_recurse/utility.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    printf("test_${DESIGN_NAME} start\n");

    if(!test_${DESIGN_NAME}(f2_${DESIGN_NAME},true)){
        printf("test_${DESIGN_NAME} failed.\n");
        return 1;
    }

    printf("test_${DESIGN_NAME} succeeded\n");

    return 0;
};
