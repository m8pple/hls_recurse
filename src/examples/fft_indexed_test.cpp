#include "hls_recurse/examples/fft_indexed.hpp"

#include "hls_recurse/utility.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    uint64_t t1=time_now();
    if(!test_fft_indexed(man_fft_indexed)){
        fprintf(stderr, "FFT_INDEXED/manual failed.\n");
    }
    uint64_t t2=time_now();
    printf("FFT_INDEXED/manual : %lg s\n", time_delta(t1,t2));
    
    uint64_t t3=time_now();
    if(!test_fft_indexed(f2_fft_indexed)){
        fprintf(stderr, "FFT_INDEXED/f2 failed.\n");
    }
    uint64_t t4=time_now();
    printf("FFT_INDEXED/f2 : %lg s\n", time_delta(t3,t4));

    return 0;
};
