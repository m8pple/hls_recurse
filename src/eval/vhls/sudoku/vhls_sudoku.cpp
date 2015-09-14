#include "hls_recurse/examples/sudoku.hpp"

using namespace hls_recurse;

bool vhls_sudoku(int *puzzle)
{
    #pragma HLS INTERFACE depth=81 port=puzzle
    return f2_sudoku(puzzle);
}

int main()
{
    return test_sudoku(vhls_sudoku);
};
