#ifndef miser_hpp
#define miser_hpp

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <utility>
#include <algorithm>

#include "state_machine_self_recursion.hpp"

class XorShift128
{
    uint32_t x, y, z, w;
    
public:
    XorShift128()
        : x(123456789),y(362436069),z(521288629),w(88675123)
    {}
    
    uint32_t NextUInt()
    {
        uint32_t t;
        t=(x^(x<<11));
        x=y;
        y=z;
        z=w;
        return( w=(w^(w>>19))^(t^(t>>8)) );
    }
    
    float operator()()
    {
        return NextUInt()*0.00000000023283064365386962890625f;
    }
    
    void ranpt(float pt[], float regn[], int n)
    /*
        Returns a uniformly random point pt in an n-dimensional rectangular region. Used by miser;
        calls ran1 for uniform deviates. Your main program should initialize the global variable idum
        to a negative seed integer.
    */
    {
        int j;
        for (j=1;j<=n;j++){
            pt[j]=regn[j]+(regn[n+j]-regn[j])*(*this)();
        }
    }
};

template<class T>
class RegionAllocator
{
private:
    T *m_pFree;
    T *m_pMax;
public:
    RegionAllocator()
        : m_pFree(0)
        , m_pMax(0)
    {}

    RegionAllocator(T *pFree, unsigned n)
        : m_pFree(pFree)
        , m_pMax(pFree+n)
    {}
    
    T* alloc(unsigned n)
    {
        T *pRes=m_pFree;
        m_pFree+=n;
        assert(m_pFree < m_pMax);
        return pRes;
    }
    
    RegionAllocator NewRegion()
    {
        return RegionAllocator(*this);
    }
};

class r_miser_impl
{
public:
    template<unsigned TDim, class TFunc, class TRng>
    static std::pair<float,float> eval(TFunc func, float regn[2*TDim], unsigned long npts, float dith,  TRng &rng, RegionAllocator<float> region)
    /*
    Monte Carlo samples a user-supplied ndim-dimensional function func in a rectangular volume
    specified by regn[1..2*ndim], a vector consisting of ndim “lower-left” coordinates of the
    region followed by ndim “upper-right” coordinates. The function is sampled a total of npts
    times, at locations determined by the method of recursive stratified sampling. The mean value
    of the function in the region is returned as ave; an estimate of the statistical uncertainty of ave
    (square of standard deviation) is returned as var. The input parameter dith should normally
    be set to zero, but can be set to (e.g.) 0.1 if func’s active region falls on the boundary of a
    power-of-two subdivision of region.
    */
    {
        #define PFAC 0.1f
        #define MNPT 15
        #define MNBS 60
        #define TINY 1.0e-30f
        #define BIG 1.0e30f
        //Here PFAC is the fraction of remaining function evaluations used at each stage to explore the
        //variance of func. At least MNPT function evaluations are performed in any terminal subregion;
        //a subregion is further bisected only if at least MNBS function evaluations are available. We take
        //MNBS = 4*MNPT.
        
        
        float *regn_temp;
        
        unsigned long n,npre,nptl,nptr;
        int j,jb;
        
        float fracl,fval;
        float rgl,rgm,rgr,s,sigl,siglb,sigr,sigrb;
        float sum,sumb,summ,summ2;
        
        float pt[TDim];
        float *rmid;
        if (npts < MNBS) { // Too few points to bisect; do straight
            summ=summ2=0.0; //Monte Carlo.
            for (n=0;n<npts;n++) {
                rng.ranpt(pt,regn,TDim);
                fval=func(pt);
                summ += fval;
                summ2 += fval * fval;
            }
            return std::make_pair(
                summ/npts,
                std::max(TINY,(summ2-summ*summ/npts)/(npts*npts))
            );
        }else{ //Do the preliminary (uniform) sampling.
            rmid=region.alloc(TDim);
            npre=std::max((unsigned)(npts*PFAC),(unsigned)MNPT);

            {
                float fmaxl[TDim];
                float fmaxr[TDim];
                float fminl[TDim];
                float fminr[TDim];
                for (j=0;j<TDim;j++) { // Initialize the left and right bounds for
                    float s=rng()*dith;
                    rmid[j]=(0.5+s)*regn[j]+(0.5-s)*regn[TDim+j];
                    fminl[j]=fminr[j]=BIG;
                    fmaxl[j]=fmaxr[j] = -BIG;
                }
                for (n=0;n<npre;n++) { // Loop over the points in the sample.
                    rng.ranpt(pt,regn,TDim);
                    fval=func(pt);
                    for (j=0;j<TDim;j++) { // Find the left and right bounds for each
                        if (pt[j]<=rmid[j]) { // dimension.
                            fminl[j]=std::min(fminl[j],fval);
                            fmaxl[j]=std::max(fmaxl[j],fval);
                        }
                        else {
                            fminr[j]=std::min(fminr[j],fval);
                            fmaxr[j]=std::max(fmaxr[j],fval);
                        }
                    }
                }
                sumb=BIG; //Choose which dimension jb to bisect.
                jb=0;
                siglb=sigrb=1.0;
                for (j=0;j<TDim;j++) {
                    if (fmaxl[j] > fminl[j] && fmaxr[j] > fminr[j]) {
                        sigl=std::max(TINY,powf(fmaxl[j]-fminl[j],1.5f));
                        sigr=std::max(TINY,powf(fmaxr[j]-fminr[j],1.5f));
                        sum=sigl+sigr; //Equation (7.8.24), see text.
                        if (sum<=sumb)
                        {
                            sumb=sum;
                            jb=j;
                            siglb=sigl;
                            sigrb=sigr;
                        }
                    }
                }
            }
            if (!jb){
                jb=rng.NextUInt()%TDim;
            }
            rgl=regn[jb]; // Apportion the remaining points between
            rgm=rmid[jb]; // left and right.
            rgr=regn[TDim+jb];
            fracl=fabs((rgm-rgl)/(rgr-rgl));
            nptl=(unsigned long)(MNPT+(npts-npre-2*MNPT)*fracl*siglb
                /(fracl*siglb+(1.0-fracl)*sigrb)); //Equation (7.8.23).
            nptr=npts-npre-nptl;
            
            regn_temp=region.alloc(2*TDim); //Now allocate and integrate the two sub
            for(j=0;j<TDim;j++) { // regions.
                regn_temp[j]=regn[j];
                regn_temp[TDim+j]=regn[TDim+j];
            }
            
            regn_temp[TDim+jb]=rmid[jb];
            auto resl=eval<TDim,TFunc,TRng>(func,regn_temp,nptl,dith,rng, region.NewRegion());
            
            regn_temp[jb]=rmid[jb]; //Dispatch recursive call; will return back
            regn_temp[TDim+jb]=regn[TDim+jb]; // here eventually.
            auto resr=eval<TDim,TFunc,TRng>(func,regn_temp,nptr,dith,rng, region.NewRegion());
            
            return std::make_pair(
                fracl*resl.first+(1-fracl)*resr.first,
                fracl*fracl*resl.second+(1-fracl)*(1-fracl)*resr.second
            );
            // Combine left and right regions by equation (7.8.11) (1st line).
        }
    }
}r_miser;


