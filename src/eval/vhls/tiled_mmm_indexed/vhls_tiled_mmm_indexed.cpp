#include "hls_recurse/examples/tiled_mmm_indexed.hpp"

using namespace hls_recurse;

// 1024*1024*3 = 3145728
void vhls_tiled_mmm_indexed(int log2n, element_t p[3145728])
{
    #pragma HLS INTERFACE ap_memory depth=3145728 port=p

    unsigned n=1<<log2n;

    f2_tiled_mmm_indexed(n, n, p, p+n, p+2*n);
}
