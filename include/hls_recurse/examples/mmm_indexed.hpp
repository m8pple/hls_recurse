#ifndef mmm_indexed_hpp
#define mmm_indexed_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

namespace hls_recurse
{

// No implementation, as we can't deal with the pointe problem
void f_mmm_indexed(int n, int stride, float *dst, const float *a, const float *b);

void f2_mmm_indexed(int n, int stride, float *dst, const float *a, const float *b)
{
    auto sub = [&](int v,int h) -> int
    { return v*stride*n/2+h*n/2; };
    
    int dstIndex=0;
    int aIndex=0;
    int bIndex=0;
    
    run_function_old<void>(
        IfElse([&](){ return n<=16; },
            [&](){
                for(int r=0; r<n; r++){
                    for(int c=0; c<n; c++){
                        float acc=dst[dstIndex+r*stride+c];
                        for(int i=0; i<n; i++){
                            acc += a[aIndex+r*stride+i] * b[bIndex+i*stride+c];
                        }
                        dst[dstIndex+r*stride+c]=acc;
                    }
                }
            },
            Sequence(
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+sub(0,0), aIndex+sub(0,0), bIndex+sub(0,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+sub(0,1), aIndex+sub(0,0), bIndex+sub(0,1)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+sub(1,0), aIndex+sub(1,0), bIndex+sub(0,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+sub(1,1), aIndex+sub(1,0), bIndex+sub(0,1)); }),

                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+sub(0,0), aIndex+sub(0,1), bIndex+sub(1,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+sub(0,1), aIndex+sub(0,1), bIndex+sub(1,1)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+sub(1,0), aIndex+sub(1,1), bIndex+sub(1,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+sub(1,1), aIndex+sub(1,1), bIndex+sub(1,1)); })
            )
        ),
        n, stride, dstIndex, aIndex, bIndex      
    );
}


template<class T>
bool test_mmm_indexed(T mmm_indexed)
{
    const unsigned n=128;
    float a[n*n], b[n*n], got[n*n], ref[n*n];

    float f_rand=1;

    for(unsigned i=0;i<n*n;i++){
        // Keep the values small so that it is exact
        a[i]=f_rand;
        f_rand=f_rand+1;
        b[i]=f_rand;
        f_rand=f_rand+1;
        got[i]=0;
        ref[i]=0;
    }

    mmm_indexed(n, n, got, a, b);

    for(unsigned r=0; r<n; r++){
        for(unsigned c=0; c<n; c++){
            float acc=0;
            for(unsigned i=0; i<n; i++){
                acc += a[r*n+i] * b[i*n+c];
            }
            ref[r*n+c]=acc;
        }
    }

    for(unsigned i=0; i<n*n; i++){
        if(ref[i]!=got[i]){
            return false;
        }
    }

    return true;
}

}; // hls_recurse

#endif
