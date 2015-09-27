#ifndef sort_indexed_man_hpp
#define sort_indexed_man_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

#include "hls_recurse/examples/sort_indexed.hpp"

namespace hls_recurse
{

void f2_sort_indexed_man(uint32_t *a, int n)
{
    man_sort_indexed(a,n);
}

template<class T>
bool test_sort_indexed_man(T sort, bool logEvents=false)
{
    return test_sort_indexed(sort,logEvents);
}

template<class T>
int harness_sort_indexed_man(T sort)
{
    return harness_sort_indexed(sort);
}

}; // hls_recurse

#endif
