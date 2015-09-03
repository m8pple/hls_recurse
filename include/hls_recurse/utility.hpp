#ifndef utility_hpp
#define utility_hpp

#include <assert.h>

#ifdef __GNUC__
#define HLS_INLINE_STEP inline __attribute__((always_inline))
#else
#define HLS_INLINE_STEP inline 
#endif

#include <time.h>

namespace hls_recurse
{

uint64_t time_now()
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return uint64_t(tp.tv_sec)*1000000000 + tp.tv_nsec;
}

double time_delta(uint64_t begin, uint64_t end)
{
    int64_t diff=end-begin;
    
    return diff*1e-9;
}

}; // hls_recurse

#endif
