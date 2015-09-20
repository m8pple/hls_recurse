#include "hls_recurse/examples/miser_indexed.hpp"

using namespace hls_recurse;

float vhls_miser_indexed(float p[4096], unsigned regn, unsigned long npts, unsigned freeStart)
{
    #pragma HLS RESOURCE core=RAM_T2P_BRAM variable=p
    #pragma HLS INTERFACE ap_memory depth=4096 port=p

    uint32_t seed=12345678;
    return pfloat_pair_first(f2_miser_indexed(p, regn, npts, seed, freeStart));
}

/*
int main()
{
    return test_miser_indexed(vhls_miser_indexed);
};
*/
