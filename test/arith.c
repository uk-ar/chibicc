#include "test.h"

int main(int argc, char **argv)
{
    {
        char c1=1,c2=2,c3=3;
        int i1=2;
        //ASSERT(1, i1 == (int)c2);
        ASSERT(131585, (int)8590066177);//0x200020201
        //ASSERT(513, (short)8590066177);
        ASSERT(1, (char)8590066177);
        ASSERT(1, (long)1);
        //ASSERT(0, (long)&*(int *)0);
        //ASSERT(513, ({ int x=512; *(char *)&x=1; x; }));
        //ASSERT(5, ({ int x=5; long y=(long)&x; *(int*)y; }));
    }//*/
    ASSERT(0, 1 > 2);
    ASSERT(3, 1 + 2);
    ASSERT(7, 10 - 1 - 2);
    ASSERT(0, 0);
    ASSERT(0, 1 > 2);
    ASSERT(4, 4);
    ASSERT(42, 42);
    ASSERT(6, 1 + 2 + 3);
    ASSERT(3, 5 - 2);
    ASSERT(7, 1 + 2 * 3);
    ASSERT(36, 1 + 2 * 3 * 5 + 2 + 3);
    ASSERT(9, 1 * 2 + (3 + 4));
    ASSERT(21, 5 + 20 - 4);
    ASSERT(10, -10 + 20);
    ASSERT(1, 1 <= 2);
    ASSERT(1, 2 == 2);
    ASSERT(0, 2 % 2);
    ASSERT(1, 1 % 2);
    ASSERT(2, 5 % 3);
    ASSERT(1, 4 % 3);
    ASSERT(0, 3 % 3);
    int i = 0, j = 0;
    ASSERT(1, ++i);
    ASSERT(-1, --j);
    i = 0, j = 0;
    ASSERT(0, i++);
    ASSERT(0, j--);
    i = 0;
    ASSERT(2, i += 2);
    ASSERT(1, i -= 1);
    ASSERT(6, i *= 6);
    ASSERT(2, i /= 3);
    ASSERT(0, i %= 2);
    return 0;
}