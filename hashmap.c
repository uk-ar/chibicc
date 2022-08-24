#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>
#include "9cc.h"

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
        for (HashNode *c = h->nodes[hash]; c; c = c->next_bucket)
        {
                if (strcmp(c->key, key) == 0)
                {
                        c->value = value;
                        return NULL;//already exist
                }
        }
        h->nodes[hash] = new_hashnode(key, value, h->nodes[hash], h->begin);
        h->begin = h->nodes[hash];
        return h->nodes[hash];
}

void *get_node_value(HashNode *n)
{
        return n->value;
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
