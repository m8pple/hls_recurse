#include "hls_recurse/examples/sudoku_man.hpp"

using namespace hls_recurse;

bool vhls_sudoku_man(int puzzle[81])
{
    #pragma HLS INTERFACE ap_memory depth=81 port=puzzle
    return f2_sudoku_man(puzzle);
}

int main()
{
    return test_sudoku(vhls_sudoku_man);
};