class f2_miser_impl
{
public:
    template<unsigned TDim, class TFunc, class TRng>
    static std::pair<float,float> eval(TFunc func, float regn[2*TDim], unsigned long npts, float dith, TRng &rng, RegionAllocator<float> region)
    
    //Monte Carlo samples a user-supplied ndim-dimensional function func in a rectangular volume
    //specified by regn[1..2*ndim], a vector consisting of ndim “lower-left” coordinates of the
    //region followed by ndim “upper-right” coordinates. The function is sampled a total of npts
    //times, at locations determined by the method of recursive stratified sampling. The mean value
    //of the function in the region is returned as ave; an estimate of the statistical uncertainty of ave
    //(square of standard deviation) is returned as var. The input parameter dith should normally
    //be set to zero, but can be set to (e.g.) 0.1 if func’s active region falls on the boundary of a
    //power-of-two subdivision of region.
    
    {
        #define PFAC 0.1f
        #define MNPT 15
        #define MNBS 60
        #define TINY 1.0e-30f
        #define BIG 1.0e30f
        //Here PFAC is the fraction of remaining function evaluations used at each stage to explore the
        //variance of func. At least MNPT function evaluations are performed in any terminal subregion;
        //a subregion is further bisected only if at least MNBS function evaluations are available. We take
        //MNBS = 4*MNPT.
        
        
        float *regn_temp;
        
        unsigned long n,npre,nptl,nptr;
        int j,jb;
        
        float fracl,fval;
        float rgl,rgm,rgr,s,sigl,siglb,sigr,sigrb;
        float sum,sumb,summ,summ2;
        
        float pt[TDim];
        float *rmid;
        
        std::pair<float,float> resl, resr;
        
        return run_function_old<std::pair<float,float>>(
            IfElse( [&](){ return npts < MNBS; },  // Too few points to bisect; do straight
                Sequence(
                    [&](){
                        summ=summ2=0.0; //Monte Carlo.
                        for (n=0;n<npts;n++) {
                            rng.ranpt(pt,regn,TDim);
                            fval=func(pt);
                            summ += fval;
                            summ2 += fval * fval;
                        }
                    },
                    Return([&](){ return std::make_pair(
                        summ/npts,
                        std::max(TINY,(summ2-summ*summ/npts)/(npts*npts))
                    );})
                )
            ,   //Do the preliminary (uniform) sampling.
                Sequence(
                    [&](){
                        rmid=region.alloc(TDim);
                        npre=std::max((unsigned)(npts*PFAC),(unsigned)MNPT);

                        {
                            float fmaxl[TDim];
                            float fmaxr[TDim];
                            float fminl[TDim];
                            float fminr[TDim];
                            for (j=0;j<TDim;j++) { // Initialize the left and right bounds for
                                float s=rng()*dith;
                                rmid[j]=(0.5+s)*regn[j]+(0.5-s)*regn[TDim+j];
                                fminl[j]=fminr[j]=BIG;
                                fmaxl[j]=fmaxr[j] = -BIG;
                            }
                            for (n=0;n<npre;n++) { // Loop over the points in the sample.
                                rng.ranpt(pt,regn,TDim);
                                fval=func(pt);
                                for (j=0;j<TDim;j++) { // Find the left and right bounds for each
                                    if (pt[j]<=rmid[j]) { // dimension.
                                        fminl[j]=std::min(fminl[j],fval);
                                        fmaxl[j]=std::max(fmaxl[j],fval);
                                    }
                                    else {
                                        fminr[j]=std::min(fminr[j],fval);
                                        fmaxr[j]=std::max(fmaxr[j],fval);
                                    }
                                }
                            }
                            sumb=BIG; //Choose which dimension jb to bisect.
                            jb=0;
                            siglb=sigrb=1.0;
                            for (j=0;j<TDim;j++) {
                                if (fmaxl[j] > fminl[j] && fmaxr[j] > fminr[j]) {
                                    sigl=std::max(TINY,powf(fmaxl[j]-fminl[j],1.5f));
                                    sigr=std::max(TINY,powf(fmaxr[j]-fminr[j],1.5f));
                                    sum=sigl+sigr; //Equation (7.8.24), see text.
                                    if (sum<=sumb)
                                    {
                                        sumb=sum;
                                        jb=j;
                                        siglb=sigl;
                                        sigrb=sigr;
                                    }
                                }
                            }
                        }
                        if (!jb){
                            jb=rng.NextUInt()%TDim;
                        }
                        rgl=regn[jb]; // Apportion the remaining points between
                        rgm=rmid[jb]; // left and right.
                        rgr=regn[TDim+jb];
                        fracl=fabs((rgm-rgl)/(rgr-rgl));
                        nptl=(unsigned long)(MNPT+(npts-npre-2*MNPT)*fracl*siglb
                            /(fracl*siglb+(1.0-fracl)*sigrb)); //Equation (7.8.23).
                        nptr=npts-npre-nptl;
                        
                        regn_temp=region.alloc(2*TDim); //Now allocate and integrate the two sub
                        for(j=0;j<TDim;j++) { // regions.
                            regn_temp[j]=regn[j];
                            regn_temp[TDim+j]=regn[TDim+j];
                        }
                        
                        regn_temp[TDim+jb]=rmid[jb];
                    },
                    RecurseWithResult(resl, [&](){ return make_hls_state_tuple(
                        regn_temp,nptl,dith, region.NewRegion()
                    );}),
                    [&](){
                        regn_temp[jb]=rmid[jb]; //Dispatch recursive call; will return back
                        regn_temp[TDim+jb]=regn[TDim+jb]; // here eventually.
                    },
                    RecurseWithResult(resr, [&](){ return make_hls_state_tuple(
                        regn_temp,nptr,dith, region.NewRegion()
                    );}),
                    Return([&](){
                        return std::make_pair(
                            fracl*resl.first+(1-fracl)*resr.first,
                            fracl*fracl*resl.second+(1-fracl)*(1-fracl)*resr.second
                        );
                    })
                    // Combine left and right regions by equation (7.8.11) (1st line).
                )
            )
            ,
            regn, npts, dith, region,
            regn_temp, jb, resl, resr, fracl, nptl, nptr, rmid
        );
    }
} f2_miser;

template<class T>
bool test_miser(T miser)
{
    float globalMem[4096];
    
    const unsigned N = 4;
    
    auto f=[](float p[N]){
        float acc=0;
        for(unsigned i=0; i<N; i++){
            acc += p[i]*p[i];
        }
        return exp(-acc/2);
    };
    
    float regn[2*N]={0,0,0,0, 1,2,3,4};
    
    unsigned long npts=100000;
    float dith=0.05f;
    float ave, var;
    XorShift128 rng;
    RegionAllocator<float> region(globalMem, sizeof(globalMem)/sizeof(globalMem[0]));
    
    auto res=miser.template eval<N,decltype(f),decltype(rng)>(f, regn, npts, dith, rng, region);
    
    //printf("ave = %g, std = %g\n", res.first, sqrtf(res.second));
    
    return ((0.0780804-0.001) < res.first) && ( res.first < (0.0780804+0.001));
}

#endif

