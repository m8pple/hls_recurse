#ifndef strassen_hpp
#define strassen_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"

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


void man_strassen(matrix_t &_dst, const matrix_t &_a, const matrix_t &_b, free_region_t _hFree)
{
    const unsigned DEPTH=512;

    //printf("strassen(%d)\n", _dst.n);

    int sp=0;
    int stack_n[DEPTH];
    matrix_t stack_dst[DEPTH];
    matrix_t stack_a[DEPTH];
    matrix_t stack_b[DEPTH];
    free_region_t stack_hFree[DEPTH];
    matrix_t stack_tmp1[DEPTH];
    matrix_t stack_tmp2[DEPTH];
    matrix_t stack_M1[DEPTH];
    matrix_t stack_M2[DEPTH];
    matrix_t stack_M3[DEPTH];
    matrix_t stack_M4[DEPTH];
    matrix_t stack_M5[DEPTH];
    matrix_t stack_M6[DEPTH];
    matrix_t stack_M7[DEPTH];
    int stack_state[DEPTH];

    stack_dst[0]=_dst;
    stack_a[0]=_a;
    stack_b[0]=_b;
    stack_hFree[0]=_hFree;
    stack_state[0]=0;

    while(1){
        int n=stack_n[sp];
        matrix_t dst=stack_dst[sp];
        matrix_t a=stack_a[sp];
        matrix_t b=stack_b[sp];
        free_region_t hFree=stack_hFree[sp];
        matrix_t tmp1=stack_tmp1[sp];
        matrix_t tmp2=stack_tmp2[sp];
        matrix_t M1=stack_M1[sp];
        matrix_t M2=stack_M2[sp];
        matrix_t M3=stack_M3[sp];
        matrix_t M4=stack_M4[sp];
        matrix_t M5=stack_M5[sp];
        matrix_t M6=stack_M6[sp];
        matrix_t M7=stack_M7[sp];
        int state=stack_state[sp];

        /*for(int i=0;i<sp;i++){
            printf("  ");
        }*/

        if(state==0){
            n=dst.n;
            stack_n[sp]=n;

            //printf("state0, n=%d", n);

            assert(sp<=8);


            if(n<=16){
                //printf(", leaf\n");
                mul_matrix(dst,a,b);
                //return;
                if(sp==0){
                    break;
                }else{
                    sp--;
                    continue;
                }
            }

            //printf(", branch\n");

            n=n/2; // Size of quads
            stack_n[sp]=n;

            tmp1=alloc_matrix(hFree, n);
            stack_tmp1[sp]=tmp1;
            stack_hFree[sp]=hFree;
            tmp2=alloc_matrix(hFree, n);
            stack_tmp2[sp]=tmp2;
            stack_hFree[sp]=hFree;

            M1=alloc_matrix(hFree, n);
            stack_M1[sp]=M1;
            stack_hFree[sp]=hFree;
            add_matrix(tmp1,quad(a,0,0),quad(a,1,1));
            add_matrix(tmp2,quad(b,0,0),quad(b,1,1));
            // call r_strassen(M1,tmp1,tmp2, hFree);
            stack_state[sp]=1;
            sp++;
            stack_state[sp]=0;
            stack_dst[sp]=M1;
            stack_a[sp]=tmp1;
            stack_b[sp]=tmp2;
            stack_hFree[sp]=hFree;
        }else if(state==1){
            //printf(", state1\n");
            M2=alloc_matrix(hFree, n);
            stack_M2[sp]=M2;
            stack_hFree[sp]=hFree;
            add_matrix(tmp1,quad(a,1,0),quad(a,1,1));
            // call r_strassen(M2,tmp1,quad(b,0,0), hFree);
            stack_state[sp]=2;
            sp++;
            stack_state[sp]=0;
            stack_dst[sp]=M2;
            stack_a[sp]=tmp1;
            stack_b[sp]=quad(b,0,0);
            stack_hFree[sp]=hFree;
        }else if(state==2){
            //printf(", state2\n");
            M3=alloc_matrix(hFree, n);
            stack_M3[sp]=M3;
            stack_hFree[sp]=hFree;
            sub_matrix(tmp1,quad(b,0,1),quad(b,1,1));
            // call r_strassen(M3,quad(a,0,0),tmp1, hFree);
            stack_state[sp]=3;
            sp++;
            stack_state[sp]=0;
            stack_dst[sp]=M3;
            stack_a[sp]=quad(a,0,0);
            stack_b[sp]=tmp1;
            stack_hFree[sp]=hFree;
        }else if(state==3){
            //printf(", state3\n");
            M4=alloc_matrix(hFree, n);
            stack_hFree[sp]=hFree;
            stack_M4[sp]=M4;
            sub_matrix(tmp1,quad(b,1,0),quad(b,0,0));
            // call r_strassen(M4,quad(a,1,1),tmp1, hFree);
            stack_state[sp]=4;
            sp++;
            stack_state[sp]=0;
            stack_dst[sp]=M4;
            stack_a[sp]=quad(a,1,1);
            stack_b[sp]=tmp1;
            stack_hFree[sp]=hFree;
        }else if(state==4){
            //printf(", state4\n");
            M5=alloc_matrix(hFree, n);
            stack_M5[sp]=M5;
            stack_hFree[sp]=hFree;
            add_matrix(tmp1,quad(a,0,0),quad(a,0,1));
            // call r_strassen(M5,tmp1,quad(b,1,1), hFree);
            stack_state[sp]=5;
            sp++;
            stack_state[sp]=0;
            stack_dst[sp]=M5;
            stack_a[sp]=tmp1;
            stack_b[sp]=quad(b,1,1);
            stack_hFree[sp]=hFree;
        }else if(state==5){
            //printf(", state5\n");
            M6=alloc_matrix(hFree, n);
            stack_M6[sp]=M6;
            stack_hFree[sp]=hFree;
            sub_matrix(tmp1,quad(a,1,0),quad(a,0,0));
            add_matrix(tmp2,quad(b,0,0),quad(b,0,1));
            // call r_strassen(M6,tmp1,tmp2, hFree);
            stack_state[sp]=6;
            sp++;
            stack_state[sp]=0;
            stack_dst[sp]=M6;
            stack_a[sp]=tmp1;
            stack_b[sp]=tmp2;
            stack_hFree[sp]=hFree;
        }else if(state==6){
            //printf(", state6\n");
            M7=alloc_matrix(hFree, n);
            stack_M7[sp]=M7;
            stack_hFree[sp]=hFree;
            sub_matrix(tmp1,quad(a,0,1),quad(a,1,1));
            add_matrix(tmp2,quad(b,1,0),quad(b,1,1));
            // call r_strassen(M7,tmp1,tmp2, hFree);
            stack_state[sp]=7;
            sp++;
            stack_state[sp]=0;
            stack_dst[sp]=M7;
            stack_a[sp]=tmp1;
            stack_b[sp]=tmp2;
            stack_hFree[sp]=hFree;
        }else{
            //printf(", state7\n");
            add_sub_add_matrix(quad(dst,0,0), M1, M4, M5, M7);
            add_matrix(quad(dst,0,1), M3, M5);
            add_matrix(quad(dst,1,0), M2, M4);
            add_sub_add_matrix(quad(dst,1,1), M1,M3,M2,M6);
            if(sp==0){
                break;
            }else{
                sp--;
            }
        }
    }
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

uint32_t strassen_globalMem[1<<21];

template<class TImpl>
bool test_strassen(TImpl strassen, bool logEvents=false)
{
    bool ok=true;

    for(unsigned n=2; n<=128; n*=2){

    free_region_t hFree=strassen_globalMem;

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



        if(n<=128){
            //printf("Begin standard mul...\n");
            mul_matrix(ref, a, b);
        }
        //printf("Begin strassen mul...\n");
        if(logEvents){
            printf("strassen, n=%u, start\n", n);
        }
        strassen(got, a, b, hFree);
        if(logEvents){
            printf("strassen, n=%u, start\n", n);
        }


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
        if(n<=128){
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
        }
    }
    return ok;
}

template<class TImpl>
int harness_strassen(TImpl strassen)
{
    const unsigned N=1<<12;

    uint32_t ram[N];

    unsigned n=64;
    free_region_t hFree=ram;

    for(int i=0;i<N;i++){
        ram[i]=i;
    }

    matrix_t a=alloc_matrix(hFree, n);
    matrix_t b=alloc_matrix(hFree, n);
    matrix_t got=alloc_matrix(hFree, n);
    matrix_t ref=alloc_matrix(hFree, n);

    strassen(got, a, b, hFree);

    int res=0;
    for(int i=0;i<N;i++){
        res+=ram[i];
    }

    return res;
}

}; // hls_recurse

#endif
