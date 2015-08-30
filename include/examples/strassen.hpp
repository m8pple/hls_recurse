#ifndef strassen_hpp
#define strassen_hpp

#include "state_machine_self_recursion.hpp"

namespace hls_recurse
{

struct matrix_t
{
    int n;
    int stride;
    uint32_t *data;
};

void set(const matrix_t &a, int r, int c, uint32_t val)
{
    assert(r<a.n);
    assert(c<a.n);
    a.data[r*a.stride+c]=val;
}

uint32_t get(const matrix_t &a, int r, int c)
{
    assert(r<a.n);
    assert(c<a.n);
    return a.data[r*a.stride+c];
}

matrix_t quad(const matrix_t &src, int r, int c)
{
    assert(src.n>1);
    assert(0<=r && r<2);
    assert(0<=c && c<2);
    int hn=src.n/2;
    return matrix_t{ hn, src.stride, src.data+r*hn*src.stride+c*hn };
}

void add_matrix(const matrix_t &dst, const matrix_t &a, const matrix_t &b)
{
    assert(dst.data!=a.data);
    assert(dst.data!=b.data);

    int n=dst.n;
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            dst.data[r*dst.stride+c] = a.data[r*a.stride+c] + b.data[r*b.stride+c];
        }
    }
}

void sub_matrix(const matrix_t &dst, const matrix_t &a, const matrix_t &b)
{
    assert(dst.data!=a.data);
    assert(dst.data!=b.data);

    int n=dst.n;
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            dst.data[r*dst.stride+c] = a.data[r*a.stride+c] - b.data[r*b.stride+c];
        }
    }
}

void add_sub_add_matrix(const matrix_t &dst, const matrix_t &a, const matrix_t &b, const matrix_t &c, const matrix_t &d)
{
    int n=dst.n;
    for(int r=0; r<n; r++){
        for(int ci=0; ci<n; ci++){
            dst.data[r*dst.stride+ci] = a.data[r*a.stride+ci]
                                      + b.data[r*b.stride+ci]
                                      - c.data[r*c.stride+ci]
                                      + d.data[r*d.stride+ci];
        }
    }
}



void mul_matrix(const matrix_t &dst, const matrix_t &a, const matrix_t &b)
{
    assert(dst.data!=a.data);
    assert(dst.data!=b.data);

    int n=dst.n;
    for(int r=0; r<n; r++){
        for(int c=0; c<n; c++){
            uint32_t acc=0;
            for(int i=0; i<n; i++){
                acc += a.data[r*a.stride+i] * b.data[i*b.stride+c];
            }
            dst.data[r*dst.stride+c]=acc;
        }
    }
}


typedef uint32_t *free_region_t;

matrix_t alloc_matrix(free_region_t &h, int n)
{
    uint32_t *data=h;
    h+=n*n;

    return matrix_t{n,n,data};
}

void r_strassen(matrix_t &dst, const matrix_t &a, const matrix_t &b, free_region_t hFree)
{
    int n=a.n;

    if(n<=16){
        mul_matrix(dst,a,b);
        return;
    }

    n=n/2; // Size of quads

    matrix_t tmp1=alloc_matrix(hFree, n);
    matrix_t tmp2=alloc_matrix(hFree, n);

    matrix_t M1=alloc_matrix(hFree, n);
    add_matrix(tmp1,quad(a,0,0),quad(a,1,1));
    add_matrix(tmp2,quad(b,0,0),quad(b,1,1));
    r_strassen(M1,tmp1,tmp2, hFree);

    matrix_t M2=alloc_matrix(hFree, n);
    add_matrix(tmp1,quad(a,1,0),quad(a,1,1));
    r_strassen(M2,tmp1,quad(b,0,0), hFree);

    matrix_t M3=alloc_matrix(hFree, n);
    sub_matrix(tmp1,quad(b,0,1),quad(b,1,1));
    r_strassen(M3,quad(a,0,0),tmp1, hFree);

    matrix_t M4=alloc_matrix(hFree, n);
    sub_matrix(tmp1,quad(b,1,0),quad(b,0,0));
    r_strassen(M4,quad(a,1,1),tmp1, hFree);

    matrix_t M5=alloc_matrix(hFree, n);
    add_matrix(tmp1,quad(a,0,0),quad(a,0,1));
    r_strassen(M5,tmp1,quad(b,1,1), hFree);

    matrix_t M6=alloc_matrix(hFree, n);
    sub_matrix(tmp1,quad(a,1,0),quad(a,0,0));
    add_matrix(tmp2,quad(b,0,0),quad(b,0,1));
    r_strassen(M6,tmp1,tmp2, hFree);

    matrix_t M7=alloc_matrix(hFree, n);
    sub_matrix(tmp1,quad(a,0,1),quad(a,1,1));
    add_matrix(tmp2,quad(b,1,0),quad(b,1,1));
    r_strassen(M7,tmp1,tmp2, hFree);

    add_sub_add_matrix(quad(dst,0,0), M1, M4, M5, M7);
    add_matrix(quad(dst,0,1), M3, M5);
    add_matrix(quad(dst,1,0), M2, M4);
    add_sub_add_matrix(quad(dst,1,1), M1,M3,M2,M6);
}


