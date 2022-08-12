#include "test.h"

int a;
int main(int argc, char **argv)
{   
    ASSERT(3,({int a;a=3;a;}));//always pass;
    ASSERT(2,({int b;b=2;b;}));    
    ASSERT(14,({int a;int b;a=3;b=5*6-8;a+b/2;}));    
    ASSERT(3,({int foo;foo=3;foo;}));
    ASSERT(5,({int foo;foo=3;2+3;}));
    ASSERT(3,({int foo;foo=3;2;foo;}));
    ASSERT(3,({int foo;foo=3;2+3;foo;}));
    ASSERT(4,({int foo;foo=3;2+3;foo+1;}));
    ASSERT(6,({int foo;int bar;foo=1;bar=2+3;foo+bar;}));
    
    //global
    a=4;
    ASSERT(4,({a;}));
    ASSERT(3,({a;3;}));
    ASSERT(3,({a=1;3;}));
    ASSERT(1,({a=1;printI(a);a;}));

    a=2;
    //nested
    ASSERT(1,({int a;a=3;{a=1;}a;}));
    ASSERT(3,({int a;a=3;{int a;a=1;}a;}));

    //comment
    ASSERT(3,({//hello
    3;}));
    ASSERT(2,({/*hello\n commnent*/2;}));

   return 0;
}