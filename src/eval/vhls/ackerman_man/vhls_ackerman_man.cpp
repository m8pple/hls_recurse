#include "hls_recurse/examples/ackerman.hpp"

using namespace hls_recurse;

uint32_t vhls_ackerman_man(uint32_t m, uint32_t n)
{
    return man_ackerman(m,n);
}

int main()
{
    return test_ackerman(vhls_ackerman_man);
};
