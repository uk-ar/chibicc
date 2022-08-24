#include "test.h"
#include "../9cc.h"

HashMap *new_hash(int size);
void add_hash(HashMap *h, char *key, void *value);
void *get_hash(HashMap *h, char *key);

int main(int argc, char **argv)
{
    // printf("foo %c", (char)10);
    // printf("foo %p", (void *)10);
    // char a = (char)4;
    HashMap *h = new_hash(10);
    int a;
    int *p = &a;

    HashNode *n = add_hash(h, "3", 6);
    ASSERT(6, get_hash(h, "3"));
    ASSERT(6, get_node_value(n));
    ASSERT(32, sizeof(HashNode));
    printf("%p\n", n);
    printf("%p\n", &(n->next_bucket));
    printf("%p\n", &(n->next));
    printf("%p\n", &(n->key));
    printf("%p\n", &(n->value));
    ASSERT(6, n->value);
    ASSERT(h->begin, n);

    n = add_hash(h, "4", (void *)8);
    ASSERT(8, get_hash(h, "4"));
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
    /*for (HashNode *c = h->begin; c; c = c->next)
    {
        printf("%d\n", c->value);
    }*/
    return 0;
}