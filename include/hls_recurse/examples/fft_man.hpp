#ifndef fft_man_hpp
#define fft_man_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

#include "hls_recurse/complex.hpp"

#include "hls_recurse/examples/fft.hpp"

namespace hls_recurse
{

void f2_fft_man(int log2n, const complex_t *pIn, complex_t *pOut)
{
    man_fft(log2n, pIn, pOut);
}

template<class T>
bool test_fft_man(T fft, bool logEvents=false)
{
    return test_fft(fft,logEvents);
}

template<class T>
int harness_fft_man(T fft)
{
    return harness_fft(fft);
}

}; //

#endif
