#ifndef sort_hpp
#define sort_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>


#include <stdlib.h>

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


void man_sort(uint32_t *a, int n)
{
    //printf("sort(%d)\n", n);
    
    const unsigned DEPTH=512;
    
    int sp=0;
    uint32_t *stack_a[DEPTH];
    int stack_n[DEPTH];
    int stack_split[DEPTH];
    int stack_state[DEPTH];
    
    stack_a[0]=a;
    stack_n[0]=n;
    stack_state[0]=0;
    
    while(1){
        a=stack_a[sp];
        n=stack_n[sp];
        int split=stack_split[sp];
        int state=stack_state[sp];
        
        /*for(int i=0;i<sp;i++){
            printf("  ");
        }
        printf("n=%d",n);*/
        
        if(state==0){
            if( n < 32 ){
                //printf(" leaf\n");
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
                if(sp==0){
                    break;
                }else{
                    sp--;
                }
            }else{
                //printf(" branch-0\n");
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
                
                stack_split[sp]=split;
                stack_state[sp]=1;
                // call r_sort(a, split)
                sp++;
                stack_state[sp]=0;
                stack_a[sp]=a;
                stack_n[sp]=split;
            }
        }else if(state==1){
            //printf(" branch-1\n");
            // complete  r_sort(a,       split);
            // call r_sort(a+split, n-split);
            stack_state[sp]=2;
            sp++;
            stack_state[sp]=0;
            stack_a[sp]=a+split;
            stack_n[sp]=n-split;
        }else if(state==2){
            //printf(" branch-2\n");
            if(sp==0){
                break;
            }else{
                sp--;
            }
        }
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
            //printf("x[%d] = %d\n", i, x[i]);
            if(x[i-1] > x[i]){
                exit(1);
                return false;
            }
        }
    }

    return true;
}

template<class T>
int harness_sort(T sort, bool logEvents=false)
{
    uint32_t x[1024];
    
    for(int i=1023;i>=0;i--){
        x[i]=1024-i;
    }
    
    sort(x, 1024);
    
    return x[0];
}

}; // hls_recurse

#endif
