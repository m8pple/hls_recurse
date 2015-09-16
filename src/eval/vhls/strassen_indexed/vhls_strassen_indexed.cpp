#include "hls_recurse/examples/strassen_indexed.hpp"

using namespace hls_recurse;

void vhls_strassen_indexed(uint32_t p[65536], int n)
{
    imatrix_t a={n,n,0};
    imatrix_t b={n,n,n};
    imatrix_t dst={n,n,n+n};
    ifree_region_t f=3*n;

    #pragma HLS INTERFACE depth=65536 port=p

    f2_strassen_indexed(p, dst, a, b, f);
}

/*int main()
{
    return test_strassen_indexed(vhls_strassen_indexed);
};
*/
