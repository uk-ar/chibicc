#include "test.h"
#include "../9cc.h"

HashMap *new_hash(int size);
void add_hash(HashMap *h, char *key, void *value);
void *get_hash(HashMap *h, char *key);
void *print_hash(HashMap *h);

//#include <stdlib.h>
typedef long unsigned int size_t;
extern void *calloc(size_t __nmemb, size_t __size);
//#include <stdio.h>
extern int printf(const char *__restrict __fmt, ...);
//#include <string.h>
extern int strcmp(const char *__s1, const char *__s2);
//#include <stddef.h>
#define NULL ((void *)0)
//#include "../9cc.h"

HashMap *new_hash(int size)
{
    HashMap *h = calloc(1, sizeof(HashMap));
    h->size = size;
    h->nodes = calloc(size, sizeof(HashNode *));
    return h;
}

HashNode *new_hashnode(char *key, void *value, HashNode *next_bucket, HashNode *next)
{
    HashNode *n = calloc(1, sizeof(HashNode));
    n->key = key;
    n->value = value;
    n->next_bucket = next_bucket;
    n->next = next;
    return n;
}

HashNode *add_hash(HashMap *h, char *key, void *value)
{
    int hash = 0;
    for (char *p = key; *p; p++)
    {
        hash += *p;
    }
    // return add_hashI(h,c,value);
    hash = hash % (h->size); // 8
    printf("hash=%d\n", hash);
    printf("h->nodes[hash]=%p\n", h->nodes[hash]);
    for (HashNode *c = h->nodes[hash]; c; c = c->next_bucket)
    {
        printf("c=%p\n", c);
        if (strcmp(c->key, key) == 0)
        {
            c->value = value;
            return NULL; // already exist
        }
    }
    h->nodes[hash] = new_hashnode(key, value, h->nodes[hash], h->begin);
    printf("h->nodes[hash]=%p\n", h->nodes[hash]);
    h->begin = h->nodes[hash];
    return h->nodes[hash];
}

void *print_hash(HashMap *h)
{
    printf("%p\n", h);
    printf("%p\n", &(h->begin));
    printf("%p\n", &(h->nodes));
    printf("%p\n", &(h->size));
    return;
}

void *print_hashnode(HashNode *n)
{
    printf("%p\n", n);
    printf("%p\n", &(n->next_bucket));
    printf("%p\n", &(n->next));
    printf("%p\n", &(n->key));
    printf("%p\n", &(n->value));
    return;
}

void *get_hash(HashMap *h, char *key)
{
    int hash = 0;
    for (char *p = key; *p; p++)
    {
        hash += *p;
    }
    hash = hash % (h->size);
    for (HashNode *c = h->nodes[hash]; c; c = c->next_bucket)
    {
        if (strcmp(c->key, key) == 0)
            return c->value;
    }
    return NULL;
    // return get_hashI(h,c);
}

int main(int argc, char **argv)
{
    HashMap a, *h = new_hash(10);
    /*HashMap a, *h = &a;
    HashNode *nodes[10];
    h->nodes = nodes;
    printf(&(h->nodes[1]));*/
    ASSERT(8, ({ distance(&(h->nodes[0]), &(h->nodes[1])); }));
    ASSERT(32, sizeof(HashNode));
    ASSERT(24, sizeof(HashMap));
    //  print_hash(h);
    //  printf("%p\n", h);
    //  printf("%p\n", &(h->begin));
    //  printf("%p\n", &(h->nodes));
    //  printf("%p\n", &(h->size));
    ASSERT(0, h->nodes == 0);
    ASSERT(0, h->begin);
    ASSERT(10, h->size);

    HashNode *n;
    n = new_hashnode("2", 4, 0, 0);
    // print_hashnode(n);
    // printf("%p\n", n);
    // printf("%p\n", &(n->next_bucket));
    // printf("%p\n", &(n->next));
    // printf("%p\n", &(n->key));
    // printf("%p\n", &(n->value));
    ASSERT(0, strcmp(n->key, "2"));
    ASSERT(4, n->value);
    ASSERT(0, n->next);
    ASSERT(0, n->next_bucket);

    n = add_hash(h, "3", 6);
    printf("n=%p\n", n);
    ASSERT(n, h->begin);
    ASSERT(6, n->value);
    ASSERT(n, h->begin);
    ASSERT(6, get_hash(h, "3"));
    ASSERT(0, strcmp(n->key, "3"));
    ASSERT(0, n->next);

    n = add_hash(h, "4", (void *)8);
    ASSERT(8, n->value);
    ASSERT(n, h->begin);
    ASSERT(8, get_hash(h, "4"));
    ASSERT(8, n->value);
    ASSERT(h->begin, n);

    n = add_hash(h, "5", (void *)10);
    ASSERT(10, get_hash(h, "5"));
    ASSERT(h->begin, n);

    n = add_hash(h, "4", (void *)12);
    ASSERT(12, get_hash(h, "4"));
    // ASSERT(h->begin, n);//over write

    HashNode *c = h->begin;
    ASSERT(10, c->value);
    c = c->next;
    ASSERT(12, (int)(c->value));
    c = c->next;
    ASSERT(6, (int)(c->value));
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
        // get_node_value(o2);
        ASSERT(32, sizeof(HashNode));
    }
    /*for (HashNode *c = h->begin; c; c = c->next)
    {
        printf("%d\n", c->value);
    }*/
    return 0;
}