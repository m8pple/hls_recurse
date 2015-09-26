#ifndef strassen_indexed_v2_hpp
#define strassen_indexed_v2_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

/*
    VHLS-HACK:
    This is a design that has been optimised down a bit,
    to try to get it through Vivado HLS.

*/

namespace hls_recurse
{

// We support matrices up to 2^15*2^15, and always powers of two size and stride

/*struct ipmatrix_t
{
    unsigned log2n : 4 ;
    unsigned log2stride : 4;
    unsigned base  : 24; // 24-bit word address space
};*/
/*
struct ipmatrix_t
{
    uint32_t value;

    HLS_INLINE_STEP ipmatrix_t()
        : value(0)
    {}

    HLS_INLINE_STEP ipmatrix_t(unsigned _log2n, unsigned _log2stride, unsigned _base)
        : value( (_log2n<<28) | (_log2stride<<24) | _base)
    {
        assert(_log2n<16);
        assert(_log2stride<16);
    }

    HLS_INLINE_STEP ipmatrix_log2n(unsigned) const
    { return value>>28; }

    HLS_INLINE_STEP ipmatrix_log2stride(unsigned) const
    { return (value>>24)&0xF; }

    HLS_INLINE_STEP ipmatrix_base(unsigned) const
    { return value&0xFFFFFF; }
};*/
/*
struct ipmatrix_t
{
    uint8_t m_log2n;
    uint8_t m_log2stride;
    uint32_t m_base;

    HLS_INLINE_STEP ipmatrix_t()
    {}

    HLS_INLINE_STEP ipmatrix_t(unsigned _log2n, unsigned _log2stride, unsigned _base)
        : m_log2n(_log2n)
        , m_log2stride(_log2stride)
        , m_base(_base)
    {
        assert(_log2n<16);
        assert(_log2stride<16);
    }

    HLS_INLINE_STEP ipmatrix_log2n(unsigned) const
    { return m_log2n; }

    HLS_INLINE_STEP ipmatrix_log2stride(unsigned) const
    { return m_log2stride; }

    HLS_INLINE_STEP ipmatrix_base(unsigned) const
    { return m_base; }
};*/

/*
VHLS-HACK:
    Turns out you can only put scalar types on the stack...
*/
typedef uint32_t ipmatrix_t;

HLS_INLINE_STEP ipmatrix_t ipmatrix_create(unsigned _log2n, unsigned _log2stride, unsigned _base)
{
    assert(_log2n<16);
    assert(_log2stride<16);
    return (_log2n<<28) | (_log2stride<<24) | _base;
}


HLS_INLINE_STEP unsigned ipmatrix_log2n(ipmatrix_t a)
{ return (a>>28)&0xFU; }

HLS_INLINE_STEP unsigned ipmatrix_log2stride(ipmatrix_t a)
{ return (a>>24)&0xFU; }

HLS_INLINE_STEP unsigned ipmatrix_base(ipmatrix_t a)
{ return a&0xFFFFFFu; }


HLS_INLINE_STEP void ipmatrix_set(uint32_t *p, const ipmatrix_t &m, unsigned r, unsigned c, uint32_t val)
{
    assert(r < (1<<ipmatrix_log2n(m)));
    assert(c < (1<<ipmatrix_log2n(m)));
    p[ipmatrix_base(m)+(r<<ipmatrix_log2stride(m))+c]=val;
}

HLS_INLINE_STEP uint32_t ipmatrix_get(const uint32_t *p, const ipmatrix_t &m, unsigned r, unsigned c)
{
    return p[ipmatrix_base(m)+(r<<ipmatrix_log2stride(m))+c];
}

HLS_INLINE_STEP ipmatrix_t ipmatrix_quad(const ipmatrix_t &m, unsigned r, unsigned c)
{
    assert(ipmatrix_log2n(m)>0);
    assert(0<=r && r<2);
    assert(0<=c && c<2);
    unsigned log2hn=ipmatrix_log2n(m)-1;
    return ipmatrix_create( log2hn, ipmatrix_log2stride(m), ipmatrix_base(m)+(r<<(log2hn+ipmatrix_log2stride(m)))+(c<<log2hn) );
}

HLS_INLINE_STEP void add_ipmatrix(uint32_t * p, const ipmatrix_t &dst, const ipmatrix_t &a, const ipmatrix_t &b)
{
    /*fprintf(stderr, "add_ipmatrix, dst=%u,%u,%u, a=%u,%u,%u, b=%u,%u,%u\n",
        1<<ipmatrix_log2n(dst), 1<<ipmatrix_log2stride(dst), ipmatrix_base(dst),
        1<<ipmatrix_log2n(a), 1<<ipmatrix_log2stride(a), ipmatrix_base(a),
        1<<ipmatrix_log2n(b), 1<<ipmatrix_log2stride(b), ipmatrix_base(b)
    );*/

    unsigned n=1<<ipmatrix_log2n(dst);
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            p[ipmatrix_base(dst) + (r<<ipmatrix_log2stride(dst))+c] = p[ipmatrix_base(a) + (r<<ipmatrix_log2stride(a))+c] + p[ipmatrix_base(b) + (r<<ipmatrix_log2stride(b))+c];
            //set(p, dst, r, c, get(p, a, r, c) + get(p, b, r, c));
        }
    }
}

