#ifndef fib_man_hpp
#define fib_man_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

#include "hls_recurse/examples/fib.hpp"

namespace hls_recurse
{

// Stupid proxy to make legup build easier
HLS_INLINE_STEP uint32_t f2_fib_man(uint32_t n)
{
    return man_fib(n);
}

template<class T>
bool test_fib_man(T fib, bool logEvents=false)
{
    return test_fib(fib, logEvents);
}

template<class T>
int harness_fib_man(T fib)
{
    return harness_fib(fib);
}

}; // hls_recurse

#endif
