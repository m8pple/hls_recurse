#ifndef tiled_mmm_indexed_hpp
#define tiled_mmm_indexed_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

#ifndef __SYNTHESIS__
#include <functional>
#endif

namespace hls_recurse
{

typedef float element_t;

// No implementation, as we can't deal with the pointe problem
void f_tiled_mmm_indexed(int n, int stride, element_t *dst, const element_t *a, const element_t *b);

int f2_tiled_mmm_indexed_sub(int stride,int n, int v,int h)
{ return v*stride*n/2+h*n/2; };

void r_tiled_mmm_indexed(int n, int stride, element_t *dst, const element_t *a, const element_t *b)
#ifdef __SYNTHESIS__
;
#else
{
    std::function<void(int,int,int,int,int)> aux=[&](int n, int stride, int dstIndex, int aIndex, int bIndex){
        if( n<=16 ){
            for(int r=0; r<n; r++){
                for(int c=0; c<n; c++){
                    element_t sum=0;
                    for(int i=0; i<n; i++){
                        sum += a[aIndex+r*stride+i] * b[bIndex+i*stride+c];
                    }
                    dst[dstIndex+r*stride+c] += sum;
                }
            }
        }else{
            aux(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0), aIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0), bIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0));
            aux(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1), aIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0), bIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1));
            aux(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0), aIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0), bIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0));
            aux(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1), aIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0), bIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1));

            aux(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0), aIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1), bIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0));
            aux(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1), aIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1), bIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1));
            aux(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0), aIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1), bIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0));
            aux(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1), aIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1), bIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1));
        }
    };

    aux(n, stride, 0, 0, 0);
}
#endif

void f2_tiled_mmm_indexed(int n, int stride, element_t *dst, const element_t *a, const element_t *b)
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
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0), aIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0), bIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1), aIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0), bIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0), aIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0), bIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1), aIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0), bIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1)); }),

                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,0,0), aIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1), bIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1), aIndex+f2_tiled_mmm_indexed_sub(stride, n,0,1), bIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0), aIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1), bIndex+f2_tiled_mmm_indexed_sub(stride, n,1,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dstIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1), aIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1), bIndex+f2_tiled_mmm_indexed_sub(stride, n,1,1)); })
            )
        ),
        n, stride, dstIndex, aIndex, bIndex
    );
}


template<class T>
bool test_tiled_mmm_indexed(T tiled_mmm_indexed)
{
    const unsigned n=128;
    element_t a[n*n], b[n*n], got[n*n], ref[n*n];

    element_t f_rand=1;

    for(unsigned i=0;i<n*n;i++){
        // Keep the values small so that it is exact
        a[i]=f_rand;
        f_rand=f_rand+2;
        b[i]=f_rand;
        f_rand=f_rand+7;
        got[i]=0;
        ref[i]=0;
        if(f_rand>100){
            f_rand-=97;
        }
    }

    tiled_mmm_indexed(n, n, got, a, b);

    for(unsigned r=0; r<n; r++){
        for(unsigned c=0; c<n; c++){
            element_t acc=0;
            for(unsigned i=0; i<n; i++){
                acc += a[r*n+i] * b[i*n+c];
            }
            ref[r*n+c]=acc;
        }
    }

    for(unsigned r=0; r<n; r++){
        for(unsigned c=0; c<n; c++){
            if(ref[r*n+c]!=got[r*n+c]){
                fprintf(stderr, "got[%u,%u]=%f, ref[%u,%u]=%f\n", r,c,got[r*n+c], r,c,ref[r*n+c]);
                return false;
            }
        }
    }

    return true;
}

}; // hls_recurse

#endif