HLS_INLINE_STEP void sub_ipmatrix(uint32_t *p, const ipmatrix_t &dst, const ipmatrix_t &a, const ipmatrix_t &b)
{
    unsigned n=1<<(ipmatrix_log2n(dst));
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            p[ipmatrix_base(dst) + (r<<ipmatrix_log2stride(dst))+c] = p[ipmatrix_base(a) + (r<<ipmatrix_log2stride(a))+c] - p[ipmatrix_base(b) + (r<<ipmatrix_log2stride(b))+c];
        }
    }
}

HLS_INLINE_STEP void add_sub_add_ipmatrix(uint32_t *p, const ipmatrix_t &dst, const ipmatrix_t &a, const ipmatrix_t &b, const ipmatrix_t &c, const ipmatrix_t &d)
{
    unsigned n=1<<ipmatrix_log2n(dst);
    for(int r=0; r<n; r++){
        for(int ci=0; ci<n; ci++){
            p[ipmatrix_base(dst) + (r<<ipmatrix_log2stride(dst))+ci] = p[ipmatrix_base(a) + (r<<ipmatrix_log2stride(a))+ci]
                                      + p[ipmatrix_base(b)+(r<<ipmatrix_log2stride(b))+ci]
                                      - p[ipmatrix_base(c)+(r<<ipmatrix_log2stride(c))+ci]
                                      + p[ipmatrix_base(d)+(r<<ipmatrix_log2stride(d))+ci];
        }
    }
}



HLS_INLINE_STEP void mul_ipmatrix(uint32_t *p, const ipmatrix_t &dst, const ipmatrix_t &a, const ipmatrix_t &b)
{
    unsigned n=1<<ipmatrix_log2n(dst);
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            uint32_t acc=0;
            for(int i=0; i<n; i++){
                //uint32_t va=p[ipmatrix_base(a)+(r<<ipmatrix_log2stride(a))+i];
                //acc += va * p[ipmatrix_base(b) + (i<<ipmatrix_log2stride(b))+c];
                acc += ipmatrix_get(p,a,r,i) * ipmatrix_get(p,b,i,c);
            }
            //p[ipmatrix_base(dst) + (r<<ipmatrix_log2stride(dst))+c]=acc;
            ipmatrix_set(p,dst,r,c, acc);
        }
    }
}


typedef int ipfree_region_t;

HLS_INLINE_STEP ipmatrix_t alloc_ipmatrix(ipfree_region_t &h, unsigned log2n)
{
    unsigned data=h;
    h+=1<<(2*log2n);

    return ipmatrix_create(log2n,log2n,data);
}

