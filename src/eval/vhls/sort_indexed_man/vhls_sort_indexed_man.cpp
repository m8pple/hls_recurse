#include "hls_recurse/examples/sort_indexed_man.hpp"

#include <stdlib.h>
#include <string.h>

using namespace hls_recurse;

void vhls_sort_indexed_man(uint32_t a[4096], int n)
{
    #pragma HLS INTERFACE ap_memory depth=4096 port=a

    f2_sort_indexed_man(a, n);

}

int main()
{
    return test_sort_indexed(vhls_sort_indexed_man);
};
