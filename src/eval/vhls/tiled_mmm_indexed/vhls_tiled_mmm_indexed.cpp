#include "hls_recurse/examples/tiled_mmm_indexed.hpp"

using namespace hls_recurse;

void vhls_tiled_mmm_indexed(int n, int stride, element_t dst[8192], const element_t a[8192], const element_t b[8192])
{
    #pragma HLS INTERFACE ap_memory depth=4096 port=a
    #pragma HLS INTERFACE ap_memory depth=4096 port=b
    #pragma HLS INTERFACE ap_memory depth=4096 port=dst

    f2_tiled_mmm_indexed(n, stride, dst, a, b);
}

int main()
{
    return test_tiled_mmm_indexed(vhls_tiled_mmm_indexed);
};