void r_strassen_indexed_v2(uint32_t *p, ipmatrix_t &dst, const ipmatrix_t &a, const ipmatrix_t &b, ipfree_region_t hFree)
{
    int log2n=ipmatrix_log2n(a);

    /*fprintf(stderr, "strassen_matrix, dst=%u,%u,%u, a=%u,%u,%u, b=%u,%u,%u\n",
        1<<ipmatrix_log2n(dst), 1<<ipmatrix_log2stride(dst), ipmatrix_base(dst),
        1<<ipmatrix_log2n(a), 1<<ipmatrix_log2stride(a), ipmatrix_base(a),
        1<<ipmatrix_log2n(b), 1<<ipmatrix_log2stride(b), ipmatrix_base(b)
    );*/

    if(log2n<=4){
        mul_ipmatrix(p, dst,a,b);
        return;
    }

    log2n=log2n-1; // Size of ipmatrix_quads

    ipmatrix_t tmp1=alloc_ipmatrix(hFree, log2n);
    ipmatrix_t tmp2=alloc_ipmatrix(hFree, log2n);

    ipmatrix_t M1=alloc_ipmatrix(hFree, log2n);
    add_ipmatrix(p, tmp1,ipmatrix_quad(a,0,0),ipmatrix_quad(a,1,1));
    add_ipmatrix(p, tmp2,ipmatrix_quad(b,0,0),ipmatrix_quad(b,1,1));
    r_strassen_indexed_v2(p, M1,tmp1,tmp2, hFree);

    ipmatrix_t M2=alloc_ipmatrix(hFree, log2n);
    add_ipmatrix(p, tmp1,ipmatrix_quad(a,1,0),ipmatrix_quad(a,1,1));
    r_strassen_indexed_v2(p, M2,tmp1,ipmatrix_quad(b,0,0), hFree);

    ipmatrix_t M3=alloc_ipmatrix(hFree, log2n);
    sub_ipmatrix(p, tmp1,ipmatrix_quad(b,0,1),ipmatrix_quad(b,1,1));
    r_strassen_indexed_v2(p, M3,ipmatrix_quad(a,0,0),tmp1, hFree);

    ipmatrix_t M4=alloc_ipmatrix(hFree, log2n);
    sub_ipmatrix(p, tmp1,ipmatrix_quad(b,1,0),ipmatrix_quad(b,0,0));
    r_strassen_indexed_v2(p, M4,ipmatrix_quad(a,1,1),tmp1, hFree);

    ipmatrix_t M5=alloc_ipmatrix(hFree, log2n);
    add_ipmatrix(p, tmp1,ipmatrix_quad(a,0,0),ipmatrix_quad(a,0,1));
    r_strassen_indexed_v2(p, M5,tmp1,ipmatrix_quad(b,1,1), hFree);

    ipmatrix_t M6=alloc_ipmatrix(hFree, log2n);
    sub_ipmatrix(p, tmp1,ipmatrix_quad(a,1,0),ipmatrix_quad(a,0,0));
    add_ipmatrix(p, tmp2,ipmatrix_quad(b,0,0),ipmatrix_quad(b,0,1));
    r_strassen_indexed_v2(p, M6,tmp1,tmp2, hFree);

    ipmatrix_t M7=alloc_ipmatrix(hFree, log2n);
    sub_ipmatrix(p, tmp1,ipmatrix_quad(a,0,1),ipmatrix_quad(a,1,1));
    add_ipmatrix(p, tmp2,ipmatrix_quad(b,1,0),ipmatrix_quad(b,1,1));
    r_strassen_indexed_v2(p, M7,tmp1,tmp2, hFree);

    add_sub_add_ipmatrix(p, ipmatrix_quad(dst,0,0), M1, M4, M5, M7);
    add_ipmatrix(p, ipmatrix_quad(dst,0,1), M3, M5);
    add_ipmatrix(p, ipmatrix_quad(dst,1,0), M2, M4);
    add_sub_add_ipmatrix(p, ipmatrix_quad(dst,1,1), M1,M3,M2,M6);
}


