#include "test.h"


int f1()
{
    return 1;
}
int f2(int a)
{
    return a;
}
int f3(int a, int b)
{
    return a + b;
}
int f4()
{
    return 4 + 2;
}
int f5()
{
    int a;
    return 5;
}
int f6()
{
    int a; // duplicate variable name
    return 6;
}
int fib(int x)
{
    if (x <= 1)
    {
        return x;
    }
    return fib(x - 2) + fib(x - 1);
}
int quxx()
{
    return 3;
}
int qux()
{
    return quxx();
}

int main(int argc, char **argv)
{

    ASSERT(1, f1());
    ASSERT(2, f2(2));
    ASSERT(3, f3(1, 2));
    ASSERT(7, f3(2 + 3, 2));
    ASSERT(2, foo());
    ASSERT(3, bar(1, 2));
    ASSERT(5, bar(1 + 2, 2));
    ASSERT(3, baz(3));
    ASSERT(6, f4());

    ASSERT(1, fib(1));
    ASSERT(1, fib(2));
    ASSERT(2, fib(3));
    ASSERT(3, fib(4));
    ASSERT(5, fib(5));
    ASSERT(8, fib(6));
    ASSERT(3, qux());

    ASSERT(2, ({int *p;alloc4(&p,1,2,4,8);int *q;q=p+2;2; }));
    ASSERT(2, ({int *p;alloc4(&p,1,2,4,8);int *q;q=p+2;printI(p);printI(q); 2; }));
    ASSERT(4, ({int *p;alloc4(&p,1,2,4,8);int *q;q=p+2; *q; }));
    ASSERT(8, ({int *p;alloc4(&p,1,2,4,8);int *q;q=p+3; *q; }));

    ASSERT(1, arg2(0, 1));
    ASSERT(2, arg3(1, 1, 2));
    ASSERT(4, arg4(2, 1, 2, 4));
    ASSERT(8, arg5(3, 1, 2, 4, 8));

    ASSERT(2, ({int *p;alloc4(&p,1,2,4,8); 2; }));
    ASSERT(1, ({int *p;alloc4(&p,1,2,4,8); *p; }));
    ASSERT(3, ({int x;int *y;y=&x; 3; }));
    ASSERT(3, ({int x;int *y;y=&x;*y=3; x; }));

    ASSERT(1, ({arg2(1+2,3+4);1; }));
    ASSERT(2, ({arg3(1+2,3+4,2+3);2; }));
    return 0;
}