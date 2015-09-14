#include "hls_recurse/examples/sort_indexed.hpp"

using namespace hls_recurse;

void vhls_sort_indexed(float *a, int n)
{
    #pragma HLS INTERFACE depth=65536 port=a

    f2_sort_indexed(a, n);
}

int main()
{
    return test_sort_indexed(vhls_sort_indexed);
};
