#ifndef sudoku_hpp
#define sudoku_hpp

#include "hls_recurse/state_machine_self_recursion.hpp"
#include <stdint.h>

namespace hls_recurse
{

// This code is adapted from MITCHELL JOHNSON
// June 18, 2012
// http://spin.atomicobject.com/2012/06/18/solving-sudoku-in-c-with-recursive-backtracking/
//
// http://spin.atomicobject.com/wp-content/uploads/sudokuSolver.c


/*
 * Checks to see if a particular value is presently valid in a
 * given position.
 */
/* NOTE: This should not really be inline, but it is done because
   otherwise there is no way of communicating the array depth to
    Vivado HLS.
    I tried:
    - Specifying int puzzle[81] as the type
    - #pragma Directives on the port
    */
HLS_INLINE_STEP int isValid(int number, int *puzzle, int row, int column) {
    int i=0;
    int sectorRow = 3*(row/3);
    int sectorCol = 3*(column/3);
    int row1 = (row+2)%3;
    int row2 = (row+4)%3;
    int col1 = (column+2)%3;
    int col2 = (column+4)%3;

    /* Check for the value in the given row and column */
    for (i=0; i<9; i++) {
        if (puzzle[i*9+column] == number) return false;
        if (puzzle[row*9+i] == number) return false;
    }

    /* Check the remaining four spaces in this sector */
    if(puzzle[ (row1+sectorRow)*9+col1+sectorCol] == number) return false;
    if(puzzle[ (row2+sectorRow)*9+col1+sectorCol] == number) return false;
    if(puzzle[ (row1+sectorRow)*9+col2+sectorCol] == number) return false;
    if(puzzle[ (row2+sectorRow)*9+col2+sectorCol] == number) return false;
    return true;
}

bool r_sudoku_aux(int *puzzle, int row=0, int column=0)
{
    if( 9==row ) {
        return true;
    }
    // Is this element already set?  If so, we don't want to
    // change it.  Recur immediately to the next cell.
    if( puzzle[row*9+column] ){
        if( column==8 ){
            return r_sudoku_aux(puzzle, row+1, 0);
        }else{
            return r_sudoku_aux(puzzle, row, column+1);
        }
    }
    for(int nextNum=1; nextNum<10; nextNum++)
    {
        if(isValid(nextNum, puzzle, row, column)){
            // Temporarily bind this entry to nextNum
            puzzle[row*9+column] = nextNum;
            if(column == 8){
                if(r_sudoku_aux(puzzle, row+1, 0))
                    return true;
            }else{
                if(r_sudoku_aux(puzzle, row, column+1))
                    return true;
            }
            // We failed to find a valid value for this, undo
            puzzle[row*9+column] = 0;
        }
    }
    // Have to backtrack
    return false;
}



bool r_sudoku(int *puzzle)
{
    return r_sudoku_aux(puzzle, 0, 0);
}

bool man_sudoku(int *puzzle)
{
    int row=0;
    int column=0;

    const unsigned DEPTH=512;

    int sp=0;
    int stack_row[DEPTH];
    int stack_column[DEPTH];
    int stack_nextNum[DEPTH];
    int stack_state[DEPTH];
    bool retval;

    stack_row[0]=0;
    stack_column[0]=0;
    stack_state[0]=0;

    while(1){
        row=stack_row[sp];
        column=stack_column[sp];
        int nextNum=stack_nextNum[sp];
        int state=stack_state[sp];

        /*for(int i=0;i<sp;i++){
            printf(" ");
        }
        printf("r=%d, c=%d", row, column);*/

        if(state==0){
            //printf("enter\n");

            if( 9==row ) {
                retval=true;
                if(sp==0){
                    break;
                }else{
                    sp--;
                }
                continue;
            }

            // Is this element already set?  If so, we don't want to
            // change it.  Recur immediately to the next cell.
            if( puzzle[row*9+column] ){
                if( column==8 ){
                    //return r_sudoku_aux(puzzle, row+1, 0);
                    stack_state[sp]=1;
                    sp++;
                    stack_state[sp]=0;
                    stack_row[sp]=row+1;
                    stack_column[sp]=0;
                    continue;
                }else{
                    //return r_sudoku_aux(puzzle, row, column+1);
                    stack_state[sp]=1;
                    sp++;
                    stack_state[sp]=0;
                    stack_row[sp]=row;
                    stack_column[sp]=column+1;
                    continue;
                }
            }

            stack_nextNum[sp]=1;
            stack_state[sp]=2;
        }else if(state==1){
            //printf("return0-1\n");
            if(sp==0){
                break;
            }else{
                sp--;
            }
        }else if(state==2){
            if(nextNum<10){
                //printf("loop-2\n");
                if(isValid(nextNum, puzzle, row, column)){
                    puzzle[row*9+column]=nextNum;
                    if(column==8){
                        stack_state[sp]=3;
                        sp++;
                        stack_state[sp]=0;
                        stack_row[sp]=row+1;
                        stack_column[sp]=0;
                        continue;
                    }else{
                        stack_state[sp]=3;
                        sp++;
                        stack_state[sp]=0;
                        stack_row[sp]=row;
                        stack_column[sp]=column+1;
                        continue;
                    }
                }else{
                    stack_state[sp]=2; // next iteration
                    stack_nextNum[sp]=nextNum+1;
                    continue;
                }
            }else{
                //printf("break-2\n");
                // Have to backtrack
                // return false;
                retval=false;
                if(sp==0){
                    break;
                }else{
                    sp--;
                }
            }
        }else if(state==3){
            // return from recursion
            if(retval==true){ // success
                //printf("return-3\n");
                if(sp==0){
                    break;
                }else{
                    sp--;
                }
            }else{
                //printf("backtrack-3\n");
                // We failed to find a valid value for this, undo
                puzzle[row*9+column] = 0;

                stack_state[sp]=2; // next iteration
                stack_nextNum[sp]=nextNum+1;
            }
        }
    }
    return retval;
}

class Sudoku
    : public Function<Sudoku, bool, int, int, int, bool>
{
private:
    int *puzzle;
    int row;
    int column;
    int nextNum;
    bool tmp;
public:
    Sudoku(stack_entry_t *stack, int *_puzzle)
        : function_base_t(*this, stack, row, column, nextNum, tmp)
        , puzzle(_puzzle)
    {}
public:
    template<class TVisitor>
    void VisitBody(TVisitor visitor)
    {
        visitor(
            If([&](){ return 9==row; },
                Return([&](){ return true; })
            ),
            // Is this element already set?  If so, we don't want to
            // change it.  Recur immediately to the next cell.
            If([&](){ return puzzle[row*9+column]; },
                IfElse([&](){ return column==8; },
                    RecurseAndReturn([&](){ return make_hls_state_tuple(row+1, 0); }),
                    RecurseAndReturn([&](){ return make_hls_state_tuple(row, column+1); })
                )
            ),
            // Try all possible values for the number
            [&](){ nextNum=1; },
            While([&](){ return nextNum<10; },
                If([&](){ return isValid(nextNum, puzzle, row, column); },
                    // Temporarily bind this entry to nextNum
                    [&](){ puzzle[row*9+column] = nextNum; },
                    IfElse([&](){ return column == 8; },
                        RecurseWithResult(tmp, [&](){ return make_hls_state_tuple(row+1, 0); }),
                        RecurseWithResult(tmp, [&](){ return make_hls_state_tuple(row, column+1); })
                    ),
                    If([&](){ return tmp; },
                        Return([&](){ return tmp; })
                    ),
                    // We failed to find a valid value for this, undo
                    [&](){ puzzle[row*9+column] = 0; }
                ),
                [&](){ nextNum=nextNum+1; }
            ),
            // Have to backtrack
            Return([&](){ return false; })
        );
    }
};

bool f_sudoku(int *puzzle)
{
    Sudoku::stack_entry_t stack[1024];
    Sudoku sudoku(stack, puzzle);

    return sudoku( 0,0, 0,false);
}


bool f2_sudoku(int *puzzle)
{
    int row=0;
    int column=0;
    int nextNum=0;
    bool tmp=false;

    return run_function_old<bool>(
        Sequence(
            //[&](){ printf("row=%d, col=%d, addr=%u\n", row, column, row*9+column); },
            If([&](){ return 9==row; },
                    Return([&](){ return true; })
            ),
            // Is this element already set?  If so, we don't want to
            // change it.  Recur immediately to the next cell.
            If([&](){ return puzzle[row*9+column]; },
                IfElse([&](){ return column==8; },
                    RecurseAndReturn([&](){ return make_hls_state_tuple(row+1, 0,       0); }),
                    RecurseAndReturn([&](){ return make_hls_state_tuple(row, column+1,  0); })
                )
            ),
            // Try all possible values for the number
            [&](){ nextNum=1; },
            While([&](){ return nextNum<10; },
                If([&](){ return isValid(nextNum, puzzle, row, column); },
                    // Temporarily bind this entry to nextNum
                    [&](){ puzzle[row*9+column] = nextNum; },
                    IfElse([&](){ return column == 8; },
                        RecurseWithResult(tmp, [&](){ return make_hls_state_tuple(row+1, 0,      0); }),
                        RecurseWithResult(tmp, [&](){ return make_hls_state_tuple(row, column+1, 0); })
                    ),
                    If([&](){ return tmp; },
                        Return([&](){ return tmp; })
                    ),
                    // We failed to find a valid value for this, undo
                    [&](){ puzzle[row*9+column] = 0; }
                ),
                [&](){ nextNum=nextNum+1; }
            ),
            // Have to backtrack
            Return([&](){ return false; })
       ),
       row, column, nextNum, tmp
   );
}


template<class T>
bool test_sudoku(T sudoku, bool logEvents=false)
{
    int puzzle[81]={
        0,0,3, 0,2,0, 6,0,0,
        9,0,0, 3,0,5, 0,0,1,
        0,0,1, 8,0,6, 4,0,0,
        0,0,8, 1,0,2, 9,0,0,
        7,0,0, 0,0,0, 0,0,8,
        0,0,6, 7,0,8, 2,0,0,
        0,0,2, 6,0,9, 5,0,0,
        8,0,0, 2,0,3, 0,0,9,
        0,0,5, 0,1,0, 3,0,0
    };

    unsigned n=81;
    if(logEvents){
        printf("sudoku, n=%u, start\n", n);
    }
    bool success=sudoku(puzzle);
    if(logEvents){
        printf("sudoku, n=%u, finish\n", n);
    }

    for(int i=0; i<9; i++){
        for(int j=0; j<9; j++){
            //printf("%d ", puzzle[i*9+j]);
        }
        //printf("\n");
    }
    return success;
}

template<class T>
int harness_sudoku(T sudoku)
{
    int puzzle[81]={
        0,0,3, 0,2,0, 6,0,0,
        9,0,0, 3,0,5, 0,0,1,
        0,0,1, 8,0,6, 4,0,0,
        0,0,8, 1,0,2, 9,0,0,
        7,0,0, 0,0,0, 0,0,8,
        0,0,6, 7,0,8, 2,0,0,
        0,0,2, 6,0,9, 5,0,0,
        8,0,0, 2,0,3, 0,0,9,
        0,0,5, 0,1,0, 3,0,0
    };

    sudoku(puzzle);

    return puzzle[80];
}

}; // hls_recurse

#endif
