#ifndef mmm_indexed_hpp
#define mmm_indexed_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

namespace hls_recurse
{

typedef float element_t;

// No implementation, as we can't deal with the pointe problem
void f_mmm_indexed(int n, int stride, element_t *dst, const element_t *a, const element_t *b);

int f2_mmm_indexed_sub(int stride,int n, int v,int h)
{ return v*stride*n/2+h*n/2; };

void f2_mmm_indexed(int n, int stride, element_t *dst, const element_t *a, const element_t *b)
{
    int dstIndex=0;
    int aIndex=0;
    int bIndex=0;

    run_function_old<void>(
        IfElse([&](){ return n<=16; },
            [&](){
                for(int r=0; r<n; r++){
                    for(int c=0; c<n; c++){
                        // The slightly odd structure is to avoid a bug in Vivado HLS
                        // if sum is initialised with sum=dst[dstIndex+r*stride+c]
                        element_t sum=0;
                        for(int i=0; i<n; i++){
                            sum += a[aIndex+r*stride+i] * b[bIndex+i*stride+c];
                        }
                        dst[dstIndex+r*stride+c] += sum;
                    }
                }
            }
         ,
            Sequence(
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_mmm_indexed_sub(stride, n,0,0), aIndex+f2_mmm_indexed_sub(stride, n,0,0), bIndex+f2_mmm_indexed_sub(stride, n,0,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_mmm_indexed_sub(stride, n,0,1), aIndex+f2_mmm_indexed_sub(stride, n,0,0), bIndex+f2_mmm_indexed_sub(stride, n,0,1)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_mmm_indexed_sub(stride, n,1,0), aIndex+f2_mmm_indexed_sub(stride, n,1,0), bIndex+f2_mmm_indexed_sub(stride, n,0,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_mmm_indexed_sub(stride, n,1,1), aIndex+f2_mmm_indexed_sub(stride, n,1,0), bIndex+f2_mmm_indexed_sub(stride, n,0,1)); }),

                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_mmm_indexed_sub(stride, n,0,0), aIndex+f2_mmm_indexed_sub(stride, n,0,1), bIndex+f2_mmm_indexed_sub(stride, n,1,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_mmm_indexed_sub(stride, n,0,1), aIndex+f2_mmm_indexed_sub(stride, n,0,1), bIndex+f2_mmm_indexed_sub(stride, n,1,1)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_mmm_indexed_sub(stride, n,1,0), aIndex+f2_mmm_indexed_sub(stride, n,1,1), bIndex+f2_mmm_indexed_sub(stride, n,1,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_mmm_indexed_sub(stride, n,1,1), aIndex+f2_mmm_indexed_sub(stride, n,1,1), bIndex+f2_mmm_indexed_sub(stride, n,1,1)); })
            )
        ),
        n, stride, dstIndex, aIndex, bIndex
    );
}


template<class T>
bool test_mmm_indexed(T mmm_indexed)
{
    const unsigned n=128;
    element_t a[n*n], b[n*n], got[n*n], ref[n*n];

    element_t f_rand=1;

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
            element_t acc=0;
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
