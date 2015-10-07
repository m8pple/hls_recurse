#include "hls_recurse/state_machine_steps.hpp"

#include <cstdio>

using namespace hls_recurse;

struct {} ctxt;

void test1()
{
    int x=0;
    auto f=[&](){ x=1; };

    auto s=FunctorStep<decltype(f)>(f);

    assert(x==0);
    unsigned next=s.step<0,InheritedTrait_ReturnPosition>(0,ctxt);
    assert(next==1);
    assert(x==1);

    x=3;
    s.step<0,InheritedTrait_ReturnPosition>(0,ctxt);
    assert(x==1);
}

#ifndef HLS_ENABLE_SHORT_CUT
void test2()
{
    int x=0;
    auto f=[&](){ x++; };

    auto s1=FunctorStep<decltype(f)>(f);

    auto s2=SequenceStep<decltype(s1),decltype(s1)>(s1,s1);

    fprintf(stderr, "1\n");
    assert(x==0);
    unsigned next=s2.step<0,InheritedTrait_ReturnPosition>(0,ctxt);
    assert(next==1);
    assert(x==1);

    fprintf(stderr, "2\n");
    next=s2.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==2);
    assert(x==2);

    next=1;
    next=s2.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==2);
    assert(x==3);
}
#else
void test2()
{
    fprintf(stderr, "ShortCut\n");

    int x=0;
    auto f=[&](){
        fprintf(stderr, "f();\n");
        x++;
    };

    auto s1=FunctorStep<decltype(f)>(f);

    auto s2=SequenceStep<decltype(s1),decltype(s1)>(s1,s1);

    assert(x==0);
    unsigned next=s2.step<0,InheritedTrait_ReturnPosition>(0,ctxt);
    fprintf(stderr, "next = %u\n", next);
    assert(next==2);
    assert(x==2);

    next=s2.step<0,InheritedTrait_ReturnPosition>(0,ctxt);
    assert(next==2);
    assert(x==4);
}
#endif

#ifndef HLS_ENABLE_SHORT_CUT
void test3()
{
    int x=0;
    unsigned next;

    auto c1=[&](){ return x%2; };
    auto f1=[&](){ x++; };

    auto s1=FunctorStep<decltype(f1)>(f1);

    auto s3=IfStep<decltype(c1),decltype(s1)>(c1,s1);

    x=3;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==1);
    assert(x==3);

    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==2);
    assert(x==4);

    x=4;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==2);
    assert(x==4);
}
#else
void test3()
{
    int x=0;
    unsigned next;

    auto c1=[&](){ return x%2; };
    auto f1=[&](){ x++; };

    auto s1=FunctorStep<decltype(f1)>(f1);

    auto s3=IfStep<decltype(c1),decltype(s1)>(c1,s1);

    x=3;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==2); // Step taken
    assert(x==4);

    x=4;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==2);
    assert(x==4);
}
#endif

#if !defined(HLS_ENABLE_SHORT_CUT)
void test4()
{
    int x=0;
    unsigned next;

    auto c1=[&](){ return x%2; };
    auto f1=[&](){ x++; };
    auto f2=[&](){ x*=2; };

    auto s1=FunctorStep<decltype(f1)>(f1);
    auto s2=FunctorStep<decltype(f2)>(f2);

    auto s3=IfElseStep<decltype(c1),decltype(s1),decltype(s2)>(c1,s1,s2);

    x=3;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==1);
    assert(x==3);

    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==3);
    assert(x==4);

    x=4;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==2);
    assert(x==4);

    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==3);
    assert(x==8);
}
#else
void test4()
{
    int x=0;
    unsigned next;

    auto c1=[&](){ return x%2; };
    auto f1=[&](){ x++; };
    auto f2=[&](){ x*=2; };

    auto s1=FunctorStep<decltype(f1)>(f1);
    auto s2=FunctorStep<decltype(f2)>(f2);

    auto s3=IfElseStep<decltype(c1),decltype(s1),decltype(s2)>(c1,s1,s2);

    x=3;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==3); // true branch
    assert(x==4);

    x=4;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==3); // false branch
    assert(x==8);
}
#endif

void test5()
{
    int x=1;
    auto condFunc=[&](){
        //fprintf(stderr, "cond\n");
        return x<10;
    };
    auto bodyFunc=[&](){
        //fprintf(stderr, "body\n");
        x*=3;
    };

    auto sBody=FunctorStep<decltype(bodyFunc)>(bodyFunc);
    auto sWhile=WhileStep<decltype(condFunc),decltype(sBody)>(condFunc,sBody);

    unsigned next=0;

    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==1);
    assert(next==1);

    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==0);
    assert(x==3);

    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==1);
    assert(x==3);

    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==0);
    assert(x==9);

    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==1);
    assert(x==9);

    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==0);
    assert(x==27);

    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==2);
    assert(x==27);
}


void test6()
{
    int x=-1;
    int alt=1;

    auto initFunc=[&](){
        x=0;
    };
    auto condFunc=[&](){
        //fprintf(stderr, "cond\n");
        return x<3;
    };
    auto stepFunc=[&](){
        x=x+1;
    };
    auto bodyFunc=[&](){
        //fprintf(stderr, "body\n");
        alt*=2;
    };

    auto sBody=FunctorStep<decltype(bodyFunc)>(bodyFunc);
    auto sFor=ForStep<decltype(initFunc),decltype(condFunc),decltype(stepFunc),decltype(sBody)>(
        initFunc,condFunc,stepFunc,sBody
    );

    assert(sFor.total_state_count==4);

    unsigned next=0;

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==0);
    assert(next==1);

    // Iteration 1

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==0);
    assert(next==2);

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(alt==2);
    assert(next==3);

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==1);
    assert(next==1);

    // Iteration 2

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==1);
    assert(next==2);

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(alt==4);
    assert(next==3);

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==2);
    assert(next==1);

    // Iteration 3

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==2);
    assert(next==2);

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(alt==8);
    assert(next==3);

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==3);
    assert(next==1);

    // out of loop

    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(next==sFor.total_state_count);

}


int main()
{
    test1();
    test2();
    test3();
    test4();
    test5();
    test6();

  return 0;
};
