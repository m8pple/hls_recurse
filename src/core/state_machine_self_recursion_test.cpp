#include "state_machine_self_recursion.hpp"

#include <cstdio>
#include <vector>
#include <cstdlib>
#include <complex>
#include <iostream>

#include "utility.hpp"

using namespace hls_recurse;

void test1()
{
    uint32_t s1=10;

    generic_context_t<hls_state_tuple<uint32_t> > storage[16];

    v0::CallStack<void,uint32_t> stack(storage, s1);

    stack.PushContext(100);
    s1=5;
    stack.PushContext(2000);
    s1=7;

    assert(stack.PopContext()==2000);
    assert(s1==5);
    assert(stack.PopContext()==100);
    assert(s1==10);
}

void test2()
{
    uint32_t s1=10;
    float s2=100.0;

    generic_context_t<hls_state_tuple<uint32_t,float> > storage[16];
    v0::CallStack<void,uint32_t,float> stack(storage, s1,s2);

    stack.PushContext(100);
    s1=5;
    s2=50.0;
    stack.PushContext(2000);
    s1=7;
    s2=70.0;

    assert(stack.PopContext()==2000);
    assert(s1==5);
    assert(s2==50.0);
    assert(stack.PopContext()==100);
    assert(s1==10);
    assert(s2==100.0);
}

void test3()
{
    uint32_t s1=10;
    float s2=100.0;

    generic_context_t<hls_state_tuple<uint32_t,float> > storage[16];
    v0::CallStack<void,uint32_t,float> stack(storage,s1,s2);

    auto x1=[](){
        return make_hls_state_tuple(uint32_t(5), float(50.0));
    };
    RecurseStep<decltype(x1)> step1(x1);
    unsigned n=step1.step<2000,(traits_t)0>(2000, stack);
    assert(n==0);

    auto x2=[](){
        return make_hls_state_tuple(uint32_t(7), float(70.0));
    };
    RecurseStep<decltype(x2)> step2(x2);
    n=step2.step<3000,(traits_t)0>(3000, stack);
    assert(n==0);

    assert(stack.PopContext()==3001);
    assert(s1==5);
    assert(s2==50.0);

    assert(stack.PopContext()==2001);
    assert(s1==10);
    assert(s2==100.0);
}

class SumV0
    : public Function<SumV0, void, uint32_t,float*>
{
private:
    uint32_t n;
    float *f;
public:
    template<class TVisitor>
    void VisitBody(TVisitor visitor)
    {
        visitor(
            IfElse([&](){ return n>1; },
                Sequence(
                    Recurse([&](){ return make_hls_state_tuple(n/2, f); }),
                    Recurse([&](){ return make_hls_state_tuple(n-(n/2), f+n/2); }),
                    [&](){  f[0] += f[n/2];  }
                ),Sequence(
                    [](){ fprintf(stderr, "Base case\n"); }
                )
            )
        );
    }

    SumV0(stack_entry_t *stack)
        : function_base_t(*this, stack, n, f)
    {}
};

void test4()
{
    uint32_t n=4;
    float x[4]={1.0, 2.0, 3.0, 4.0};

    SumV0::stack_entry_t stack[1024];
    SumV0 sumv0(stack);

    sumv0(n,x);

    assert(x[0]==10);
}







int main()
{
    uint64_t a, b;
    
    test1();
    test2();
    test3();
    test4();
    
    
    fprintf(stderr, "\nSuccess\n");
    return 0;
};
