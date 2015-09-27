#include "hls_recurse/examples/${DESIGN_NAME}.hpp"

#include "hls_recurse/utility.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    return harness_${DESIGN_NAME}(f2_${DESIGN_NAME});
};
