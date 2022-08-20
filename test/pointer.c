
#include "test.h"

char x;
int main(int argc, char **argv)
{
    ASSERT(4, ({int x;sizeof(int); }));
    //ASSERT(4, ({int x;sizeof(int *); }));
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

    return 0;
}