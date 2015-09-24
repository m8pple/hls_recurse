#include "hls_recurse/examples/fib.hpp"

#include "hls_recurse/utility.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    printf("fib start\n");

    if(test_fib(f2_fib)){
        printf("fib failed.\n");
        return 1;
    }

    printf("fib succeeded\n");

    return 0;
};
