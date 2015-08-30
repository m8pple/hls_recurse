#ifndef fft_hpp
#define fft_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

#include "hls_recurse/complex.hpp"

namespace hls_recurse
{

void r_fft_aux(int n, complex_t wn, const complex_t *pIn, int sIn, complex_t *pOut, int sOut)
{
    if( n<=2 ){
        if (n == 1){
            pOut[0] = pIn[0];
        }else if (n == 2){
            pOut[0] = pIn[0]+pIn[sIn];
            pOut[sOut] = pIn[0]-pIn[sIn];
        }
    }else{
        n=n/2;
        r_fft_aux(n,wn*wn,pIn,2*sIn,pOut,sOut);
        r_fft_aux(n,wn*wn,pIn+sIn,2*sIn,pOut+sOut*n,sOut);
        
        complex_t w=complex_t::from_int(1);
        for (int j=0;j<n;j++){
            complex_t t1 = w*pOut[n+j];
            complex_t t2 = pOut[j]-t1;
            pOut[j] = pOut[j]+t1;
            pOut[j+n] = t2;
            w = w*wn; // This introduces quite a lot of numerical error
        }
    }
}

void r_fft(int log2n, const complex_t *pIn, complex_t *pOut)
{
    const complex_t wn_table[16]={
      complex_t::from_float(1,     -2.44929359829471e-16),
      complex_t::from_float(-1,     1.22464679914735e-16),
      complex_t::from_float(6.12323399573677e-17, 1),
      complex_t::from_float(0.707106781186548,         0.707106781186547),
      complex_t::from_float(0.923879532511287,          0.38268343236509),
      complex_t::from_float(0.98078528040323 ,        0.195090322016128),
      complex_t::from_float(0.995184726672197 ,       0.0980171403295606),
      complex_t::from_float(0.998795456205172,         0.049067674327418),
      complex_t::from_float(0.999698818696204,        0.0245412285229123),
      complex_t::from_float(0.999924701839145,        0.0122715382857199),
      complex_t::from_float(0.999981175282601,       0.00613588464915448),
      complex_t::from_float(0.999995293809576,       0.00306795676296598),
      complex_t::from_float(0.999998823451702,       0.00153398018628477),
      complex_t::from_float(0.999999705862882,      0.000766990318742704),
      complex_t::from_float(0.999999926465718,      0.000383495187571396),
      complex_t::from_float(0.999999981616429,      0.000191747597310703)
    };

    int n=1<<log2n;
    complex_t wn=wn_table[log2n];
    int sIn=1;
    int sOut=1;
    
    r_fft_aux(n, wn, pIn, sIn, pOut, sOut);
}


class FFT
    : public Function<FFT, void, int, complex_t, const complex_t *, int, complex_t *, int>
{
private:
    int n;
    complex_t wn;
    const complex_t *pIn;
    int sIn;
    complex_t *pOut;
    int sOut;
public:
    template<class TVisitor>
    void VisitBody(TVisitor visitor)
    {
        visitor(
            IfElse([&](){ return n<=2; },
                [&](){
                    if (n == 1){
                        pOut[0] = pIn[0];
                    }else if (n == 2){
                        pOut[0] = pIn[0]+pIn[sIn];
                        pOut[sOut] = pIn[0]-pIn[sIn];
                    }
                },
                Sequence(
                    [&](){
                        n=n/2;
                    },
                    Recurse([&](){ return make_hls_state_tuple(n,wn*wn,pIn,2*sIn,pOut,sOut); }),
                    Recurse([&](){ return make_hls_state_tuple(n,wn*wn,pIn+sIn,2*sIn,pOut+sOut*n,sOut); }),
                    [&](){
                        complex_t w=complex_t::from_int(1);

                        for (int j=0;j<n;j++){
                            complex_t t1 = w*pOut[n+j];
                            complex_t t2 = pOut[j]-t1;
                            pOut[j] = pOut[j]+t1;
                            pOut[j+n] = t2;
                            w = w*wn; // This introduces quite a lot of numerical error
                        }
                    }
                )
            )
        );
    }

    FFT(stack_entry_t *stack)
        : function_base_t(
            *this, stack, n, wn, pIn, sIn, pOut, sOut
        )
    {}
};

void f_fft(int log2n, const complex_t *pIn, complex_t *pOut)
{
    const complex_t wn[16]={
      complex_t::from_float(1,     -2.44929359829471e-16),
      complex_t::from_float(-1,     1.22464679914735e-16),
      complex_t::from_float(6.12323399573677e-17, 1),
      complex_t::from_float(0.707106781186548,         0.707106781186547),
      complex_t::from_float(0.923879532511287,          0.38268343236509),
      complex_t::from_float(0.98078528040323 ,        0.195090322016128),
      complex_t::from_float(0.995184726672197 ,       0.0980171403295606),
      complex_t::from_float(0.998795456205172,         0.049067674327418),
      complex_t::from_float(0.999698818696204,        0.0245412285229123),
      complex_t::from_float(0.999924701839145,        0.0122715382857199),
      complex_t::from_float(0.999981175282601,       0.00613588464915448),
      complex_t::from_float(0.999995293809576,       0.00306795676296598),
      complex_t::from_float(0.999998823451702,       0.00153398018628477),
      complex_t::from_float(0.999999705862882,      0.000766990318742704),
      complex_t::from_float(0.999999926465718,      0.000383495187571396),
      complex_t::from_float(0.999999981616429,      0.000191747597310703)
    };


    FFT::stack_entry_t stack[64];
    FFT fft(stack);

    fft(1<<log2n, wn[log2n], pIn, 1, pOut, 1);
}

void f2_fft(int log2n, const complex_t *pIn, complex_t *pOut)
{
    const complex_t wn_table[16]={
      complex_t::from_float(1,     -2.44929359829471e-16),
      complex_t::from_float(-1,     1.22464679914735e-16),
      complex_t::from_float(6.12323399573677e-17, 1),
      complex_t::from_float(0.707106781186548,         0.707106781186547),
      complex_t::from_float(0.923879532511287,          0.38268343236509),
      complex_t::from_float(0.98078528040323 ,        0.195090322016128),
      complex_t::from_float(0.995184726672197 ,       0.0980171403295606),
      complex_t::from_float(0.998795456205172,         0.049067674327418),
      complex_t::from_float(0.999698818696204,        0.0245412285229123),
      complex_t::from_float(0.999924701839145,        0.0122715382857199),
      complex_t::from_float(0.999981175282601,       0.00613588464915448),
      complex_t::from_float(0.999995293809576,       0.00306795676296598),
      complex_t::from_float(0.999998823451702,       0.00153398018628477),
      complex_t::from_float(0.999999705862882,      0.000766990318742704),
      complex_t::from_float(0.999999926465718,      0.000383495187571396),
      complex_t::from_float(0.999999981616429,      0.000191747597310703)
    };

    int n=1<<log2n;
    complex_t wn=wn_table[log2n];
    int sIn=1;
    int sOut=1;
    
    run_function_old<void>(
        IfElse([&](){ return n<=2; },
            [&](){
                if (n == 1){
                    pOut[0] = pIn[0];
                }else if (n == 2){
                    pOut[0] = pIn[0]+pIn[sIn];
                    pOut[sOut] = pIn[0]-pIn[sIn];
                }
            },
            Sequence(
                [&](){
                    n=n/2;
                },
                Recurse([&](){ return make_hls_state_tuple(n,wn*wn,pIn,2*sIn,pOut,sOut); }),
                Recurse([&](){ return make_hls_state_tuple(n,wn*wn,pIn+sIn,2*sIn,pOut+sOut*n,sOut); }),
                [&](){
                    complex_t w=complex_t::from_int(1);

                    for (int j=0;j<n;j++){
                        complex_t t1 = w*pOut[n+j];
                        complex_t t2 = pOut[j]-t1;
                        pOut[j] = pOut[j]+t1;
                        pOut[j+n] = t2;
                        w = w*wn; // This introduces quite a lot of numerical error
                    }
                }
            )
        ),
        n, wn, pIn, sIn, pOut, sOut
    );
}

int close(complex_t ref, complex_t got)
{
    complex_t diff=ref-got;
    float err=diff.re_float()*diff.re_float()+diff.im_float()*diff.im_float();

    if(err<0)
        err=-err;
    return err < 0.001;
}

template<class T>
bool test_fft(T fft)
{
    int log2n=8;
    int n=1<<log2n;

    complex_t in[1024];
    for(int i=0; i<n; i++){
      in[i]=complex_t::from_int(i,0);
    }
    complex_t out[1024];

    fft(log2n, &in[0], &out[0]);

    for(int i=0; i<n; i++){
      int expectedRe;
      if(i==0){
        expectedRe=(1<<(log2n-1)) * (n-1);
      }else{
        expectedRe=-1<<(log2n-1);
      }
      float err=expectedRe-out[i].re_float();
      if(err<0){
          err=-err;
      }
      int milliErr=int(err*1000);
      //printf("%d : got = %d + i %d, ref= %d + i ?,  err=%d/1000 \n", i, out[i].re_int(), out[i].im_int(), expectedRe, milliErr);
      if( err > 0.05){
          return false;
      }
    }

    return true;
}

}; // hls_recurse

#endif
