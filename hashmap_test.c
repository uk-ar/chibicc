//#include "test/test.h"
#include "9cc.h"
#include <assert.h>

HashMap *new_hash(int size);
void add_hash(HashMap *h, char *key, void *value);
void *get_hash(HashMap *h, char *key);

void hashmap_test(void)
{
    HashMap *h = new_hash(10);
    add_hash(h, "3", (void *)6);
    add_hash(h, "4", (void *)8);
    add_hash(h, "5", (void *)10);
    assert(10 == get_hash(h, "5"));
    assert(6 == get_hash(h, "3"));
    assert(8 == get_hash(h, "4"));
    HashNode *c = h->begin;
    assert(10 == (int)c->value);
    c = c->next;
    assert(8 == (int)c->value);
    c = c->next;
    assert(6 == (int)c->value);
    add_hash(h, "4", 12);
    assert(12 == get_hash(h, "4"));
    for (HashNode *c = h->begin; c; c = c->next)
    {
        // printf("%d\n",c->value);
    }
    return 0;
}