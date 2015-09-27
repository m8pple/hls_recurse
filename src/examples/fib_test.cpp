#include "hls_recurse/examples/fib.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    if(!test_fib(r_fib)){
        fprintf(stderr, "fib/recursive failed.\n");
    }
    
    if(!test_fib(man_fib)){
        fprintf(stderr, "fib/manual failed.\n");
    }

    if(!test_fib(f_fib)){
        fprintf(stderr, "fib/state failed.\n");
    }

    if(!test_fib(f2_fib)){
        fprintf(stderr, "fib/state2 failed.\n");
    }

    return 0;
};
