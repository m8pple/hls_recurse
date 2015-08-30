#include "state_machine_builder.hpp"

#include <cstdio>

struct {} ctxt;

void test1()
{
    int x=0;
    auto f=[&x](){
        fprintf(stderr, "Fire, x=%u, &x=%p\n", x, &x);
        x=1;
    };
    
    auto s=Sequence(f);
    
    fprintf(stderr, "Init, x=%u, &x=%p\n", x, &x);
    
    assert(x==0);
    unsigned next=s.step<0,InheritedTrait_ReturnPosition>(0, ctxt);
    assert(next==1);
    assert(x==1);
    
    x=3;
    s.step<0,InheritedTrait_ReturnPosition>(0, ctxt);
    assert(x==1);
}

void test1b()
{
    int x=0;
    auto f=[&x](){
        fprintf(stderr, "Fire, x=%u, &x=%p\n", x, &x);
        x=1;
    };
    
    auto fs=FunctorStep<decltype(f)>(f);
    
    auto s=Sequence(fs);
    
    fprintf(stderr, "Init, x=%u, &x=%p\n", x, &x);
    
    assert(x==0);
    unsigned next=s.step<0,InheritedTrait_ReturnPosition>(0, ctxt);
    assert(next==1);
    assert(x==1);
    
    x=3;
    s.step<0,InheritedTrait_ReturnPosition>(0, ctxt);
    assert(x==1);
}


void test2()
{
    int x=0;
    auto f=[&](){ x++; };
    
    auto s1=FunctorStep<decltype(f)>(f);
    
    auto s2=Sequence(s1,s1);
    
    assert(x==0);
    unsigned next=s2.step<0,InheritedTrait_ReturnPosition>(0, ctxt);
    assert(next==1);
    assert(x==1);
    
    next=s2.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==2);
    
    next=1;
    next=s2.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==3);
}


void test2b()
{
    int x=0;
    auto f=[&](){ x++; };
    
    auto s2=Sequence(f,f);
    
    assert(x==0);
    unsigned next=s2.step<0,InheritedTrait_ReturnPosition>(0, ctxt);
    assert(next==1);
    assert(x==1);
    
    next=s2.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==2);
    
    next=1;
    next=s2.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==3);
}

void test2c()
{
    int x=0, y=0;
    auto f=[&](){ x++; };
    auto g=[&](){ y++; };
    
    auto s2=Sequence(f,f, g);
    
    assert(x==0);
    unsigned next=s2.step<0,InheritedTrait_ReturnPosition>(0, ctxt);
    assert(next==1);
    assert(x==1);
    
    next=s2.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==2);
    
    next=s2.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==3);
    assert(y==1);
    
    next=1;
    next=s2.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==3);
}

void test3()
{
    int x=0;
    unsigned next;
    
    auto c1=[&](){ return x%2; };
    auto f1=[&](){ x++; };
    
    auto s3=If(c1,f1);
    
    x=3;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==1);
    assert(x==3);
    
    next=s3.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==4);

    x=4;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==4);   
}

void test4()
{
    int x=0;
    unsigned next;
    
    auto c1=[&](){ return x%2; };
    auto f1=[&](){ x++; };
    auto f2=[&](){ x*=2; };
    
    auto s3=IfElse(c1,f1,f2);
    
    x=3;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==1);
    assert(x==3);
    
    next=s3.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==3);
    assert(x==4);

    x=4;
    next=0;
    next=s3.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==4);
    
    next=s3.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==3);
    assert(x==8);    
}

void test5()
{
    int x=1;
    auto condFunc=[&](){
        fprintf(stderr, "cond\n");
        return x<10;
    };
    auto bodyFunc=[&](){
        fprintf(stderr, "body\n");
        x*=3;
    };
    
    auto sWhile=While(condFunc,bodyFunc);
    
    unsigned next=0;
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(x==1);
    assert(next==1);
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==0);
    assert(x==3);
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==1);
    assert(x==3);
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==0);
    assert(x==9);
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==1);
    assert(x==9);

    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==0);
    assert(x==27);
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==27);
}

