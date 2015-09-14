#include "hls_recurse/examples/fft_indexed.hpp"

using namespace hls_recurse;

void vhls_fft_indexed(int log2n, const complex_t *pIn, complex_t *pOut)
{
    #pragma HLS INTERFACE depth=65536 port=pIn
    #pragma HLS INTERFACE depth=65536 port=pOut

    f2_fft_indexed(log2n, pIn, pOut);
}

int main()
{
    return test_fft_indexed(vhls_fft_indexed);
};