class Strassen
    : public Function<Strassen, void,
            matrix_t, matrix_t, matrix_t, free_region_t,
            matrix_t, matrix_t, matrix_t, matrix_t, matrix_t, matrix_t, matrix_t, matrix_t, matrix_t
      >
{
private:
    matrix_t dst, a, b;
    free_region_t hFree;
    matrix_t M1, M2, M3, M4, M5, M6, M7;
    matrix_t tmp1, tmp2;
public:
    Strassen(stack_entry_t *stack)
        : function_base_t(*this, stack,
            dst, a, b, hFree,
            M1, M2, M3, M3, M5, M6, M7, tmp1, tmp2
        )
    {}

    template<class TVisitor>
    void VisitBody(TVisitor visitor)
    {
        visitor(
            Sequence(
                If([&](){ return dst.n <= 16; },
                    [&](){ mul_matrix(dst,a,b); },
                    Return()
                ),
                [&](){
                    tmp1=alloc_matrix(hFree, dst.n/2);
                    tmp2=alloc_matrix(hFree, dst.n/2);

                    M1=alloc_matrix(hFree, dst.n/2);
                    add_matrix(tmp1,quad(a,0,0),quad(a,1,1));
                    add_matrix(tmp2,quad(b,0,0),quad(b,1,1));
                },
                Recurse([&](){ return make_hls_state_tuple(M1,tmp1,tmp2, hFree); }),
                [&](){
                    M2=alloc_matrix(hFree, dst.n/2);
                    add_matrix(tmp1,quad(a,1,0),quad(a,1,1));
                },
                Recurse([&](){ return make_hls_state_tuple(M2,tmp1,quad(b,0,0), hFree); }),
                [&](){
                    M3=alloc_matrix(hFree, dst.n/2);
                    sub_matrix(tmp1,quad(b,0,1),quad(b,1,1));
                },
                Recurse([&](){ return make_hls_state_tuple(M3,quad(a,0,0),tmp1, hFree); } ),
                [&](){
                    M4=alloc_matrix(hFree, dst.n/2);
                    sub_matrix(tmp1,quad(b,1,0),quad(b,0,0));
                },
                Recurse([&](){ return make_hls_state_tuple(M4,quad(a,1,1),tmp1, hFree); } ),
                [&](){
                    M5=alloc_matrix(hFree, dst.n/2);
                    add_matrix(tmp1,quad(a,0,0),quad(a,0,1));
                },
                Recurse([&](){ return make_hls_state_tuple(M5,tmp1,quad(b,1,1), hFree); } ),
                [&](){
                    M6=alloc_matrix(hFree, dst.n/2);
                    sub_matrix(tmp1,quad(a,1,0),quad(a,0,0));
                    add_matrix(tmp2,quad(b,0,0),quad(b,0,1));
                },
                Recurse([&](){ return make_hls_state_tuple(M6,tmp1,tmp2, hFree); } ),
                [&](){
                    M7=alloc_matrix(hFree, dst.n/2);
                    sub_matrix(tmp1,quad(a,0,1),quad(a,1,1));
                    add_matrix(tmp2,quad(b,1,0),quad(b,1,1));
                },
                Recurse([&](){ return make_hls_state_tuple(M7,tmp1,tmp2, hFree); } ),
                [&](){
                    add_sub_add_matrix(quad(dst,0,0), M1, M4, M5, M7);
                    add_matrix(quad(dst,0,1), M3, M5);
                    add_matrix(quad(dst,1,0), M2, M4);
                    add_sub_add_matrix(quad(dst,1,1), M1,M3,M2,M6);
                }
            )
        );
    }
};

void f_strassen(matrix_t &dst, const matrix_t &a, const matrix_t &b, free_region_t hFree)
{
    matrix_t E;

    Strassen::stack_entry_t stack[64];
    Strassen strassen(stack);

    strassen(dst, a, b, hFree,  E,E,E,E,E,E,E, E,E);
}



