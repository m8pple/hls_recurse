#ifndef strassen_man_hpp
#define strassen_man_hpp

#include "hls_recurse/examples/strassen.hpp"

namespace hls_recurse
{

void f2_strassen_man(matrix_t &_dst, const matrix_t &_a, const matrix_t &_b, free_region_t _hFree)
{
    man_strassen(_dst, _a, _b, _hFree);
}

template<class TImpl>
bool test_strassen_man(TImpl strassen, bool logEvents=false)
{
    return test_strassen(strassen,logEvents);
}

template<class TImpl>
int harness_strassen_man(TImpl strassen)
{
    return harness_strassen(strassen);
}

}; // hls_recurse

#endif
