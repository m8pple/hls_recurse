#ifndef sort_hpp
#define sort_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

namespace hls_recurse
{

void r_sort(uint32_t *a, int n)
{
    int split=0;

    if( n < 32 ){

        // From Rosetta Code
        // http://rosettacode.org/wiki/Sorting_algorithms/Insertion_sort#C
        /*fprintf(stderr, "n=%d\n", n);
        for(int ii=0; ii<n;ii++){
            fprintf(stderr, " %.5f", a[ii]);
        }
        fprintf(stderr, "\n");*/
        int i, j;
        uint32_t t;
        for (i = 1; i < n; i++) {
            t = a[i];
            for (j = i; j > 0 && t < a[j - 1]; j--) {
                a[j] = a[j - 1];
            }
            a[j] = t;
            /*for(int ii=0; ii<n;ii++){
                fprintf(stderr, " %.5f", a[ii]);
            }
            fprintf(stderr, "\n");*/
        }
    }else{
       // fprintf(stderr, "n=%d  ''\n", n);
        // This is taken from Rosetta Code
        // http://rosettacode.org/wiki/Sorting_algorithms/Quicksort#C
        uint32_t pivot=a[n/2];
        int i,j;
        for(i=0, j=n-1;; i++, j--){
            while(a[i]<pivot)
                i++;
            while(pivot<a[j])
                j--;
            if(i>=j)
                break;
            uint32_t tmp=a[i];
            a[i]=a[j];
            a[j]=tmp;
        }
        split=i;
        r_sort(a,       split);
        r_sort(a+split, n-split);
    }
}

class Sort
    : public Function<Sort, void, uint32_t*, int, int>
{
private:
    uint32_t *a;
    int n;
    int split;
public:
    HLS_INLINE_STEP Sort(stack_entry_t *stack)
        : function_base_t(*this,stack, a,n,split)
    {}

    template<class TVisitor>
    HLS_INLINE_STEP void VisitBody(TVisitor visitor)
    {
        visitor(
            IfElse([&](){ return n < 32; },
                [&]{
                    // From Rosetta Code
                    // http://rosettacode.org/wiki/Sorting_algorithms/Insertion_sort#C
                    int i, j;
                    uint32_t t;
                    for (i = 1; i < n; i++) {
                        t = a[i];
                        for (j = i; j > 0 && t < a[j - 1]; j--) {
                            a[j] = a[j - 1];
                        }
                        a[j] = t;
                    }
                },
                Sequence(
                    [&](){
                        // This is taken from Rosetta Code
                        // http://rosettacode.org/wiki/Sorting_algorithms/Quicksort#C
                        uint32_t pivot=a[n/2];
                        int i,j;
                        for(i=0, j=n-1;; i++, j--){
                            while(a[i]<pivot)
                                i++;
                            while(pivot<a[j])
                                j--;
                            if(i>=j)
                                break;
                            uint32_t tmp=a[i];
                            a[i]=a[j];
                            a[j]=tmp;
                        }
                        split=i;
                    },
                    Recurse([&](){ return make_hls_state_tuple(a,       split, 0);     }),
                    Recurse([&](){ return make_hls_state_tuple(a+split, n-split, 0);   })
                )
            )
        );
    }
};

void f_sort(uint32_t *a, int n)
{
    Sort::stack_entry_t stack[1024];
    Sort sort(stack);

    sort(a,n,0);
}

void f2_sort(uint32_t *a, int n)
{
    int split=0;

    run_function_old<void>(
        IfElse([&](){ return n < 32; },
            [&]{
                // From Rosetta Code
                // http://rosettacode.org/wiki/Sorting_algorithms/Insertion_sort#C
                /*fprintf(stderr, "n=%d\n", n);
                for(int ii=0; ii<n;ii++){
                    fprintf(stderr, " %.5f", a[ii]);
                }
                fprintf(stderr, "\n");*/
                int i, j;
                uint32_t t;
                for (i = 1; i < n; i++) {
                    t = a[i];
                    for (j = i; j > 0 && t < a[j - 1]; j--) {
                        a[j] = a[j - 1];
                    }
                    a[j] = t;
                    /*for(int ii=0; ii<n;ii++){
                        fprintf(stderr, " %.5f", a[ii]);
                    }
                    fprintf(stderr, "\n");*/
                }
            },
            Sequence(
                [&](){
                   // fprintf(stderr, "n=%d  ''\n", n);
                    // This is taken from Rosetta Code
                    // http://rosettacode.org/wiki/Sorting_algorithms/Quicksort#C
                    uint32_t pivot=a[n/2];
                    int i,j;
                    for(i=0, j=n-1;; i++, j--){
                        while(a[i]<pivot)
                            i++;
                        while(pivot<a[j])
                            j--;
                        if(i>=j)
                            break;
                        uint32_t tmp=a[i];
                        a[i]=a[j];
                        a[j]=tmp;
                    }
                    split=i;
                },
                Recurse([&](){ return make_hls_state_tuple(a,       split, 0);     }),
                Recurse([&](){ return make_hls_state_tuple(a+split, n-split, 0);   })
            )
        ),
        a, n, split
    );
}

template<class T>
bool test_sort(T sort, bool logEvents=false)
{
    uint32_t x[8192];

    for(uint32_t n=2; n<=8192; n*=2){


        uint32_t f_rand=1;

        for(int i=0;i<n;i++){
            x[i]=f_rand;
            f_rand=f_rand+7;
            if(f_rand>1000){
                f_rand-=501;
            }
        }

        if(logEvents){
            printf("sort, n=%u, start\n", n);
        }
        sort(x,n);
        if(logEvents){
            printf("sort, n=%u, finish\n", n);
        }

        for(int i=1; i<n; i++){
            if(x[i-1] > x[i])
                return false;
        }
    }

    return true;
}

}; // hls_recurse

#endif
