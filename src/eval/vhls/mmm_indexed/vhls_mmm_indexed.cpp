#include "hls_recurse/examples/mmm_indexed.hpp"

using namespace hls_recurse;

void vhls_mmm_indexed(int n, int stride, float *dst, const float *a, const float *b)
{
    #pragma HLS INTERFACE depth=65536 port=a
    #pragma HLS INTERFACE depth=65536 port=b
    #pragma HLS INTERFACE depth=65536 port=dst

    f2_mmm_indexed(n, stride, dst, a, b);
}

int main()
{
    return test_mmm_indexed(vhls_mmm_indexed);
};