void f2_strassen(matrix_t &_dst, const matrix_t &_a, const matrix_t &_b, free_region_t hFree)
{
    matrix_t dst=_dst, a=_a, b=_b;
    matrix_t M1, M2, M3, M4, M5, M6, M7;
    matrix_t tmp1, tmp2;
    
    run_function_old<void>(
        Sequence(
            If([&](){ return dst.n <= 16; },
                [&](){ mul_matrix(dst,a,b); },
                Return()
            ),
            [&](){
                tmp1=alloc_matrix(hFree, dst.n/2);
                tmp2=alloc_matrix(hFree, dst.n/2);

                M1=alloc_matrix(hFree, dst.n/2);
                add_matrix(tmp1,quad(a,0,0),quad(a,1,1));
                add_matrix(tmp2,quad(b,0,0),quad(b,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple(M1,tmp1,tmp2, hFree); }),
            [&](){
                M2=alloc_matrix(hFree, dst.n/2);
                add_matrix(tmp1,quad(a,1,0),quad(a,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple(M2,tmp1,quad(b,0,0), hFree); }),
            [&](){
                M3=alloc_matrix(hFree, dst.n/2);
                sub_matrix(tmp1,quad(b,0,1),quad(b,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple(M3,quad(a,0,0),tmp1, hFree); } ),
            [&](){
                M4=alloc_matrix(hFree, dst.n/2);
                sub_matrix(tmp1,quad(b,1,0),quad(b,0,0));
            },
            Recurse([&](){ return make_hls_state_tuple(M4,quad(a,1,1),tmp1, hFree); } ),
            [&](){
                M5=alloc_matrix(hFree, dst.n/2);
                add_matrix(tmp1,quad(a,0,0),quad(a,0,1));
            },
            Recurse([&](){ return make_hls_state_tuple(M5,tmp1,quad(b,1,1), hFree); } ),
            [&](){
                M6=alloc_matrix(hFree, dst.n/2);
                sub_matrix(tmp1,quad(a,1,0),quad(a,0,0));
                add_matrix(tmp2,quad(b,0,0),quad(b,0,1));
            },
            Recurse([&](){ return make_hls_state_tuple(M6,tmp1,tmp2, hFree); } ),
            [&](){
                M7=alloc_matrix(hFree, dst.n/2);
                sub_matrix(tmp1,quad(a,0,1),quad(a,1,1));
                add_matrix(tmp2,quad(b,1,0),quad(b,1,1));
            },
            Recurse([&](){ return make_hls_state_tuple(M7,tmp1,tmp2, hFree); } ),
            [&](){
                add_sub_add_matrix(quad(dst,0,0), M1, M4, M5, M7);
                add_matrix(quad(dst,0,1), M3, M5);
                add_matrix(quad(dst,1,0), M2, M4);
                add_sub_add_matrix(quad(dst,1,1), M1,M3,M2,M6);
            }
        ),
        dst, a, b, hFree,
        M1, M2, M3, M4, M5, M6, M7,
        tmp1, tmp2
    );
}

void iter_strassen(matrix_t &_dst, const matrix_t &_a, const matrix_t &_b, free_region_t hFree)
{
    mul_matrix(_dst, _a, _b);
}

uint32_t strassen_globalMem[1<<20];

template<class TImpl>
bool test_strassen(TImpl strassen)
{

    free_region_t hFree=strassen_globalMem;

    const unsigned n=128;
    
    matrix_t a=alloc_matrix(hFree, n);
    matrix_t b=alloc_matrix(hFree, n);
    matrix_t got=alloc_matrix(hFree, n);
    matrix_t ref=alloc_matrix(hFree, n);

    uint32_t f_rand=1;

    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            set(a,r,c,f_rand);
            f_rand=f_rand+1;
            set(b,r,c,-f_rand);
            f_rand=f_rand+2;
        }
    }

    /*
    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            printf(" %u", get(a,r,c));
        }
        printf("\n");
    }
    printf("\n");

    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            printf(" %u", get(b,r,c));
        }
        printf("\n");
    }
    */



    //printf("Begin standard mul...\n");
    mul_matrix(ref, a, b);
    //printf("Begin strassen mul...\n");
    strassen(got, a, b, hFree);

    /*for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            printf(" %u", get(ref,r,c));
        }
        printf("\n");
    }*/

    /*for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            printf(" %u", get(got,r,c));
        }
        printf("\n");
    }*/

    //printf("Begin check...\n");
    bool ok=true;
    for(unsigned r=0;r<n;r++){
        for(unsigned c=0;c<n;c++){
            uint32_t rr=get(ref,r,c);
            uint32_t gg=get(got,r,c);
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
