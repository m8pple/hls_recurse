#include "hls_recurse/examples/miser_indexed.hpp"

using namespace hls_recurse;

void vhls_miser_indexed(float *p, unsigned regn, unsigned long npts, float dith,  unsigned freeStart, unsigned freeTotal)
{
    #pragma HLS INTERFACE depth=65536 port=p

    f2_miser_indexed(p, regn, npts, dith, freeStart, freeTotal);
}

int main()
{
    return test_miser_indexed(vhls_miser_indexed);
};
