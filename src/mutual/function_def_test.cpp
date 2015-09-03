#include "hls_recurse/mutual/function_def.hpp"

#include "hls_recurse/state_machine_builder.hpp"
#include "hls_recurse/utility.hpp"

#include <stdio.h>

using namespace hls_recurse;
using namespace hls_recurse::mutual;

int main()
{
    enum{
        F1=10,
        F2=100,
        FERR=1000
    };
    
    int i1, i2;
    int s;
    
    auto f1_body=Sequence(
        [&](){ s=i1+i2; }
    );
    
    auto f1=FunctionDef<F1,void>(
        make_hls_binding_tuple(i1, i2),
        make_hls_binding_tuple(s),
        f1_body
    );
        
    auto f2_body=Sequence(
        [&](){ s=i1; }
    );
            
    auto f2=FunctionDef<F2,void>(
        make_hls_binding_tuple(i1),
        make_hls_binding_tuple(s),
        f2_body
    );
            
    auto list_f1_f2=Functions(
        f1,
        f2
    );
            
    static_assert(list_f1_f2.total_state_count == f1_body.total_state_count+f2_body.total_state_count,
        "Body state counts do not add."
    );
        
    static_assert(list_f1_f2.template IndexOf<F1>() == 0, "Index not found.");
    static_assert(list_f1_f2.template IndexOf<F2>() == 1, "Index not found.");
    static_assert(list_f1_f2.template IndexOf<FERR>() >= mutual::detail::MAX_INDEX, "Index found.");
    
    static_assert(list_f1_f2.template StartStateOf<F1>() == 0, "Start state wrong.");
    // The +1 is to handle the implicit return code
    static_assert(list_f1_f2.template StartStateOf<F2>() == f1_body.total_state_count+1, "Start state wrong.");
    // Will not (quite correctly) compile:
    //static_assert(list_f1_f2.template StartStateOf<FERR>() >= mutual::detail::MAX_INDEX, "Index found.");

    struct context_t {} context;
        
    const traits_t traits = (traits_t)(InheritedTrait_SequenceEnd|InheritedTrait_ReturnPosition);
    const unsigned base = 5;
        
    unsigned state=list_f1_f2.template step<
        base,
        traits,
        context_t
    >(base, context);
    printf("state  = %u\n", state);
    assert(state==base+1);

    return 0;
}

