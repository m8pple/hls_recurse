#ifndef strassen_indexed_v2_man_hpp
#define strassen_indexed_v2_man_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

#include "hls_recurse/examples/strassen_indexed_v2.hpp"

namespace hls_recurse
{

void f2_strassen_indexed_v2_man(uint32_t *p, ipmatrix_t &_dst, const ipmatrix_t &_a, const ipmatrix_t &_b, ipfree_region_t _hFree)
{
    man_strassen_indexed_v2(p,_dst,_a,_b,_hFree);
}

template<class TImpl>
bool test_strassen_indexed_v2_man(TImpl strassen_indexed_v2,bool logEvents=false)
{
    return test_strassen_indexed_v2(strassen_indexed_v2,logEvents);
}

template<class TImpl>
int harness_strassen_indexed_v2_man(TImpl strassen_indexed_v2)
{
    return harness_strassen_indexed_v2(strassen_indexed_v2);
}

}; // hls_recurse

#endif
