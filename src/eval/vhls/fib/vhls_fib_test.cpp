#include <stdint.h>
#include <stdio.h>

uint32_t vhls_fib(uint32_t n);

bool test_fib()
{
	uint32_t aRef[]={0, 1, 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584};
    uint32_t nRef=sizeof(aRef)/sizeof(aRef[0]);

    int failed=0;
    for(unsigned i=1; i<nRef; i++){
        uint32_t got=vhls_fib(i);
        printf("got=%u\n", got);
        if(got!=aRef[i]){
            failed++;
        }
    }

    return failed==0;
}

int main()
{
	return test_fib() ? 0 : 1;
};
