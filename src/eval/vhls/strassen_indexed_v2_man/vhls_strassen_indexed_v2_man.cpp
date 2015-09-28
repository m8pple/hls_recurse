#include "hls_recurse/examples/strassen_indexed_v2_man.hpp"

using namespace hls_recurse;

// 5242880 == 1024*1024*5
void vhls_strassen_indexed_v2_man(uint32_t p[5242880], unsigned log2n)
{
    unsigned n=1<<log2n;
    auto a=ipmatrix_create(log2n,log2n,0);
    auto b=ipmatrix_create(log2n,log2n,n);
    auto dst=ipmatrix_create(log2n,log2n,n+n);
    ipfree_region_t f=3*n;

    #pragma HLS INTERFACE depth=5242880 port=p

    f2_strassen_indexed_v2_man(p, dst, a, b, f);
}

/*int main()
{
    return test_strassen_indexed(vhls_strassen_indexed);
};
*/
