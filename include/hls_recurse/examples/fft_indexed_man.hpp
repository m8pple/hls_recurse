#ifndef fft_indexed_man_hpp
#define fft_indexed_man_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

#include "hls_recurse/complex.hpp"

#include "hls_recurse/examples/fft_indexed.hpp"

namespace hls_recurse
{

void f2_fft_indexed_man(int log2n, const complex_t *pIn, complex_t *pOut)
{
    man_fft_indexed(log2n, pIn, pOut);
}

template<class T>
bool test_fft_indexed_man(T fft_indexed, bool logEvents=false)
{
    return test_fft_indexed(fft_indexed, logEvents);
}

template<class T>
bool harness_fft_indexed_man(T fft_indexed)
{
    return harness_fft_indexed(fft_indexed);
}

}; // hls_recurse

#endif
