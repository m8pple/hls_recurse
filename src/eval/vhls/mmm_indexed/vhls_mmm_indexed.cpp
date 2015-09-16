#include "hls_recurse/examples/mmm_indexed.hpp"

using namespace hls_recurse;

void vhls_mmm_indexed(int n, int stride, element_t dst[4096], const element_t a[4096], const element_t b[4096])
{
    #pragma HLS INTERFACE ap_memory depth=4096 port=a
    #pragma HLS INTERFACE ap_memory depth=4096 port=b
    #pragma HLS INTERFACE ap_memory depth=4096 port=dst

    f2_mmm_indexed(n, stride, dst, a, b);
}

int main()
{
    return test_mmm_indexed(vhls_mmm_indexed);
};
