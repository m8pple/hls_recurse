#include "hls_recurse/examples/sum_indexed.hpp"

using namespace hls_recurse;


void vhls_sum_indexed(uint32_t n, int32_t array[4096])
{
    #pragma HLS INTERFACE ap_memory depth=4096 port=array

    f2_sum_indexed(n, array);
}
/*
int main()
{
    return test_sum_indexed(vhls_sum_indexed);
};
*/
