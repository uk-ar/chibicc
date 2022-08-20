#include "test.h"
//#include "../9cc.h"

//HashMap *new_hash(int size);
//void add_hash(HashMap *h, char *key, void *value);
//void *get_hash(HashMap *h, char *key);

int main(int argc,char**argv)
{
    printf("foo %c", (char)10);
    //printf("foo %p", (void *)10);
    // char a = (char)4;
    /*HashMap *h = new_hash(10);
    add_hash(h, "3", 6);
    ASSERT(6, get_hash(h, "3"));
    add_hash(h, "4", (void *)8);
    /*add_hash(h, "5", (void *)10);
    ASSERT(10 , get_hash(h, "5"));

    ASSERT(8 , get_hash(h, "4"));
    HashNode *c = h->begin;
    ASSERT(10 , (int)c->value);
    c = c->next;
    ASSERT(8 , (int)c->value);
    c = c->next;
    ASSERT(6 , (int)c->value);
    add_hash(h, "4", 12);
    ASSERT(12 , get_hash(h, "4"));
    for (HashNode *c = h->begin; c; c = c->next)
    {
        // printf("%d\n",c->value);
    }*/
    return 0;
}