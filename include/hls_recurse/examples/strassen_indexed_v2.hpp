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

struct ipmatrix_t
{
    unsigned log2n : 4 ;
    unsigned log2stride : 4;
    unsigned base  : 24; // 24-bit word address space
};

HLS_INLINE_STEP void set(uint32_t *p, const ipmatrix_t &m, unsigned r, unsigned c, uint32_t val)
{
    assert(r < (1<<m.log2n));
    assert(c < (1<<m.log2n));
    p[m.base+(r<<m.log2stride)+c]=val;
}

HLS_INLINE_STEP uint32_t get(const uint32_t *p, const ipmatrix_t &m, unsigned r, unsigned c)
{
    return p[m.base+(r<<m.log2stride)+c];
}

HLS_INLINE_STEP ipmatrix_t quad(const ipmatrix_t &m, unsigned r, unsigned c)
{
    assert(m.log2n>0);
    assert(0<=r && r<2);
    assert(0<=c && c<2);
    unsigned log2hn=m.log2n-1;
    return ipmatrix_t{ log2hn, m.log2stride, m.base+(r<<(log2hn+m.log2stride))+(c<<log2hn) };
}

HLS_INLINE_STEP void add_matrix(uint32_t * p, const ipmatrix_t &dst, const ipmatrix_t &a, const ipmatrix_t &b)
{
    /*fprintf(stderr, "add_matrix, dst=%u,%u,%u, a=%u,%u,%u, b=%u,%u,%u\n",
        1<<dst.log2n, 1<<dst.log2stride, dst.base,
        1<<a.log2n, 1<<a.log2stride, a.base,
        1<<b.log2n, 1<<b.log2stride, b.base
    );*/
    
    unsigned n=1<<dst.log2n;
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            p[dst.base + (r<<dst.log2stride)+c] = p[a.base + (r<<a.log2stride)+c] + p[b.base + (r<<b.log2stride)+c];
            //set(p, dst, r, c, get(p, a, r, c) + get(p, b, r, c));
        }
    }
}

HLS_INLINE_STEP void sub_matrix(uint32_t *p, const ipmatrix_t &dst, const ipmatrix_t &a, const ipmatrix_t &b)
{
    unsigned n=1<<(dst.log2n);
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            p[dst.base + (r<<dst.log2stride)+c] = p[a.base + (r<<a.log2stride)+c] - p[b.base + (r<<b.log2stride)+c];
        }
    }
}

HLS_INLINE_STEP void add_sub_add_matrix(uint32_t *p, const ipmatrix_t &dst, const ipmatrix_t &a, const ipmatrix_t &b, const ipmatrix_t &c, const ipmatrix_t &d)
{
    unsigned n=1<<dst.log2n;
    for(int r=0; r<n; r++){
        for(int ci=0; ci<n; ci++){
            p[dst.base + (r<<dst.log2stride)+ci] = p[a.base + (r<<a.log2stride)+ci]
                                      + p[b.base+(r<<b.log2stride)+ci]
                                      - p[c.base+(r<<c.log2stride)+ci]
                                      + p[d.base+(r<<d.log2stride)+ci];
        }
    }
}



HLS_INLINE_STEP void mul_matrix(uint32_t *p, const ipmatrix_t &dst, const ipmatrix_t &a, const ipmatrix_t &b)
{
    unsigned n=1<<dst.log2n;
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            uint32_t acc=0;
            for(int i=0; i<n; i++){
                acc += p[a.base+(r<<a.log2stride)+i] * p[b.base + (i<<b.log2stride)+c];
            }
            p[dst.base + (r<<dst.log2stride)+c]=acc;
        }
    }
}


typedef int ipfree_region_t;

HLS_INLINE_STEP ipmatrix_t alloc_matrix(ipfree_region_t &h, unsigned log2n)
{
    unsigned data=h;
    h+=1<<(2*log2n);

    return ipmatrix_t{log2n,log2n,data};
}

void r_strassen_indexed_v2(uint32_t *p, ipmatrix_t &dst, const ipmatrix_t &a, const ipmatrix_t &b, ipfree_region_t hFree)
{
    int log2n=a.log2n;
    
    /*fprintf(stderr, "strassen_matrix, dst=%u,%u,%u, a=%u,%u,%u, b=%u,%u,%u\n",
        1<<dst.log2n, 1<<dst.log2stride, dst.base,
        1<<a.log2n, 1<<a.log2stride, a.base,
        1<<b.log2n, 1<<b.log2stride, b.base
    );*/

    if(log2n<=4){
        mul_matrix(p, dst,a,b);
        return;
    }

    log2n=log2n-1; // Size of quads

    ipmatrix_t tmp1=alloc_matrix(hFree, log2n);
    ipmatrix_t tmp2=alloc_matrix(hFree, log2n);
    
    ipmatrix_t M1=alloc_matrix(hFree, log2n);
    add_matrix(p, tmp1,quad(a,0,0),quad(a,1,1));
    add_matrix(p, tmp2,quad(b,0,0),quad(b,1,1));
    r_strassen_indexed_v2(p, M1,tmp1,tmp2, hFree);

    ipmatrix_t M2=alloc_matrix(hFree, log2n);
    add_matrix(p, tmp1,quad(a,1,0),quad(a,1,1));
    r_strassen_indexed_v2(p, M2,tmp1,quad(b,0,0), hFree);

    ipmatrix_t M3=alloc_matrix(hFree, log2n);
    sub_matrix(p, tmp1,quad(b,0,1),quad(b,1,1));
    r_strassen_indexed_v2(p, M3,quad(a,0,0),tmp1, hFree);

    ipmatrix_t M4=alloc_matrix(hFree, log2n);
    sub_matrix(p, tmp1,quad(b,1,0),quad(b,0,0));
    r_strassen_indexed_v2(p, M4,quad(a,1,1),tmp1, hFree);

    ipmatrix_t M5=alloc_matrix(hFree, log2n);
    add_matrix(p, tmp1,quad(a,0,0),quad(a,0,1));
    r_strassen_indexed_v2(p, M5,tmp1,quad(b,1,1), hFree);

    ipmatrix_t M6=alloc_matrix(hFree, log2n);
    sub_matrix(p, tmp1,quad(a,1,0),quad(a,0,0));
    add_matrix(p, tmp2,quad(b,0,0),quad(b,0,1));
    r_strassen_indexed_v2(p, M6,tmp1,tmp2, hFree);

    ipmatrix_t M7=alloc_matrix(hFree, log2n);
    sub_matrix(p, tmp1,quad(a,0,1),quad(a,1,1));
    add_matrix(p, tmp2,quad(b,1,0),quad(b,1,1));
    r_strassen_indexed_v2(p, M7,tmp1,tmp2, hFree);

    add_sub_add_matrix(p, quad(dst,0,0), M1, M4, M5, M7);
    add_matrix(p, quad(dst,0,1), M3, M5);
    add_matrix(p, quad(dst,1,0), M2, M4);
    add_sub_add_matrix(p, quad(dst,1,1), M1,M3,M2,M6);
}


