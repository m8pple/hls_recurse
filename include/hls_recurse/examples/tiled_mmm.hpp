#ifndef tiled_mmm_hpp
#define tiled_mmm_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

namespace hls_recurse
{

void r_tiled_mmm(int n, int stride, float *dst, const float *a, const float *b)
{
    auto sub = [&](int v,int h) -> int
    { return v*stride*n/2+h*n/2; };

    if( n<=16 ){
        for(int r=0; r<n; r++){
            for(int c=0; c<n; c++){
                float acc=dst[r*stride+c];
                for(int i=0; i<n; i++){
                    acc += a[r*stride+i] * b[i*stride+c];
                }
                dst[r*stride+c]=acc;
            }
        }
    }else{

        r_tiled_mmm(n/2, stride, dst+sub(0,0), a+sub(0,0), b+sub(0,0));
        r_tiled_mmm(n/2, stride, dst+sub(0,1), a+sub(0,0), b+sub(0,1));
        r_tiled_mmm(n/2, stride, dst+sub(1,0), a+sub(1,0), b+sub(0,0));
        r_tiled_mmm(n/2, stride, dst+sub(1,1), a+sub(1,0), b+sub(0,1));

        r_tiled_mmm(n/2, stride, dst+sub(0,0), a+sub(0,1), b+sub(1,0));
        r_tiled_mmm(n/2, stride, dst+sub(0,1), a+sub(0,1), b+sub(1,1));
        r_tiled_mmm(n/2, stride, dst+sub(1,0), a+sub(1,1), b+sub(1,0));
        r_tiled_mmm(n/2, stride, dst+sub(1,1), a+sub(1,1), b+sub(1,1));
    }
}

class tiled_mmm
    : public Function<tiled_mmm, void, int, int, float *, const float *, const float *>
{
private:
    int n;
    int stride;
    float *dst;
    const float *a;
    const float *b;

    int sub(int v,int h)
    { return v*stride*n/2+h*n/2; }
public:
    tiled_mmm(stack_entry_t *stack)
        : function_base_t(*this, stack, n, stride, dst, a, b)
    {}

    template<class TVisitor>
    void VisitBody(TVisitor visitor)
    {
        visitor(
            IfElse([&](){ return n<=16; },
                [&](){
                    for(int r=0; r<n; r++){
                        for(int c=0; c<n; c++){
                            float acc=dst[r*stride+c];
                            for(int i=0; i<n; i++){
                                acc += a[r*stride+i] * b[i*stride+c];
                            }
                            dst[r*stride+c]=acc;
                        }
                    }
                },
                Sequence(
                    Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(0,0), a+sub(0,0), b+sub(0,0)); }),
                    Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(0,1), a+sub(0,0), b+sub(0,1)); }),
                    Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(1,0), a+sub(1,0), b+sub(0,0)); }),
                    Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(1,1), a+sub(1,0), b+sub(0,1)); }),

                    Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(0,0), a+sub(0,1), b+sub(1,0)); }),
                    Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(0,1), a+sub(0,1), b+sub(1,1)); }),
                    Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(1,0), a+sub(1,1), b+sub(1,0)); }),
                    Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(1,1), a+sub(1,1), b+sub(1,1)); })
                )
            )
        );
    }
};

void f_tiled_mmm(int n, int stride, float *dst, const float *a, const float *b)
{
    tiled_mmm::stack_entry_t stack[64];
    tiled_mmm tiled_mmm(stack);
    tiled_mmm(n,stride,dst,a,b);
}


void f2_tiled_mmm(int n, int stride, float *dst, const float *a, const float *b)
{
    auto sub = [&](int v,int h) -> int
    { return v*stride*n/2+h*n/2; };

    run_function_old<void>(
        IfElse([&](){ return n<=16; },
            [&](){
                for(int r=0; r<n; r++){
                    for(int c=0; c<n; c++){
                        float acc=dst[r*stride+c];
                        for(int i=0; i<n; i++){
                            acc += a[r*stride+i] * b[i*stride+c];
                        }
                        dst[r*stride+c]=acc;
                    }
                }
            },
            Sequence(
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(0,0), a+sub(0,0), b+sub(0,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(0,1), a+sub(0,0), b+sub(0,1)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(1,0), a+sub(1,0), b+sub(0,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(1,1), a+sub(1,0), b+sub(0,1)); }),

                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(0,0), a+sub(0,1), b+sub(1,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(0,1), a+sub(0,1), b+sub(1,1)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(1,0), a+sub(1,1), b+sub(1,0)); }),
                Recurse([&](){ return make_hls_state_tuple(n/2, stride, dst+sub(1,1), a+sub(1,1), b+sub(1,1)); })
            )
        ),
        n, stride, dst, a, b
    );
}

template<class T>
bool test_tiled_mmm(T tiled_mmm, bool logEvents=false)
{
    bool ok=true;

    const unsigned MN=128;
    float a[MN*MN], b[MN*MN], got[MN*MN], ref[MN*MN];

    for(int n=2;n<=MN;n*=2){

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

        if(logEvents){
            printf("tiled_mmm, n=%u, start\n", n);
        }
        tiled_mmm(n, n, got, a, b);
        if(logEvents){
            printf("tiled_mmm, n=%u, finish\n", n);
        }

        if(n<=128){
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
                    ok=false;
                }
            }
        }
    }

    return true;
}

}; // hls_recurse

#endif
