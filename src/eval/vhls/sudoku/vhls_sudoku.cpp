#include "hls_recurse/examples/sudoku.hpp"

using namespace hls_recurse;

bool vhls_sudoku(int puzzle[81])
{
    #pragma HLS INTERFACE ap_memory depth=81 port=puzzle
    return f2_sudoku(puzzle);
}

int main()
{
    return test_sudoku(vhls_sudoku);
};
