#ifndef sum_man_hpp
#define sum_man_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

#include "hls_recurse/examples/sum.hpp"

namespace hls_recurse
{

void f2_sum_man(uint32_t n, int32_t *f)
{
    man_sum(n,f);
}

template<class T>
bool test_sum_man(T sum, bool logEvents=false)
{
    return test_sum(sum,logEvents);
}

template<class T>
bool harness_sum_man(T sum)
{
    return harness_sum(sum);
}

}; // hls_recurse

#endif
