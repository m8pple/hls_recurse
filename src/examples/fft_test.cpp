#include "examples/fft.hpp"

#include "utility.hpp"

#include <stdlib.h>

int main()
{
    
    uint64_t t1=time_now();
    if(!test_fft(r_fft)){
        fprintf(stderr, "FFT/recursive failed.\n");
    }
    uint64_t t2=time_now();
    printf("FFT/r : %lg s\n", time_delta(t1,t2));
    
    uint64_t t3=time_now();
    if(!test_fft(f2_fft)){
        fprintf(stderr, "MISER/f2 failed.\n");
    }
    uint64_t t4=time_now();
    printf("FFT/f2 : %lg s\n", time_delta(t3,t4));

    return 0;
};
