#include "hls_recurse/examples/sudoku.hpp"

#include <stdlib.h>

using namespace hls_recurse;

int main()
{
    if(!test_sudoku(r_sudoku)){
        fprintf(stderr, "SUDOKU/r failed.\n");
    }
    
    if(!test_sudoku(man_sudoku)){
        fprintf(stderr, "SUDOKU/manual failed.\n");
    }

    if(!test_sudoku(f_sudoku)){
        fprintf(stderr, "SUDOKU/sm failed.\n");
    }
    
    if(!test_sudoku(f2_sudoku)){
        fprintf(stderr, "SUDOKU/sm2 failed.\n");
    }

    return 0;
};
