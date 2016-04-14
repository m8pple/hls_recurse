hls_recurse
===========

- David Thomas

This is a library for implementing recursive algorithms
in variants of C that don't support stacks. The primary
target is HLS (High-Level Synthesis) tools such as
Vivado HLS and Legup, which can compile non-recursive C++ to circuits,
which are then implemented in FPGAs.

The recursion is implemented using an Embedded Domain
Specific Language (EDSL), which uses templates and lambda
functions to create something which is readable, but is
also synthesisable (can be converted into a circuit). A
classic example of a recursive function is a Fibonacci function:

    uint32_t r_fib(uint32_t n)
    {
        if(n<=2){
            return 1;
        }else{
            return r_fib(n-1)+r_fib(n-2);
        }
    }

The above function cannot be compiled by current HLS tools, as
there is no notion of a stack. However, the following _can_
be synthesis into a circuit by HLS tools, as well as being
directly executable in software:

    uint32_t f2_fib(uint32_t n)
    {
        uint32_t f_n_1, f_n_2;

        return run_function_old<uint32_t>(
            IfElse([&](){ return n<=2; },
                Return([](){ return 1; }),
                Sequence(
                    RecurseWithResult(f_n_1, [&](){ return make_hls_state_tuple(n-1); }),
                    RecurseWithResult(f_n_2, [&](){ return make_hls_state_tuple(n-2); }),
                    Return([&](){ return f_n_1+f_n_2; })
                )
            ),
            n, f_n_1, f_n_2
        );
     }

Depending on your background, you may think that looks appalling, but
it is certainly simpler than creating an explicit stack, building
a state machine, and then manually pushing and popping things.

The library has been used to implement a number of recursive
algorithms, from simple to complex:

- Fibonacci

- Ackerman Function (such a great function!)

- Prefix Sum

- QuickSort

- Tiled Matrix Multiplication

- Fast Fourier Transform

- Back-tracking Sudoko solver

- Strassen Matrix Multiplication

- Miser (adaptive Monte-Carlo integration)

If we compare the short EDSL versions with manually implemented
explicit stack versions, we find that the performance and area
is pretty much identical in Vivado HLS, while there is around
a 50% penalty in area and time for Legup.

For more details on the theory, implementation, and performance,
please have a look at the [draft paper](hls_recursion.pdf).
