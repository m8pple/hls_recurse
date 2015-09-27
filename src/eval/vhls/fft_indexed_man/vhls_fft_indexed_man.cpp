#include "hls_recurse/examples/fft_indexed_man.hpp"

using namespace hls_recurse;

void vhls_fft_indexed_man(int log2n, const complex_t pIn[4096], complex_t pOut[4096])
{
    #pragma HLS INTERFACE ap_memory depth=4096 port=pIn
    #pragma HLS INTERFACE ap_memory depth=4096 port=pOut

    f2_fft_indexed_man(log2n, pIn, pOut);
}

int main()
{
    return test_fft_indexed(vhls_fft_indexed_man);
};
