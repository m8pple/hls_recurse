#include "hls_recurse/examples/sum_indexed_man.hpp"

using namespace hls_recurse;


void vhls_sum_indexed_man(uint32_t n, int32_t array[4096])
{
    #pragma HLS INTERFACE ap_memory depth=4096 port=array

    f2_sum_indexed_man(n, array);
}
/*
int main()
{
    return test_sum_indexed(vhls_sum_indexed);
};
*/
