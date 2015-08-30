#include "state_machine_builder.hpp"

#include "state_machine_perf/while_perf.hpp"
#include "state_machine_perf/if_else_perf.hpp"
#include "state_machine_perf/sequence_perf.hpp"
#include "state_machine_perf/for_perf.hpp"

#include "utility.hpp"

int main()
{
    uint64_t a,b;
    
    a=time_now();
    if(!test_while_perf(while_perf_native)){
        printf("WHILE_PERF/native : failed.\n");
        return 1;
    }
    b=time_now();
    printf("WHILE_PERF/na : %lg\n", time_delta(a,b));
    
    a=time_now();
    if(!test_while_perf(while_perf_sm)){
        printf("WHILE_PERF/sm : failed.\n");
        return 1;
    }
    b=time_now();
    printf("WHILE_PERF/sm : %lg\n", time_delta(a,b));
    
    a=time_now();
    if(!test_while_perf(while_perf_native)){
        printf("FOR/native : failed.\n");
        return 1;
    }
    b=time_now();
    printf("FOR/na : %lg\n", time_delta(a,b));
    
    a=time_now();
    if(!test_for_perf(for_perf_sm)){
        printf("FOR/sm : failed.\n");
        return 1;
    }
    b=time_now();
    printf("FOR/sm : %lg\n", time_delta(a,b));
    
    a=time_now();
    if(!test_if_else_perf(if_else_perf_native)){
        printf("IF_ELSE/native : failed.\n");
        return 1;
    }
    b=time_now();
    printf("IF_ELSE/na : %lg\n", time_delta(a,b));
    
    a=time_now();
    if(!test_if_else_perf(if_else_perf_sm)){
        printf("IF_ELSE/sm : failed.\n");
        return 1;
    }
    b=time_now();
    printf("IF_ELSE_PERF/sm : %lg\n", time_delta(a,b));
    
    a=time_now();
    if(!test_sequence_perf(sequence_perf_native)){
        printf("SEQUENCE/native : failed.\n");
        return 1;
    }
    b=time_now();
    printf("SEQUENCE/na : %lg\n", time_delta(a,b));
    
    a=time_now();
    if(!test_sequence_perf(sequence_perf_sm)){
        printf("SEQUENCE/sm : failed.\n");
        return 1;
    }
    b=time_now();
    printf("SEQUENCE/sm : %lg\n", time_delta(a,b));
    
    printf("\nOk\n");
    
    return 0;
}
