
#include "test.h"

char x;
int main(int argc, char **argv)
{
    ASSERT(8, ({int x; sizeof(&x); }));

    // ASSERT(3, sizeof(char[3]));//
    // ASSERT(24, ({sizeof(int *[3]); }));//(c) array of three pointer to int = 8*3 = 24
    ASSERT(8, ({ sizeof(int(*)[3]); })); //(d) pointer to an array of three ints = 8
    ASSERT(8, ({ sizeof(int(*)[*]); })); //(e) pointer to a variable length array of an unspecified number of ints,

    // not supported
    ASSERT(8, ({ sizeof(int *()); })); //(f) function with no parameter specification returning a pointer to int,
    // ASSERT(8, ({sizeof(int (*)(void)); }));//(g) pointer to function with no parameters returning an int,
    // ASSERT(8, ({sizeof(int (*const [])(unsigned int,...)); }));//(h) array of an unspecified number of constant pointers to functions, each with one parameter that has type unsigned int and an unspecified number of other parameters, returning an int.
    
    ASSERT(4, ({int x;sizeof(int); }));
    ASSERT(8, ({int x;sizeof(int *); }));

    ASSERT(4, ({int x;sizeof(x); }));
    ASSERT(8, ({int x;int *y;sizeof(y); }));
    ASSERT(4, ({int x;sizeof(x+3); }));
    ASSERT(8, ({int x;int *y;sizeof(y+3); }));
    ASSERT(4, ({int x;int *y;sizeof(*y); }));
    ASSERT(4, ({int x;sizeof(1); }));
    ASSERT(4, ({int x;sizeof(sizeof(1)); }));

    ASSERT(3, ({int x;int *y;x=3;y=&x;*y; }));
    ASSERT(4, ({int x;int y;distance(&y,&x); }));
    ASSERT(3, ({int x;int y;int *z;x=3;y=5;z=&y+4;*z; }));

    ASSERT(1, ({char x;sizeof(x); }));
    ASSERT(8, ({char x;char *y;sizeof(y); }));
    ASSERT(1, ({char x;char *y;sizeof(*y); }));
    ASSERT(1, ({ sizeof(x); }));
    //*/
    return 0;
}