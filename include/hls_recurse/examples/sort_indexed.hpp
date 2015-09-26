#ifndef sort_indexed_hpp
#define sort_indexed_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

namespace hls_recurse
{

void f_sort_indexed(uint32_t *a, int n);

void f2_sort_indexed(uint32_t *a, int n)
{
    int split=0;
    int aBase=0;

    run_function_old<void>(
        IfElse([&](){ return n < 32; },
            [&]{
                // From Rosetta Code
                // http://rosettacode.org/wiki/Sorting_algorithms/Insertion_sort#C
                int i, j;
                uint32_t t;
                for (i = 1; i < n; i++) {
                    t = a[aBase+i];
                    for (j = i; j > 0 && t < a[aBase+j - 1]; j--) {
                        a[aBase+j] = a[aBase+j - 1];
                    }
                    a[aBase+j] = t;
                }
            },
            Sequence(
                [&](){
                    // http://rosettacode.org/wiki/Sorting_algorithms/Quicksort#C
                    uint32_t pivot=a[aBase+n/2];
                    int i,j;
                    for(i=0, j=n-1;; i++, j--){
                        while(a[aBase+i]<pivot)
                            i++;
                        while(pivot<a[aBase+j])
                            j--;
                        if(i>=j)
                            break;
                        uint32_t tmp=a[aBase+i];
                        a[aBase+i]=a[aBase+j];
                        a[aBase+j]=tmp;
                    }
                    split=i;
                },
                Recurse([&](){ return make_hls_state_tuple(aBase,       split, 0);     }),
                Recurse([&](){ return make_hls_state_tuple(aBase+split, n-split, 0);   })
            )
        ),
        aBase, n, split
    );
}

template<class T>
bool test_sort_indexed(T sort_indexed, bool logEvents=false)
{
    uint32_t x[8192];

    for(uint32_t n=2; n<=8192; n*=2){


        uint32_t f_rand=1;

        for(int i=0;i<n;i++){
            x[i]=f_rand;
            f_rand=f_rand+17;
            if(f_rand>33){
                f_rand-=33;
            }
           //printf("%03d : %.8f\n", i, x[i]);
        }

        if(logEvents){
            printf("sort_indexed, n=%u, start\n", n);
        }
        sort_indexed(x,n);
        if(logEvents){
            printf("sort_indexed, n=%u, finish\n", n);
        }

        //printf("%03d : %.8f\n", 0, x[0]);
        for(int i=1; i<n; i++){
            //printf("%03d : %.8f\n", i, x[i]);
            if(x[i-1] > x[i])
                return false;
        }
    }

    return true;
}


}; // hls_recurse

#endif
