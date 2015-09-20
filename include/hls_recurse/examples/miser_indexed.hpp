#ifndef miser_indexed_hpp
#define miser_indexed_hpp

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <utility>
//#include <algorithm>
#include <stdlib.h>
#include <string.h>

#include "hls_recurse/state_machine_self_recursion.hpp"

#include "miser.hpp"

namespace hls_recurse
{

/*
    VHLS-HACK : remove dithering

    VHLS-HACK - Pair of floats as a 64 bit int, to make sure
    state is scalar

    VHLS-HACK : rewrite powf(x,1.5)=sqrt(x*x*x

    VHLS-HACK : inline function into loop
*/

typedef uint64_t pfloat_pair_t;

float pfloat_pair_first(const pfloat_pair_t &p)
{
    uint32_t f=p&0xFFFFFFFFULL;
    return *(float*)&f;
}

float pfloat_pair_second(const pfloat_pair_t &p)
{
    uint32_t f=(p>>32)&0xFFFFFFFFULL;
    return *(float*)&f;
}

pfloat_pair_t pfloat_pair_create(float a, float b)
{
    uint32_t va=*(uint32_t*)&a;
    uint32_t vb=*(uint32_t*)&b;
    return va + (uint64_t(vb)<<32);
}


float x_to_one_half(float x)
{
    return sqrtf(x*x*x);
}

pfloat_pair_t r_miser_indexed(float *p, unsigned regn, unsigned long npts, uint32_t &seed, unsigned region)
{
    /*
    Monte Carlo samples a user-supplied ndim-dimensional function func in a rectangular volume
    specified by regn[1..2*ndim], a vector consisting of ndim �lower-left� coordinates of the
    region followed by ndim �upper-right� coordinates. The function is sampled a total of npts
    times, at locations determined by the method of recursive stratified sampling. The mean value
    of the function in the region is returned as ave; an estimate of the statistical uncertainty of ave
    (square of standard deviation) is returned as var. The input parameter dith should normally
    be set to zero, but can be set to (e.g.) 0.1 if func�s active region falls on the boundary of a
    power-of-two subdivision of region.
    */

    const unsigned TDim=4;
    const float PFAC=0.1f;
    const int MNPT=15;
    const int MNBS=60;
    const float TINY=1.0e-30f;
    const float BIG=1.0e30f;
    //Here PFAC is the fraction of remaining function evaluations used at each stage to explore the
    //variance of func. At least MNPT function evaluations are performed in any terminal subregion;
    //a subregion is further bisected only if at least MNBS function evaluations are available. We take
    //MNBS = 4*MNPT.

    auto rngu=[&]() -> uint32_t{
        seed=seed*1664525+1013904223;
        return seed;
    };

    auto rngf=[&]() -> float{
        return rngu() * 0.00000000023283064365386962890625f;
    };

    if (npts < MNBS) { // Too few points to bisect; do straight
        float summ=0.0f, summ2=0.0f; //Monte Carlo.
        for (unsigned n=0;n<npts;n++) {
            float acc=0;
            for(unsigned i=0;i<TDim;i++){
                float x = p[i+regn] + rngf() * (p[TDim+i+regn]-p[i+regn]);
                acc += x*x;
            }

            float fval=expf(-acc*0.5f);

            summ += fval;
            summ2 += fval * fval;
        }
        return pfloat_pair_create(
            summ/npts,
            max(TINY,(summ2-summ*summ/npts)/(npts*npts))
        );
    }else{ //Do the preliminary (uniform) sampling.
        unsigned npre=max((unsigned)(npts*PFAC),(unsigned)MNPT);
        //fprintf(stderr, "    depth=%d, npts=%u\n", depth, npts);

        auto alloci=[&](unsigned n) -> unsigned
        {
            unsigned tmp=region;
            region+=n;
            return tmp;
        };

        auto alloc=[&](unsigned n) -> float *
        {
            return p+alloci(n);
        };

        auto pMid=[&](unsigned j){ return (p[regn+j]+p[regn+j+TDim])*0.5f; };


        float *fmaxl=alloc(TDim);
        float *fmaxr=alloc(TDim);
        float *fminl=alloc(TDim);
        float *fminr=alloc(TDim);

        for (unsigned j=0;j<TDim;j++) { // Initialize the left and right bounds for
            fminl[j]=fminr[j]=BIG;
            fmaxl[j]=fmaxr[j] = -BIG;
        }

        unsigned jb=0;
        float siglb, sigrb;

        unsigned nptl, nptr;

        float summ=0;
        for (unsigned n=0;n<npre;n++) { // Loop over the points in the sample.
            // VHLS-HACK : Inlined target function to get it to compile
            // VHLS-HACK : avoided local array for pt
            // VHLS-HACK : shift register through shift to avoid local array

            uint32_t splits=0; // Shift-register of whether left or right

            float acc=0;
            for(unsigned i=0;i<TDim;i++){
                float x = p[i+regn] + rngf() * (p[TDim+i+regn]-p[i+regn]);
                splits = splits + (x <= pMid(i) ? (1<<i) : 0 );

                acc += x*x;
            }

            float fval=expf(-acc/2);

            summ+=fval;

            for (unsigned j=0;j<TDim;j++) { // Find the left and right bounds for each
                if (splits&(1<<j)) { // dimension.
                    fminl[j]=min(fminl[j],fval);
                    fmaxl[j]=max(fmaxl[j],fval);
                }
                else {
                    fminr[j]=min(fminr[j],fval);
                    fmaxr[j]=max(fmaxr[j],fval);
                }
            }
        }

        float sumb=BIG; //Choose which dimension jb to bisect.
        siglb=sigrb=1.0;
        for (unsigned j=0;j<TDim;j++) {
            if (fmaxl[j] > fminl[j] && fmaxr[j] > fminr[j]) {

                //sigl=max(TINY,powf(fmaxl[j]-fminl[j],1.5f));
                float sigl=max(TINY,x_to_one_half(fmaxl[j]-fminl[j]));
                //sigr=max(TINY,powf(fmaxr[j]-fminr[j],1.5f));
                float sigr=max(TINY,x_to_one_half(fmaxr[j]-fminr[j]));
                float sum=sigl+sigr; //Equation (7.8.24), see text.
                if (sum<=sumb)
                {
                    sumb=sum;
                    jb=j;
                    siglb=sigl;
                    sigrb=sigr;
                }
            }
        }

        if (!jb){
            jb=rngu()%TDim;
        }

        nptl=(unsigned long)(MNPT+(npts-npre-2*MNPT)*siglb
            /(siglb+sigrb)); //Equation (7.8.23).

        nptr=npts-npre-nptl;

        assert(nptl < 1000000);
        assert(nptr < 1000000);

        // VHLS-HACK: combining region creation to avoid extra lambda step
        unsigned regn_left=alloci(2*TDim);
        unsigned regn_right=alloci(2*TDim);

        for(unsigned j=0;j<TDim;j++) { // regions.
            p[regn_left+j]=p[regn+j];
            p[regn_left+TDim+j]=p[regn+TDim+j];
            p[regn_right+j]=p[regn+j];
            p[regn_right+TDim+j]=p[regn+TDim+j];
        }
        p[regn_left+TDim+jb]=pMid(jb);  // Set upper-bound on first one
        p[regn_right+jb]=pMid(jb); // Set lower-bound on second one

        pfloat_pair_t resl=r_miser_indexed(p, regn_left, nptl, seed, region);
        pfloat_pair_t resr=r_miser_indexed(p, regn_right,nptr, seed, region);

        return pfloat_pair_create(
            0.5*pfloat_pair_first(resl)+0.5*pfloat_pair_first(resr),
            0.25*pfloat_pair_second(resl)+0.25*pfloat_pair_second(resr)
        );
    }
};


pfloat_pair_t f2_miser_indexed(float *p, uint32_t regn, uint32_t npts, uint32_t &seed, uint32_t region)
{
    /*
    Monte Carlo samples a user-supplied ndim-dimensional function func in a rectangular volume
    specified by regn[1..2*ndim], a vector consisting of ndim �lower-left� coordinates of the
    region followed by ndim �upper-right� coordinates. The function is sampled a total of npts
    times, at locations determined by the method of recursive stratified sampling. The mean value
    of the function in the region is returned as ave; an estimate of the statistical uncertainty of ave
    (square of standard deviation) is returned as var. The input parameter dith should normally
    be set to zero, but can be set to (e.g.) 0.1 if func�s active region falls on the boundary of a
    power-of-two subdivision of region.
    */

    const unsigned TDim=4;
    const float PFAC=0.1f;
    const int MNPT=15;
    const int MNBS=60;
    const float TINY=1.0e-30f;
    const float BIG=1.0e30f;
    //Here PFAC is the fraction of remaining function evaluations used at each stage to explore the
    //variance of func. At least MNPT function evaluations are performed in any terminal subregion;
    //a subregion is further bisected only if at least MNBS function evaluations are available. We take
    //MNBS = 4*MNPT.

    // VHLS-HACK : closures cause problem?
    #define rngu() (seed=seed*1664525+1013904223, seed)
    #define rngf() (rngu()*0.00000000023283064365386962890625f)

/*
    auto rngu=[&]() -> uint32_t{
        seed=seed*1664525+1013904223;
        return seed;
    };

    auto rngf=[&]() -> float{
        return rngu() * 0.00000000023283064365386962890625f;
    };
*/
    uint32_t regn_left, regn_right;
    uint32_t nptl, nptr;
    float summ, summ2;
    pfloat_pair_t resl, resr;

    return run_function_old<pfloat_pair_t>(
        IfElse([&](){ return npts < MNBS; },  // Too few points to bisect; do straight
            Sequence(
                [&](){
                    summ=0.0f, summ2=0.0f; //Monte Carlo.
                    for (unsigned n=0;n<npts;n++) {
                        float acc=0f;
                        for(unsigned i=0;i<TDim;i++){
                            float x = p[i+regn] + rngf() * (p[TDim+i+regn]-p[i+regn]);
                            acc += x*x;
                        }

                        float fval=expf(-acc*0.5f);

                        summ += fval;
                        summ2 += fval * fval;
                    }
                },
                Return([&](){ return pfloat_pair_create(
                    summ/npts,
                    max(TINY,(summ2-summ*summ/npts)/(npts*npts))
                );})
            )
        ,   //Do the preliminary (uniform) sampling.
            Sequence([&]{
                //fprintf(stderr, "branch, regn=%u, npts=%u, region=%u\n", regn, npts, region);
                assert(regn<100000);

                unsigned npre=max((unsigned)(npts*PFAC),(unsigned)MNPT);
                //fprintf(stderr, "    depth=%d, npts=%u\n", depth, npts);

                auto alloci=[&](unsigned n) -> unsigned
                {
                    unsigned tmp=region;
                    region+=n;
                    return tmp;
                };

                auto alloc=[&](unsigned n) -> float *
                {
                    return p+alloci(n);
                };

                auto pMid=[&](unsigned j){ return (p[regn+j]+p[regn+j+TDim])/2; };


                float *fmaxl=alloc(TDim);
                float *fmaxr=alloc(TDim);
                float *fminl=alloc(TDim);
                float *fminr=alloc(TDim);

                for (unsigned j=0;j<TDim;j++) { // Initialize the left and right bounds for
                    fminl[j]=fminr[j]=BIG;
                    fmaxl[j]=fmaxr[j] = -BIG;
                }

                unsigned jb=0;
                float siglb, sigrb;

                float summ=0;
                for (unsigned n=0;n<npre;n++) { // Loop over the points in the sample.
                    // VHLS-HACK : Inlined target function to get it to compile
                    // VHLS-HACK : avoided local array for pt
                    // VHLS-HACK : shift register through shift to avoid local array

                    uint32_t splits=0; // Shift-register of whether left or right

                    float acc=0;
                    for(unsigned i=0;i<TDim;i++){
                        float x = p[i+regn] + rngf() * (p[TDim+i+regn]-p[i+regn]);
                        splits = splits + (x <= pMid(i) ? (1<<i) : 0 );

                        acc += x*x;
                    }

                    float fval=expf(-acc*0.5f);

                    summ+=fval;

                    for (unsigned j=0;j<TDim;j++) { // Find the left and right bounds for each
                        if (splits&(1<<j)) { // dimension.
                            fminl[j]=min(fminl[j],fval);
                            fmaxl[j]=max(fmaxl[j],fval);
                        }
                        else {
                            fminr[j]=min(fminr[j],fval);
                            fmaxr[j]=max(fmaxr[j],fval);
                        }
                    }
                }

                float sumb=BIG; //Choose which dimension jb to bisect.
                siglb=sigrb=1.0f;
                for (unsigned j=0;j<TDim;j++) {
                    if (fmaxl[j] > fminl[j] && fmaxr[j] > fminr[j]) {
                        float sigl=max(TINY,x_to_one_half(fmaxl[j]-fminl[j]));
                        float sigr=max(TINY,x_to_one_half(fmaxr[j]-fminr[j]));
                        float sum=sigl+sigr; //Equation (7.8.24), see text.
                        if (sum<=sumb)
                        {
                            sumb=sum;
                            jb=j;
                            siglb=sigl;
                            sigrb=sigr;
                        }
                    }
                }

                if (!jb){
                    jb=rngu()%TDim;
                }

                nptl=(unsigned long)(MNPT+(npts-npre-2*MNPT)*siglb
                    /(siglb+sigrb)); //Equation (7.8.23).

                nptr=npts-npre-nptl;

                // VHLS-HACK: combining region creation to avoid extra lambda step
                regn_left=alloci(2*TDim);
                regn_right=alloci(2*TDim);

                for(unsigned j=0;j<TDim;j++) { // regions.
                    p[regn_left+j]=p[regn+j];
                    p[regn_left+TDim+j]=p[regn+TDim+j];
                    p[regn_right+j]=p[regn+j];
                    p[regn_right+TDim+j]=p[regn+TDim+j];
                }
                p[regn_left+TDim+jb]=pMid(jb);  // Set upper-bound on first one
                p[regn_right+jb]=pMid(jb); // Set lower-bound on second one
            },
            RecurseWithResult(resl, [&](){ return make_hls_state_tuple(
                regn_left, nptl, region);
            })
            ,
            RecurseWithResult(resr, [&](){ return make_hls_state_tuple(
                regn_right, nptr, region);
            }),
            Return([&](){ return pfloat_pair_create(
                (pfloat_pair_first(resl)+pfloat_pair_first(resr))*0.5f,
                (pfloat_pair_second(resl)+pfloat_pair_second(resr))*0.25f
            ); })
            )
        ),
        regn, npts, region,
        resl, resr, regn_left, regn_right, nptl, nptr
    );

    #undef rngu
    #undef rngf
};

template<class T>
bool test_miser_indexed(T miser_indexed)
{
    const unsigned N = miser_test_config::N;

    float p[16384];

    // Initial N*2 region
    p[0]=0;
    p[1]=0;
    p[2]=0;
    p[3]=0;
    p[4]=1;
    p[5]=2;
    p[6]=3;
    p[7]=4;

    unsigned long npts=100000;

    uint32_t seed=12345678;

    unsigned freeStart=N*2;

    auto res=miser_indexed(p, 0, npts, seed, freeStart);

    printf("ave = %g, std = %g\n", pfloat_pair_first(res), sqrtf(pfloat_pair_second(res)));

    return ((0.0668252-0.001) < pfloat_pair_first(res)) && ( pfloat_pair_first(res) < (0.0668252+0.001));
}

}; // hls_recurse

#endif

