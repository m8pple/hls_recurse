#ifndef utility_hpp
#define utility_hpp

#include <assert.h>

#if defined(__SYNTHESIS__)
#define HLS_IS_SYNTHESIS
#elif defined( __linux__ ) || defined(__CYGWIN__)
#define HLS_IS_POSIX
#elif defined( _WIN32 )
#define HLS_IS_WINDOWS
#endif


#if defined(__GNUC__) || defined(__clang__)
#if (defined(HLS_IS_SYNTHESIS)||defined(NDEBUG))
#define HLS_INLINE_STEP inline __attribute__((always_inline))
#else
#define HLS_INLINE_STEP inline
#endif

#define HLS_NO_RETURN __attribute__((noreturn))

#else
#define HLS_INLINE_STEP inline
#define HLS_NO_RETURN
#endif

#ifdef HLS_IS_POSIX
#include <time.h>
#include <stdio.h>
#endif

#ifdef HLS_IS_WINDOWS
// Woo, 2015 and still trying to supress macros for min and max
#define NOMINMAX
#include <Windows.h>
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
    *(static_cast<int*>(0)) = 0;
#endif
}



uint64_t time_now()
{
#ifdef HLS_IS_SYNTHESIS
    return 0;
#elif defined(HLS_IS_POSIX)
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return uint64_t(tp.tv_sec)*1000000000 + tp.tv_nsec;
#elif defined(HLS_IS_WINDOWS)
	LARGE_INTEGER time, freq;
	QueryPerformanceCounter(&time);
	QueryPerformanceFrequency(&freq);
	return time.QuadPart/(double)freq.QuadPart;
#else
#error "No way of getting the time."
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
