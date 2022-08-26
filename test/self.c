
//#include <stdlib.h>
typedef long unsigned int size_t;
extern void *calloc(size_t __nmemb, size_t __size);
//#include <stdio.h>
extern int printf(const char *__restrict __fmt, ...);
//#include <string.h>
extern int strcmp(const char *__s1, const char *__s2);
//#include <stddef.h>
#define NULL ((void *)0)
#include "../9cc.h"

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
        // printf("hash=%d\n", hash);
        // printf("h->nodes[hash]=%p\n", h->nodes[hash]);
        for (HashNode *c = h->nodes[hash]; c; c = c->next_bucket)
        {
                // printf("c=%p\n", c);
                if (strcmp(c->key, key) == 0)
                {
                        c->value = value;
                        return NULL; // already exist
                }
        }
        h->nodes[hash] = new_hashnode(key, value, h->nodes[hash], h->begin);
        // printf("h->nodes[hash]=%p\n", h->nodes[hash]);
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
