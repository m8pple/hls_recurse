#ifndef strassen_indexed_hpp
#define strassen_indexed_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

namespace hls_recurse
{

struct imatrix_t
{
    int n;
    int stride;
    int base;
};

HLS_INLINE_STEP void set(uint32_t *p, const imatrix_t &m, int r, int c, uint32_t val)
{
    assert(r < m.n);
    assert(c < m.n);
    p[m.base+r*m.stride+c]=val;
}

HLS_INLINE_STEP uint32_t get(const uint32_t *p, const imatrix_t &m, int r, int c)
{
    return p[m.base+r*m.stride+c];
}

HLS_INLINE_STEP imatrix_t quad(const imatrix_t &m, int r, int c)
{
    assert(m.n>1);
    assert(0<=r && r<2);
    assert(0<=c && c<2);
    int hn=m.n/2;
    return imatrix_t{ hn, m.stride, m.base+r*hn*m.stride+c*hn };
}

HLS_INLINE_STEP void add_matrix(uint32_t * p, const imatrix_t &dst, const imatrix_t &a, const imatrix_t &b)
{
    int n=dst.n;
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            p[dst.base + r*dst.stride+c] = p[a.base + r*a.stride+c] + p[b.base + r*b.stride+c];
        }
    }
}

HLS_INLINE_STEP void sub_matrix(uint32_t *p, const imatrix_t &dst, const imatrix_t &a, const imatrix_t &b)
{
    int n=dst.n;
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            p[dst.base + r*dst.stride+c] = p[a.base + r*a.stride+c] - p[b.base + r*b.stride+c];
        }
    }
}

HLS_INLINE_STEP void add_sub_add_matrix(uint32_t *p, const imatrix_t &dst, const imatrix_t &a, const imatrix_t &b, const imatrix_t &c, const imatrix_t &d)
{
    int n=dst.n;
    for(int r=0; r<n; r++){
        for(int ci=0; ci<n; ci++){
            p[dst.base + r*dst.stride+ci] = p[a.base + r*a.stride+ci]
                                      + p[b.base+r*b.stride+ci]
                                      - p[c.base+r*c.stride+ci]
                                      + p[d.base+r*d.stride+ci];
        }
    }
}



HLS_INLINE_STEP void mul_matrix(uint32_t *p, const imatrix_t &dst, const imatrix_t &a, const imatrix_t &b)
{
    int n=dst.n;
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            uint32_t acc=0;
            for(int i=0; i<n; i++){
                acc += p[a.base+r*a.stride+i] * p[b.base + i*b.stride+c];
            }
            p[dst.base + r*dst.stride+c]=acc;
        }
    }
}


typedef int ifree_region_t;

HLS_INLINE_STEP imatrix_t alloc_matrix(ifree_region_t &h, int n)
{
    int data=h;
    h+=n*n;

    return imatrix_t{n,n,data};
}

void r_strassen_indexed(uint32_t *p, imatrix_t &dst, const imatrix_t &a, const imatrix_t &b, ifree_region_t hFree)
{
    int n=a.n;

    if(n<=16){
        mul_matrix(p, dst,a,b);
        return;
    }

    n=n/2; // Size of quads

    imatrix_t tmp1=alloc_matrix(hFree, n);
    imatrix_t tmp2=alloc_matrix(hFree, n);

    imatrix_t M1=alloc_matrix(hFree, n);
    add_matrix(p, tmp1,quad(a,0,0),quad(a,1,1));
    add_matrix(p, tmp2,quad(b,0,0),quad(b,1,1));
    r_strassen_indexed(p, M1,tmp1,tmp2, hFree);

    imatrix_t M2=alloc_matrix(hFree, n);
    add_matrix(p, tmp1,quad(a,1,0),quad(a,1,1));
    r_strassen_indexed(p, M2,tmp1,quad(b,0,0), hFree);

    imatrix_t M3=alloc_matrix(hFree, n);
    sub_matrix(p, tmp1,quad(b,0,1),quad(b,1,1));
    r_strassen_indexed(p, M3,quad(a,0,0),tmp1, hFree);

    imatrix_t M4=alloc_matrix(hFree, n);
    sub_matrix(p, tmp1,quad(b,1,0),quad(b,0,0));
    r_strassen_indexed(p, M4,quad(a,1,1),tmp1, hFree);

    imatrix_t M5=alloc_matrix(hFree, n);
    add_matrix(p, tmp1,quad(a,0,0),quad(a,0,1));
    r_strassen_indexed(p, M5,tmp1,quad(b,1,1), hFree);

    imatrix_t M6=alloc_matrix(hFree, n);
    sub_matrix(p, tmp1,quad(a,1,0),quad(a,0,0));
    add_matrix(p, tmp2,quad(b,0,0),quad(b,0,1));
    r_strassen_indexed(p, M6,tmp1,tmp2, hFree);

    imatrix_t M7=alloc_matrix(hFree, n);
    sub_matrix(p, tmp1,quad(a,0,1),quad(a,1,1));
    add_matrix(p, tmp2,quad(b,1,0),quad(b,1,1));
    r_strassen_indexed(p, M7,tmp1,tmp2, hFree);

    add_sub_add_matrix(p, quad(dst,0,0), M1, M4, M5, M7);
    add_matrix(p, quad(dst,0,1), M3, M5);
    add_matrix(p, quad(dst,1,0), M2, M4);
    add_sub_add_matrix(p, quad(dst,1,1), M1,M3,M2,M6);
}


