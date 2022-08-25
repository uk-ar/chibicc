
#include "test.h"

char *x;
int main(int argc, char **argv) // 4+(4)+8=16
{
    ASSERT(10, '\n');
    ASSERT(97, 'a');
    ASSERT(3, ({printf("abc");3; }));

    ASSERT(1, ({printf("abc");1; }));
    ASSERT(2, ({printf("abc:%d",1);2; }));

    ASSERT(3, ({puts("abc");3; }));
    ASSERT(2, ({puts("abc");puts("efg");2; }));
    ASSERT(3, ({char *x;x="abc";3; }));
    ASSERT(1, ({char *x;x="abc";printVC(x,3);1; }));
    ASSERT(3, ({char *x;x="abc";puts(x);3; }));

    // global pointer
    ASSERT(97, ({x="abc";x[0]; }));
    ASSERT(98, ({x="abc";x[1]; }));

    // local pointer
    ASSERT(97, ({char *x;x="abc";x[0]; }));
    ASSERT(98, ({char *x="abc";x[1]; }));
    ASSERT(97, ({char *x;x="abc";*x; }));
    ASSERT(98, ({char *x;x="abc";*(x+1); }));
    ASSERT(3, ({char x[3];x[0]=-1;x[1]=2;int y;y=4;x[0]+y; }));

    // local array
    ASSERT(0, ({char x[3];x[0]=97;x[1]=98;x[2]=0; x[2]; }));
    ASSERT(2, ({char x[3];x[0]=97;x[1]=98;x[2]=0;printVC(x,3); 2; }));
    ASSERT(1, ({char x[3];x[0]=97;x[1]=98;x[2]=0;puts(x);1; }));

    return 0;
}