void f2_strassen_indexed_v2(uint32_t *p, ipmatrix_t dst, ipmatrix_t a, ipmatrix_t b, ipfree_region_t hFree)
{
    int log2n;
    ipmatrix_t tmp1, tmp2, tmp3, tmp4;
    ipmatrix_t M1, M2, M3, M4, M5, M6, M7;

    // VHLS-HACK : The number of steps made VHLS crash, so some
    // of the lambdas have been cost, at the expense of more memory
    // in use

    run_function_old<void>(
        Sequence(
            If([&](){ return ipmatrix_log2n(a)<=4; },
                [&](){ mul_ipmatrix(p, dst,a,b); },
                Return()
            ),
            [&](){
            log2n=ipmatrix_log2n(a)-1; // Size of ipmatrix_quads

            tmp1=alloc_ipmatrix(hFree, log2n);
            tmp2=alloc_ipmatrix(hFree, log2n);
            tmp3=alloc_ipmatrix(hFree, log2n);
            tmp4=alloc_ipmatrix(hFree, log2n);

            M1=alloc_ipmatrix(hFree, log2n);
            add_ipmatrix(p, tmp1,ipmatrix_quad(a,0,0),ipmatrix_quad(a,1,1));
            add_ipmatrix(p, tmp2,ipmatrix_quad(b,0,0),ipmatrix_quad(b,1,1));

            M2=alloc_ipmatrix(hFree, log2n);
            add_ipmatrix(p, tmp3,ipmatrix_quad(a,1,0),ipmatrix_quad(a,1,1));

            },
            Recurse([&](){ return make_hls_state_tuple( M1,tmp1,tmp2, hFree); }),
            Recurse([&](){ return make_hls_state_tuple( M2,tmp3,ipmatrix_quad(b,0,0), hFree); }),
            [&](){

            M3=alloc_ipmatrix(hFree, log2n);
            sub_ipmatrix(p, tmp1,ipmatrix_quad(b,0,1),ipmatrix_quad(b,1,1));

            M4=alloc_ipmatrix(hFree, log2n);
            sub_ipmatrix(p, tmp2,ipmatrix_quad(b,1,0),ipmatrix_quad(b,0,0));

            },
            Recurse([&](){ return make_hls_state_tuple( M3,ipmatrix_quad(a,0,0),tmp1, hFree); }),
            Recurse([&](){ return make_hls_state_tuple( M4,ipmatrix_quad(a,1,1),tmp2, hFree); }),
            [&](){

            M5=alloc_ipmatrix(hFree, log2n);
            add_ipmatrix(p, tmp1,ipmatrix_quad(a,0,0),ipmatrix_quad(a,0,1));

            },
            Recurse([&](){ return make_hls_state_tuple( M5,tmp1,ipmatrix_quad(b,1,1), hFree); }),
            [&](){

            M6=alloc_ipmatrix(hFree, log2n);
            sub_ipmatrix(p, tmp1,ipmatrix_quad(a,1,0),ipmatrix_quad(a,0,0));
            add_ipmatrix(p, tmp2,ipmatrix_quad(b,0,0),ipmatrix_quad(b,0,1));

            M7=alloc_ipmatrix(hFree, log2n);
            sub_ipmatrix(p, tmp3,ipmatrix_quad(a,0,1),ipmatrix_quad(a,1,1));
            add_ipmatrix(p, tmp4,ipmatrix_quad(b,1,0),ipmatrix_quad(b,1,1));

            },
            Recurse([&](){ return make_hls_state_tuple( M6,tmp1,tmp2, hFree); }),
            Recurse([&](){ return make_hls_state_tuple( M7,tmp3,tmp4, hFree); }),
            [&](){

            add_sub_add_ipmatrix(p, ipmatrix_quad(dst,0,0), M1, M4, M5, M7);
            add_ipmatrix(p, ipmatrix_quad(dst,0,1), M3, M5);
            add_ipmatrix(p, ipmatrix_quad(dst,1,0), M2, M4);
            add_sub_add_ipmatrix(p, ipmatrix_quad(dst,1,1), M1,M3,M2,M6);
            }
        ),
        dst, a, b, hFree,
        log2n, tmp1, tmp2, tmp3, tmp4,
        M1, M2, M3, M4, M5, M6, M7
    );
}

uint32_t iglobalMem[1<<20];

template<class TImpl>
bool test_strassen_indexed_v2(TImpl strassen_indexed_v2,bool logEvents=false)
{
    bool ok=true;

    for(unsigned log2n=1; log2n<=9; log2n++){

        ipfree_region_t hFree=0;
        uint32_t *p=iglobalMem;

        unsigned n=1<<log2n;

        ipmatrix_t a=alloc_ipmatrix(hFree, log2n);
        ipmatrix_t b=alloc_ipmatrix(hFree, log2n);
        ipmatrix_t got=alloc_ipmatrix(hFree, log2n);
        ipmatrix_t ref=alloc_ipmatrix(hFree, log2n);

        uint32_t f_rand=1;

        for(unsigned r=0;r<n;r++){
            for(unsigned c=0;c<n;c++){
                // Keep the values small so that it is exact
                ipmatrix_set(p, a,r,c,f_rand);
                f_rand=f_rand+1;
                ipmatrix_set(p, b,r,c,f_rand);
                f_rand=f_rand+2;
            }
        }

        /*
        for(unsigned r=0;r<n;r++){
            for(unsigned c=0;c<n;c++){
                printf(" %u", get(p, a,r,c));
            }
            printf("\n");
        }
        printf("\n");

        for(unsigned r=0;r<n;r++){
            for(unsigned c=0;c<n;c++){
                printf(" %u", get(p, b,r,c));
            }
            printf("\n");
        }
        */


        //printf("Begin standard mul...\n");
        if(n<=128){
            mul_ipmatrix(p, ref, a, b);
        }
        //printf("Begin strassen mul...\n");
        if(logEvents){
            printf("strassen_indexed_v2, n=%u, start\n", n);
        }
        strassen_indexed_v2(p, got, a, b, hFree);
        if(logEvents){
            printf("strassen_indexed_v2, n=%u, start\n", n);
        }

        if(n<=128){
            //printf("Begin check...\n");

            for(unsigned r=0;r<n;r++){
                for(unsigned c=0;c<n;c++){
                    uint32_t rr=ipmatrix_get(p, ref,r,c);
                    uint32_t gg=ipmatrix_get(p, got,r,c);
                    //printf("%d, %d : %g ref=%g\n", r,c, gg, rr);
                    if( rr!=gg ){
                        //printf("  fail\n");
                        ok=false;
                    }
                }
            }
        }
    }
    return ok;
}

}; // hls_recurse

#endif
