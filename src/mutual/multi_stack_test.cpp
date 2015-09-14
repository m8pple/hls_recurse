#include "hls_recurse/mutual/multi_stack.hpp"

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
    
    auto list_f1=Functions(
        f1
    );
        
    auto list_f2=Functions(
        f2
    );
        
    auto list_f1_f2=Functions(
        f1,
        f2
    );
         
    multi_stack<function_def_list<>> stack_empty;
    
    decltype(list_f2)::head_type::total_state_type storage_f2[1024];
    multi_stack<decltype(list_f2)> stack_f2(storage_f2, list_f2.m_head, stack_empty);
        
    decltype(list_f1_f2)::head_type::total_state_type storage_f1[1024];
    multi_stack<decltype(list_f1_f2)> stack_f1(storage_f1, list_f1.m_head, stack_f2);
    
    stack_f1.

    return 0;
}

