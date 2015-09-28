#ifndef sudoku_man_hpp
#define sudoku_man_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

#include "hls_recurse/examples/sudoku.hpp"

namespace hls_recurse
{

bool f2_sudoku_man(int *puzzle)
{
    return man_sudoku(puzzle);
}


template<class T>
bool test_sudoku_man(T sudoku, bool logEvents=false)
{
    return test_sudoku(sudoku, logEvents);
}

template<class T>
int harness_sudoku_man(T sudoku)
{
    return harness_sudoku(sudoku);
}

}; // hls_recurse

#endif
