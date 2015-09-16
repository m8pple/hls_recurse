#include "hls_recurse/examples/miser_indexed.hpp"

using namespace hls_recurse;

float vhls_miser_indexed(float p[4096], unsigned regn, unsigned long npts, float dith,  unsigned freeStart, unsigned freeTotal)
{
    #pragma HLS INTERFACE ap_memory depth=4096 port=p

    return f2_miser_indexed(p, regn, npts, dith, freeStart, freeTotal).first;
}

/*
int main()
{
    return test_miser_indexed(vhls_miser_indexed);
};
*/
