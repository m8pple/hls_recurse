#include "examples/ackerman.hpp"

#include <stdlib.h>

int main()
{
    if(!test_ackerman(r_ackerman)){
        fprintf(stderr, "ACKERMAN/recursive failed.\n");
    }

    if(!test_ackerman(f_ackerman)){
        fprintf(stderr, "ACKERMAN/state failed.\n");
    }

    if(!test_ackerman(f2_ackerman)){
        fprintf(stderr, "ACKERMAN/state2 failed.\n");
    }

    return 0;
};
