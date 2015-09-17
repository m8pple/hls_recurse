#include "hls_recurse/examples/strassen_indexed_v2.hpp"

using namespace hls_recurse;

void vhls_strassen_indexed_v2(uint32_t p[65536], unsigned log2n)
{
    unsigned n=1<<log2n;
    ipmatrix_t a={log2n,log2n,0};
    ipmatrix_t b={log2n,log2n,n};
    ipmatrix_t dst={log2n,log2n,n+n};
    ipfree_region_t f=3*n;

    #pragma HLS INTERFACE depth=65536 port=p

    f2_strassen_indexed_v2(p, dst, a, b, f);
}

/*int main()
{
    return test_strassen_indexed(vhls_strassen_indexed);
};
*/
