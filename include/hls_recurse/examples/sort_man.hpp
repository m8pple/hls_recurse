#ifndef sort_man_hpp
#define sort_man_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

#include "hls_recurse/examples/sort.hpp"

namespace hls_recurse
{

void f2_sort_man(uint32_t *a, int n)
{
    man_sort(a,n);
}

template<class T>
bool test_sort_man(T sort, bool logEvents=false)
{
    return test_sort(sort,logEvents);
}

template<class T>
int harness_sort_man(T sort)
{
    return harness_sort(sort);
}

}; // hls_recurse

#endif
