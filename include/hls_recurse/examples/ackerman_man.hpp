#ifndef ackerman_man_hpp
#define ackerman_man_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

#include "ackerman.hpp"

namespace hls_recurse
{

HLS_INLINE_STEP uint32_t f2_ackerman_man(uint32_t m, uint32_t n)
{
    return man_ackerman(m,n);
}

template<class T>
bool test_ackerman_man(T ackerman, bool logEvents=false)
{
    return test_ackerman(ackerman, logEvents);
}

template<class T>
int harness_ackerman_man(T ackerman)
{
    return harness_ackerman(ackerman);
}

}; // hls_recurse

#endif
