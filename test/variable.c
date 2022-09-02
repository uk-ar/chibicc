#include "test.h"

int a = 4, *b = &a;
char f = 'a', i[] = "qux", *g = "foo";
char h[4] = "bar", *j = h;
char *k[] = {"ab", "cd"};

// int *l = ((void *)0);
int *l = (0);

int main(int argc, char **argv)
{
    ASSERT(0, strcmp("main", __func__));
    ASSERT(1, ({int a;a=3;{a=1;}a; }));
    ASSERT(4, ({ a; }));//dont reuse name scope
    ASSERT(a, 4);
    //printf("%p\n", &a);
    //({int a;printf("%p\n",&a); });
    //({int a;{printf("%p\n",&a);} });

    ASSERT(10, 0xa);//hexa
    ASSERT(9, 011);//octa
    ASSERT(-1<0, 1);
    ASSERT(0, l);
    ASSERT(0, strcmp("ab", k[0]));
    ASSERT(0, strcmp("cd", k[1]));
   // ASSERT(0, strcmp("main", __func__));
    ASSERT('b', ({ j[0]; }));
    ASSERT('b', ({ h[0]; })); // ok
    ASSERT('b', ({ *j; }));   // ok
    ASSERT(3, ({int a;a=3;a; }));
    ASSERT(2, ({int b;b=2;b; }));
    ASSERT(14, ({int a;int b;a=3;b=5*6-8;a+b/2; }));
    ASSERT(3, ({int foo;foo=3;foo; }));
    ASSERT(5, ({int foo;foo=3;2+3; }));
    ASSERT(3, ({int foo;foo=3;2;foo; }));
    ASSERT(3, ({int foo;foo=3;2+3;foo; }));
    ASSERT(4, ({int foo;foo=3;2+3;foo+1; }));
    ASSERT(6, ({int foo;int bar;foo=1;bar=2+3;foo+bar; }));
    ASSERT(3, ({int a_1;a_1=3;a_1; }));

    // assign
    ASSERT(2, ({ int a, b; a=b=2; a; }));
    ASSERT(2, ({ int a, b; a=b=2; b; }));

    int d = 4, *e = &d;
    ASSERT(4, ({ d; }));
    ASSERT(3, ({*e=3;*e; }));
    ASSERT(3, ({ d; }));

    // global
    ASSERT(4, ({ a; }));
    ASSERT(3, ({a;3; }));
    ASSERT(3, ({a=1;3; }));
    ASSERT(1, ({a=1;printI(a);a; }));
    ASSERT(3, ({*b=3;*b; }));
    ASSERT(3, ({ a; }));
    ASSERT(97, ({ f; }));
    ASSERT(102, ({ g[0]; }));
    ASSERT(98, ({ h[0]; }));
    ASSERT('q', ({ i[0]; }));
    ASSERT('b', ({ *j; }));
    a = 2;
    // nested
    ASSERT(1, ({int a;a=3;{a=1;}a; }));
    ASSERT(3, ({int a;a=3;{int a;a=1;}a; }));

    // comment*/
    //#if 0
    ASSERT(3, ({ // hello
               3;
           }));
    ASSERT(2, ({ /*hello\n commnent*/2; }));
    //#endif
    return 0;
}