#include "hls_recurse/examples/fft.hpp"

#include "hls_recurse/utility.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    printf("fft start\n");

    if(test_fft(f2_fft)){
        printf("fft failed.\n");
        return 1;
    }

    printf("fft succeeded\n");

    return 0;
};
