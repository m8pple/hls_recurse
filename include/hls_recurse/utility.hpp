#ifndef utility_hpp
#define utility_hpp

#include <assert.h>

#if defined(__GNUC__) || defined(__clang__)
#define HLS_INLINE_STEP inline __attribute__((always_inline))
#define HLS_NO_RETURN __attribute__((noreturn))

#else
#define HLS_INLINE_STEP inline
#define HLS_NO_RETURN
#endif

#ifndef __SYNTHESIS__
#include <time.h>
#endif

#include <stdint.h>
#include <type_traits>

namespace hls_recurse
{

/*! Used in situations where it is a fundamental error if the
    function ever gets called. In debug builds will assert,
    in release builds will try to indicate to the optimiser
    that the code can't be reached
*/
HLS_INLINE_STEP HLS_NO_RETURN void logic_error_if_reachable()
{
    assert(0);
#if defined(__GNUC__) || defined(__clang__)
    __builtin_unreachable();
#else
    // Scream "UB!" at the compiler
    int x=5/0;
    *(static_cast<int*>(0)) = 0;
#endif
}



uint64_t time_now()
{
#ifdef __SYNTHESIS__
    return 0;
#else
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return uint64_t(tp.tv_sec)*1000000000 + tp.tv_nsec;
#endif
}

double time_delta(uint64_t begin, uint64_t end)
{
    int64_t diff=end-begin;

    return diff*1e-9;
}

// These min and max implementations are because std::min
// uses <algorithm>, which somewhere deep inside uses
// type_info, which Vivado HLS has turned off...

template<class T>
T min(const T &a, const T &b)
{
    return (a<b) ? a : b;
}

template<class T>
T max(const T &a, const T &b)
{
    return (a<b) ? b : a;
}

}; // hls_recurse

#endif
