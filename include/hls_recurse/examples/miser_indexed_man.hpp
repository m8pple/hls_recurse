#ifndef miser_indexed_man_hpp
#define miser_indexed_man_hpp

#include "hls_recurse/examples/miser_indexed.hpp"

namespace hls_recurse
{

pfloat_pair_t f2_miser_indexed_man(float *p, unsigned _regn, unsigned long _npts, uint32_t &seed, unsigned _region)
{
    return man_miser_indexed(p,_regn,_npts,seed,_region);
};

template<class T>
bool test_miser_indexed_man(T miser_indexed, bool logEvents=false)
{
    return test_miser_indexed(miser_indexed,logEvents);
}

template<class T>
int harness_miser_indexed_man(T miser_indexed)
{
    return harness_miser_indexed(miser_indexed);
}

}; // hls_recurse

#endif

