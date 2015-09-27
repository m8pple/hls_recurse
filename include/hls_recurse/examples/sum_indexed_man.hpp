#ifndef sum_indexed_man_hpp
#define sum_indexed_man_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

#include "hls_recurse/examples/sum_indexed.hpp"

namespace hls_recurse
{

void f2_sum_indexed_man(uint32_t n, int32_t *f)
{
    man_sum_indexed(n,f);
};

template<class T>
bool test_sum_indexed_man(T sum_indexed, bool logEvents=false)
{
    return test_sum_indexed(sum_indexed, logEvents);
}

template<class T>
int harness_sum_indexed_man(T sum_indexed)
{
	return harness_sum_indexed(sum_indexed);
}


}; // namespace hls_recurse

#endif
