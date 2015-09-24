#include <cstdio>
#include <cstdlib>

int wooooooooooooo_i_am_a_loooooooooooooooooooong_variable=0;

int i_am_a_very_long_function_______________________weeeeeeeeeeeeeeeeeeeeeeeeeee(int i)
{
    if(i<=0){
        wooooooooooooo_i_am_a_loooooooooooooooooooong_variable++;
        return 1;
    }else{
        return 2*i_am_a_very_long_function_______________________weeeeeeeeeeeeeeeeeeeeeeeeeee(i-1)
            +
        i_am_a_very_long_function_______________________weeeeeeeeeeeeeeeeeeeeeeeeeee(i-2);
    }    
}

int main(int argc, char *argv[])
{
    int x=atoi(argv[1]);
    
    fprintf(stderr, "res=%d\n", i_am_a_very_long_function_______________________weeeeeeeeeeeeeeeeeeeeeeeeeee(x));
    fprintf(stderr, "%d\n", wooooooooooooo_i_am_a_loooooooooooooooooooong_variable);
    return 0;
}
