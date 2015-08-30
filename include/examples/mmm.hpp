#ifndef mmm_hpp
#define mmm_hpp

#include "state_machine_self_recursion.hpp"

namespace hls_recurse
{

void r_mmm(int n, int stride, float *dst, const float *a, const float *b)
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
            
        r_mmm(n/2, stride, dst+sub(0,0), a+sub(0,0), b+sub(0,0)); 
        r_mmm(n/2, stride, dst+sub(0,1), a+sub(0,0), b+sub(0,1)); 
        r_mmm(n/2, stride, dst+sub(1,0), a+sub(1,0), b+sub(0,0)); 
        r_mmm(n/2, stride, dst+sub(1,1), a+sub(1,0), b+sub(0,1)); 

        r_mmm(n/2, stride, dst+sub(0,0), a+sub(0,1), b+sub(1,0)); 
        r_mmm(n/2, stride, dst+sub(0,1), a+sub(0,1), b+sub(1,1)); 
        r_mmm(n/2, stride, dst+sub(1,0), a+sub(1,1), b+sub(1,0)); 
        r_mmm(n/2, stride, dst+sub(1,1), a+sub(1,1), b+sub(1,1));
    }
}

class MMM
    : public Function<MMM, void, int, int, float *, const float *, const float *>
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
    MMM(stack_entry_t *stack)
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

void f_mmm(int n, int stride, float *dst, const float *a, const float *b)
{
    MMM::stack_entry_t stack[64];
    MMM mmm(stack);
    mmm(n,stride,dst,a,b);
}


void f2_mmm(int n, int stride, float *dst, const float *a, const float *b)
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
bool test_mmm(T mmm)
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

    mmm(n, n, got, a, b);

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
