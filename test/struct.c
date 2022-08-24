#include "test.h"
#include "../9cc.h"

struct s1
{
    char f1; // 1
    // padding:3
    int f2; // 4
    // padding:3
    char f3; // 1
};

struct s2
{
    int f3;
    int f4;
    struct s1 f5;
    struct s2 *f6; // recursive
};
struct
{
    int anonymous;
};

typedef struct TNode node;

struct TNode
{
    int val;
    node *next;
};

typedef struct list list;

struct list
{
    node *head;
    node *tail;
};

list *list_add(list *l, int var)
{
    /*node *n;
    n = calloc(1, sizeof(node));
    node *n = calloc(1, sizeof(node));
    n->var = var;
    if (!tail)
    {
        head = n;
        tail = n;
    }
    tail->next = n;
    tail = tail->next;*/
}

typedef struct tnode TNODE;
struct tnode
{
    int count;
    TNODE *left, *right;
};

TNODE s, *sp;
typedef struct
{
    int bar;
} foo;
foo c, *d;

struct s3
{
    char f1; // 1 -> 0
    int f2;  // 4 -> 1
    long f3; // 8 -> 8
    char f4; // 1 -> 17
    // 1+(3)+4+8+1+(7)=24;
};

int main(int argc, char **argv)
{
    char start;
    struct s1 o1; // 4+8+1+4+1=18 offset
    struct s1 o2, *o3 = &o1;
    // o1.f1 = 1;
    //  ASSERT(5, distance(&o1, &start));
    //  ASSERT(5, distance(&o2, &o1));
    //  ASSERT(10, distance(&o2, &start));
    //  ASSERT(8, distance(&o3, &o2));
    {
        struct s3 a, *c = &a;          // 4+(4)+8+24=40
        int b;                         // 4+(4)+8+24+4+(4)=48
        ASSERT(24, sizeof(struct s3)); // 81->24
        ASSERT(&(a.f1), &(c->f1));
        ASSERT(&(a.f2), &(c->f2));
        ASSERT(&(a.f3), &(c->f3));
        ASSERT(&(a.f4), &(c->f4));
    }
    {
        HashNode o1, *o2 = &o1;
        // ASSERT(32, sizeof(HashNode));
        ASSERT(&(o1.next_bucket), &(o2->next_bucket));
        ASSERT(&(o1.next), &(o2->next));
        ASSERT(&(o1.key), &(o2->key));
        ASSERT(&(o1.value), &(o2->value));
        printf("actual\n");
        printf("%p\n", o2);
        printf("%p\n", &(o2->next_bucket));
        printf("%p\n", &(o2->next));
        printf("%p\n", &(o2->key));
        printf("%p\n", &(o2->value));
        printf("expected\n");
        get_node_value(o2);
        ASSERT(32, sizeof(HashNode));
    }
    ASSERT(12, sizeof(o1)); // 1+(3)+4+1+(3)
    ASSERT(0, distance(&o1, &o1.f1));
    ASSERT(1, ({ o1.f1 = 1; o1.f1 ; }));
    ASSERT(2, ({ o1.f2 = 2; o1.f2 ; }));

    printf("c:%p\n", &start);
    printf("o:%p\n", &o1);
    printf("c:%p\n", &(o1.f1));
    printf("i:%p\n", &(o1.f2));
    printf("c:%p\n", &(o2.f1));
    printf("i:%p\n", &(o2.f2));
    ASSERT(12, sizeof(o2));
    ASSERT(0, distance(&o2, &o2.f1));
    ASSERT(3, ({ o2.f2 = 3; o2.f2 ; }));
    ASSERT(4, ({ o2.f1 = 4; o2.f1 ; }));
    ASSERT(1, o1.f1);

    ASSERT(16, sizeof(node));
    ASSERT(16, sizeof(list));
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

    struct s2 a, c, *b = &a, d;
    ASSERT(1, ({ a.f3 = 1; a.f3; }));
    ASSERT(1, ({ b->f3; }));
    ASSERT(2, ({ c.f3 = 2; c.f3; }));
    // ASSERT(2, ({ c.3}));//raise error

    d.f3 = 3;
    // TODO: copy struct
    // a.f5 = d;
    // ASSERT(3, ({ a.f5->f3; }));
    // ASSERT(3, ({ b->f5->f3; }));
    a.f6 = &c;
    ASSERT(2, ({ b->f6->f3; }));
    ASSERT(2, ({ a.f6->f3; }));
    // ASSERT(2, ({ a.next->val; }));
    //  ASSERT(2, 3);
    // struct list l = calloc(1, sizeof(struct list));
    //  ASSERT(16, sizeof(Node));
    return 0;
}