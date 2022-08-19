#include "test.h"
//#include "../9cc.h"

struct s1
{
    char f1; // 1
    // padding:3
    int f2;  // 4
    char f3; // 4
};

struct s2
{
    int f3;
    int f4;
    struct s1 f5;
    struct s2 *f6; // recursive
};

struct l
{
    int var;
    struct l *next;
};

int main(int argc, char **argv)
{
    struct l *p;
    //sizeof(int);
    // p = calloc(1, sizeof(struct l));
    char start;
    struct s1 o1; // 4+8+1+4+1=18 offset
    struct s1 o2, *o3 = &o1;
    // o1.f1 = 1;
    //  ASSERT(5, distance(&o1, &start));
    //  ASSERT(5, distance(&o2, &o1));
    //  ASSERT(10, distance(&o2, &start));
    //  ASSERT(8, distance(&o3, &o2));

    ASSERT(9, sizeof(o1));
    ASSERT(0, distance(&o1, &o1.f1));
    ASSERT(1, ({ o1.f1 = 1; o1.f1 ; }));
    ASSERT(2, ({ o1.f2 = 2; o1.f2 ; }));

    printf("c:%p\n", &start);
    printf("o:%p\n", &o1);
    printf("c:%p\n", &(o1.f1));
    printf("i:%p\n", &(o1.f2));
    printf("c:%p\n", &(o2.f1));
    printf("i:%p\n", &(o2.f2));
    ASSERT(9, sizeof(o2));
    ASSERT(0, distance(&o2, &o2.f1));
    ASSERT(3, ({ o2.f2 = 3; o2.f2 ; }));
    ASSERT(4, ({ o2.f1 = 4; o2.f1 ; }));
    ASSERT(1, o1.f1);

    ASSERT(8, sizeof(o3));
    printf("c:%p\n", &(o1.f1));
    printf("i:%p\n", &(o3->f1));
    printf("i:%p\n", &(o1.f2));
    printf("i:%p\n", &(o3->f2));
    printf("i:%d\n", o1.f1);
    printf("i:%d\n", o3->f1);
    // printf("i:%p\n", o3->f2);
    ASSERT(1, o3->f1);
    ASSERT(2, o3->f2);
    // ASSERT(2, 3);
    return 0;
}