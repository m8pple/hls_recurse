#ifndef fft_indexed_hpp
#define fft_indexed_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

#include "hls_recurse/complex.hpp"

namespace hls_recurse
{

// Does not exist
void f_fft_indexed(int log2n, const complex_t *pIn, complex_t *pOut);


void f2_fft_indexed(int log2n, const complex_t *pIn, complex_t *pOut)
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
    int bIn=0;
    int sIn=1;
    int bOut=0;
    int sOut=1;

    run_function_old<void>(
        IfElse([&](){ return n<=2; },
            [&](){
                if (n == 1){
                    pOut[bOut+0] = pIn[bIn+0];
                }else if (n == 2){
                    pOut[bOut+0] = pIn[bIn+0]+pIn[bIn+sIn];
                    pOut[bOut+sOut] = pIn[bIn+0]-pIn[bIn+sIn];
                }
            },
            Sequence(
                [&](){
                    n=n/2;
                },
                Recurse([&](){ return make_hls_state_tuple(n,wn*wn,bIn,2*sIn,bOut,sOut); }),
                Recurse([&](){ return make_hls_state_tuple(n,wn*wn,bIn+sIn,2*sIn,bOut+sOut*n,sOut); }),
                [&](){
                    complex_t w=complex_t::from_int(1);

                    for (int j=0;j<n;j++){
                        complex_t t1 = w*pOut[bOut+n+j];
                        complex_t t2 = pOut[bOut+j]-t1;
                        pOut[bOut+j] = pOut[bOut+j]+t1;
                        pOut[bOut+j+n] = t2;
                        w = w*wn; // This introduces quite a lot of numerical error
                    }
                }
            )
        ),
        n, wn, bIn, sIn, bOut, sOut
    );
}


void man_fft_indexed(int log2n, const complex_t *pIn, complex_t *pOut)
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
    int bIn=0;
    int bOut=0;

    const unsigned DEPTH=512;

    int sp=0;
    int stack_n[DEPTH];
    complex_t stack_wn[DEPTH];
    int stack_bIn[DEPTH];
    int stack_sIn[DEPTH];
    int stack_bOut[DEPTH];
    int stack_sOut[DEPTH];
    int stack_state[DEPTH];

    stack_n[0]=n;
    stack_wn[0]=wn;
    stack_bIn[0]=bIn;
    stack_sIn[0]=sIn;
    stack_bOut[0]=bOut;
    stack_sOut[0]=sOut;
    stack_state[0]=0;

    while(1){
        n=stack_n[sp];
        wn=stack_wn[sp];
        bIn=stack_bIn[sp];
        sIn=stack_sIn[sp];
        bOut=stack_bOut[sp];
        sOut=stack_sOut[sp];
        int state=stack_state[sp];

        /*for(int i=0;i<sp;i++){
            printf("  ");
        }
        printf("n=%5d, ", n);*/

        if(state==0){
            if( n<=2 ){
                //printf("Leaf\n");
                if (n == 1){
                    pOut[bOut] = pIn[bIn];
                }else if (n == 2){
                    pOut[bOut] = pIn[bIn]+pIn[bIn+sIn];
                    pOut[bOut+sOut] = pIn[bIn]-pIn[bIn+sIn];
                }
                if(sp==0){
                    break;
                }else{
                    sp--;
                }
            }else{
                //printf(" Branch-0\n");
                n=n/2;
                stack_n[sp]=n;
                // r_fft_aux(n,wn*wn,pIn,2*sIn,pOut,sOut);
                stack_state[sp]=1;
                sp++;
                stack_state[sp]=0;
                stack_n[sp]=n;
                stack_wn[sp]=wn*wn;
                stack_bIn[sp]=bIn;
                stack_sIn[sp]=2*sIn;
                stack_bOut[sp]=bOut;
                stack_sOut[sp]=sOut;
            }
        }else if(state==1){
            //printf("Branch-1\n");
            // state 1
            // r_fft_aux(n,wn*wn,pIn+sIn,2*sIn,pOut+sOut*n,sOut);
            stack_state[sp]=2;
            sp++;
            stack_state[sp]=0;
            stack_n[sp]=n;
            stack_wn[sp]=wn*wn;
            stack_bIn[sp]=bIn+sIn;
            stack_sIn[sp]=2*sIn;
            stack_bOut[sp]=bOut+sOut*n;
            stack_sOut[sp]=sOut;
        }else if(state==2){
            //printf("Branch-2\n");
            // state 2
            complex_t w=complex_t::from_int(1);
            for (int j=0;j<n;j++){
                complex_t t1 = w*pOut[bOut+n+j];
                complex_t t2 = pOut[bOut+j]-t1;
                pOut[bOut+j] = pOut[bOut+j]+t1;
                pOut[bOut+j+n] = t2;
                w = w*wn; // This introduces quite a lot of numerical error
            }
            if(sp==0)
                break;
            else
                sp--;
        }
    }
}

template<class T>
bool test_fft_indexed(T fft_indexed, bool logEvents=false)
{
    complex_t in[8192], out[8192];

    for(int log2n=1; log2n<=13; log2n++){
        int n=1<<log2n;

        for(int i=0; i<n; i++){
          in[i]=complex_t::from_int(i,0);
        }

        if(logEvents){
            printf("fft_indexed, n=%u, start\n", n);
        }
        fft_indexed(log2n, &in[0], &out[0]);
        if(logEvents){
            printf("fft_indexed, n=%u, start\n", n);
        }

        // THis FFT has atrocious error properties, so it
        // is pointless to check for large n
        if(n<=1024){
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

              float relErr=err/expectedRe;
              if(relErr<0)
                  relErr=-relErr;
              if( relErr > 0.05){
                  return false;
              }
            }
        }
    }

    return true;
}

template<class T>
bool harness_fft_indexed(T fft)
{
    complex_t in[512], out[512];

    for(int i=0; i<512; i++){
      in[i]=complex_t::from_int(i,0);
    }

    fft(9, &in[0], &out[0]);

    return out[0].re_int();
}

}; // hls_recurse

#endif