void f2_strassen_indexed_v2(uint32_t *p, ipmatrix_t dst, ipmatrix_t a, ipmatrix_t b, ipfree_region_t hFree)
{
    int log2n;
    ipmatrix_t tmp1, tmp2;
    ipmatrix_t M1, M2, M3, M4, M5, M6, M7;

    run_function_old<void>(
        Sequence(
            [&](){ log2n=a.log2n; },
            If([&](){ return log2n<=4; },
                [&](){ mul_matrix(p, dst,a,b); },
                Return()
            ),
            [&](){
            log2n=log2n-1; // Size of quads

            tmp1=alloc_matrix(hFree, log2n);
            tmp2=alloc_matrix(hFree, log2n);

            M1=alloc_matrix(hFree, log2n);
            add_matrix(p, tmp1,quad(a,0,0),quad(a,1,1));
            add_matrix(p, tmp2,quad(b,0,0),quad(b,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M1,tmp1,tmp2, hFree); }),
            [&](){

            M2=alloc_matrix(hFree, log2n);
            add_matrix(p, tmp1,quad(a,1,0),quad(a,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M2,tmp1,quad(b,0,0), hFree); }),
            [&](){

            M3=alloc_matrix(hFree, log2n);
            sub_matrix(p, tmp1,quad(b,0,1),quad(b,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M3,quad(a,0,0),tmp1, hFree); }),
            [&](){

            M4=alloc_matrix(hFree, log2n);
            sub_matrix(p, tmp1,quad(b,1,0),quad(b,0,0));
            },
            Recurse([&](){ return make_hls_state_tuple( M4,quad(a,1,1),tmp1, hFree); }),
            [&](){

            M5=alloc_matrix(hFree, log2n);
            add_matrix(p, tmp1,quad(a,0,0),quad(a,0,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M5,tmp1,quad(b,1,1), hFree); }),
            [&](){

            M6=alloc_matrix(hFree, log2n);
            sub_matrix(p, tmp1,quad(a,1,0),quad(a,0,0));
            add_matrix(p, tmp2,quad(b,0,0),quad(b,0,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M6,tmp1,tmp2, hFree); }),
            [&](){

            M7=alloc_matrix(hFree, log2n);
            sub_matrix(p, tmp1,quad(a,0,1),quad(a,1,1));
            add_matrix(p, tmp2,quad(b,1,0),quad(b,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M7,tmp1,tmp2, hFree); }),
            [&](){

            add_sub_add_matrix(p, quad(dst,0,0), M1, M4, M5, M7);
            add_matrix(p, quad(dst,0,1), M3, M5);
            add_matrix(p, quad(dst,1,0), M2, M4);
            add_sub_add_matrix(p, quad(dst,1,1), M1,M3,M2,M6);
            }
        ),
        dst, a, b, hFree,
        log2n, tmp1, tmp2,
        M1, M2, M3, M4, M5, M6, M7
    );
}

uint32_t iglobalMem[1<<20];

template<class TImpl>
bool test_strassen_indexed_v2(TImpl strassen_indexed_v2)
{

    ipfree_region_t hFree=0;
    uint32_t *p=iglobalMem;

    const unsigned log2n=7;
    const unsigned n=1<<log2n;

    ipmatrix_t a=alloc_matrix(hFree, log2n);
    ipmatrix_t b=alloc_matrix(hFree, log2n);
    ipmatrix_t got=alloc_matrix(hFree, log2n);
    ipmatrix_t ref=alloc_matrix(hFree, log2n);

    uint32_t f_rand=1;

    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            // Keep the values small so that it is exact
            set(p, a,r,c,f_rand);
            f_rand=f_rand+1;
            set(p, b,r,c,f_rand);
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
    mul_matrix(p, ref, a, b);
    //printf("Begin strassen mul...\n");
    strassen_indexed_v2(p, got, a, b, hFree);

    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            //printf(" %g", get(p, ref,r,c));
        }
        //printf("\n");
    }

    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            //printf(" %g", get(p, got,r,c));
        }
        //printf("\n");
    }

    //printf("Begin check...\n");
    bool ok=true;
    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            uint32_t rr=get(p, ref,r,c);
            uint32_t gg=get(p, got,r,c);
            //printf("%d, %d : %g ref=%g\n", r,c, gg, rr);
            if( rr!=gg ){
                //printf("  fail\n");
                ok=false;
            }
        }
    }
    return ok;
}

}; // hls_recurse

#endif