void test5b()
{
    int x=1;
    int y=0;
    auto condFunc=[&](){
        return x<10;
    };
    auto bodyFunc1=[&](){
        x*=3;
    };
    auto bodyFunc2=[&](){
        y++;
    };
    
    auto bodyStep2=Sequence(bodyFunc2);
    
    auto sWhile=
        While(condFunc,
            bodyFunc1,
            bodyStep2
        );
    
    unsigned next=0;
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(x==1);
    assert(y==0);
    assert(next==1);
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(y==0);
    assert(x==3);
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==0);
    assert(y==1);
    assert(x==3);
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==1);
    assert(y==1);
    assert(x==3);
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(y==1);
    assert(x==9);
    
    
    next=sWhile.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==0);
    assert(y==2);
    assert(x==9);
}

void test6()
{
    int rx=1, gx=1;
    int ry=0, gy=0;
    
    auto ref=[&](){
        while(rx<10){
            if(rx%2){
                ry++;
            }
            rx++;
        }
    };
    
    auto got=
        While([&](){ return gx<10; },        
            If([&](){ return gx%2; },
                [&](){ gy++; }
            ),
            [&](){ gx++; }
        );
    
    ref();
            
    unsigned state=0;
    while(state<got.total_state_count){
        state=got.step<0,InheritedTrait_ReturnPosition>(state,ctxt);
    }
   
    assert(rx==gx);
    assert(ry==gy);
}

void test6b()
{
    int rx=1, gx=1;
    int ry=0, gy=0;
    
    auto ref=[&](){
        while(1){
            if(rx>=10){
                break;
            }
            if(rx%2){
                ry++;
            }
            rx++;
        }
    };
    
    auto got=
        While([&](){ return 1; },  
            If([&](){ return gx>=10; },
                Break()
            ),
            If([&](){ return gx%2; },
                [&](){ gy++; }
            ),
            [&](){ gx++; }
        );
    
    ref();
            
    unsigned state=0;
    while(state<got.total_state_count){
        state=got.step<0,InheritedTrait_ReturnPosition>(state,ctxt);
    }
   
    assert(rx==gx);
    assert(ry==gy);
}

void test7()
{
    int x=1;
    auto condFunc=[&](){
        fprintf(stderr, "cond\n");
        return x<10;
    };
    auto bodyFunc=[&](){
        fprintf(stderr, "body\n");
        x*=3;
    };
    
    auto sWhile=While(condFunc,bodyFunc);
    auto sSeq=Sequence(sWhile);
    
    unsigned next=0;
    
    next=sSeq.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(x==1);
    assert(next==1);
    
    next=sSeq.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==0);
    assert(x==3);
    
    next=sSeq.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==1);
    assert(x==3);
    
    next=sSeq.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==0);
    assert(x==9);
    
    next=sSeq.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==1);
    assert(x==9);

    next=sSeq.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==0);
    assert(x==27);
    
    next=sSeq.step<0,InheritedTrait_ReturnPosition>(next, ctxt);
    assert(next==2);
    assert(x==27);
}

void testFor()
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
    
    auto sFor=For(initFunc, condFunc, stepFunc,
        bodyFunc
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


void testFor2()
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
    auto bodyFunc1=[&](){
        //fprintf(stderr, "body\n");
        alt*=2;
    };
    auto bodyFunc2=[&](){
        //fprintf(stderr, "body\n");
        alt*=2;
    };
    
    auto sFor=For(initFunc, condFunc, stepFunc,
        bodyFunc1,
        bodyFunc2
    );
    
    assert(sFor.total_state_count==5);
   
    unsigned next=0;
    
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==0);
    assert(next==1);
    
    // Iteration 1
   
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==0);
    assert(next==2);
    
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(alt==4);
    assert(next==4);
    
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==1);
    assert(next==1);
    
    // Iteration 2
    
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==1);
    assert(next==2);
    
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(alt==32);
    assert(next==4);
    
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==2);
    assert(next==1);
    
    // Iteration 3
    
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(x==2);
    assert(next==2);
    
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    next=sFor.step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    assert(alt==128);
    assert(next==4);
    
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
    test2b();
    test3();
    test4();
    test5();
    test5b();
    test6();
    test6b();
    test7();
    testFor();
    
    fprintf(stderr, "Ok\n");
    
  return 0;  
};
