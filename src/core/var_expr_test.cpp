#include "var_extensions.hpp"

using namespace hls_recurse;

/*
uint32_t func(uint32_t _n)
{
    Var<uint32_t> n(_n);
    Var<uint32_t> f_n_1;
    Var<uint32_t> f_n_2;
    
    return run_function_old<uint32_t>(
        IfElse( n<=Val(2),
            Return([](){ return 1; }),
            Sequence(
                RecurseWithResult(f_n_1, n-Val(1)),
                RecurseWithResult(f_n_2, [&](){ return make_hls_state_tuple(n()-2); }),
                Return( f_n_1+f_n_2 )
            )
        ),
        n, f_n_1, f_n_2
    );
}*/

uint32_t count(uint32_t _n)
{
    uint32_t _acc=0;
    {
        Var<uint32_t> n(_n);
        Var<uint32_t> acc(_acc);
        
        return run_function_old<uint32_t>(
            Sequence(
                IfElse( n<=0,
                    Return( acc ),
                    Sequence(
                        Recurse(n-Val(1), acc+Val(1))
                    )
                )
            ),
            _n, _acc
        );
    }
}


//#include "examples/fib.hpp"

int main()
{
    //test_fib(func);
    
    std::cerr<<count(5)<<"\n";
   
    return 0;
};