void f_strassen_indexed(imatrix_t &dst, const imatrix_t &a, const imatrix_t &b, ifree_region_t hFree);

void f2_strassen_indexed(uint32_t *p, imatrix_t dst, imatrix_t a, imatrix_t b, ifree_region_t hFree)
{
    int n;
    imatrix_t tmp1, tmp2;
    imatrix_t M1, M2, M3, M4, M5, M6, M7;

    run_function_old<void>(
        Sequence(
            [&](){ n=a.n; },
            If([&](){ return n<=16; },
                [&](){ mul_matrix(p, dst,a,b); },
                Return()
            ),
            [&](){
            n=n/2; // Size of quads

            tmp1=alloc_matrix(hFree, n);
            tmp2=alloc_matrix(hFree, n);

            M1=alloc_matrix(hFree, n);
            add_matrix(p, tmp1,quad(a,0,0),quad(a,1,1));
            add_matrix(p, tmp2,quad(b,0,0),quad(b,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M1,tmp1,tmp2, hFree); }),
            [&](){

            M2=alloc_matrix(hFree, n);
            add_matrix(p, tmp1,quad(a,1,0),quad(a,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M2,tmp1,quad(b,0,0), hFree); }),
            [&](){

            M3=alloc_matrix(hFree, n);
            sub_matrix(p, tmp1,quad(b,0,1),quad(b,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M3,quad(a,0,0),tmp1, hFree); }),
            [&](){

            M4=alloc_matrix(hFree, n);
            sub_matrix(p, tmp1,quad(b,1,0),quad(b,0,0));
            },
            Recurse([&](){ return make_hls_state_tuple( M4,quad(a,1,1),tmp1, hFree); }),
            [&](){

            M5=alloc_matrix(hFree, n);
            add_matrix(p, tmp1,quad(a,0,0),quad(a,0,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M5,tmp1,quad(b,1,1), hFree); }),
            [&](){

            M6=alloc_matrix(hFree, n);
            sub_matrix(p, tmp1,quad(a,1,0),quad(a,0,0));
            add_matrix(p, tmp2,quad(b,0,0),quad(b,0,1));
            },
            Recurse([&](){ return make_hls_state_tuple( M6,tmp1,tmp2, hFree); }),
            [&](){

            M7=alloc_matrix(hFree, n);
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
        n, tmp1, tmp2,
        M1, M2, M3, M4, M5, M6, M7
    );
}



uint32_t iglobalMem[1<<20];

template<class TImpl>
bool test_strassen_indexed(TImpl strassen_indexed)
{

    ifree_region_t hFree=0;
    uint32_t *p=iglobalMem;

    const unsigned n=256;

    imatrix_t a=alloc_matrix(hFree, n);
    imatrix_t b=alloc_matrix(hFree, n);
    imatrix_t got=alloc_matrix(hFree, n);
    imatrix_t ref=alloc_matrix(hFree, n);

    uint32_t f_rand=1;

    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            // Keep the values small so that it is exact
            set(p, a,r,c,f_rand);
            f_rand=f_rand+1;
            set(p, b,r,c,-f_rand);
            f_rand=f_rand+2;
        }
    }

    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            //printf(" %g", get(p, a,r,c));
        }
        //printf("\n");
    }
    //printf("\n");

    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            //printf(" %g", get(p, b,r,c));
        }
        //printf("\n");
    }



    //printf("Begin standard mul...\n");
    mul_matrix(p, ref, a, b);
    //printf("Begin strassen mul...\n");
    strassen_indexed(p, got, a, b, hFree);

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
