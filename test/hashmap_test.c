#include "test.h"
#include "../9cc.h"

int main(int argc, char **argv)
{
    HashMap a, *h = new_hash(10);
    HashNode *n;
    //HashMap a, *h = &a;//need 0 clear
    //HashNode *nodes[10];//need 0 clear
    //h->nodes = nodes;
    //printf(&(h->nodes[1]));

    ASSERT(8, ({ distance(&(h->nodes[0]), &(h->nodes[1])); }));
    /*ASSERT(32, sizeof(HashNode));
    ASSERT(24, sizeof(HashMap));
    //  print_hash(h);
    //  printf("%p\n", h);
    //  printf("%p\n", &(h->begin));
    //  printf("%p\n", &(h->nodes));
    //  printf("%p\n", &(h->size));
    ASSERT(0, h->nodes == 0);
    ASSERT(0, h->begin);
    ASSERT(10, h->size);

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
    n = add_hash(h, "3", 6); //"3"=0x401ca5
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
        {
        HashMap *keyword2token = new_hash(100);
        add_hash(keyword2token, "TypeKind", 1);
        ASSERT(get_hash(keyword2token, "TypeKind"), 1);
    }
    /*{
        int a, b;
        Node *n=new_node(1, &a, &b);
        ASSERT(&a, n->token);
        ASSERT(&b, n->type);
    }*/
        /*for (HashNode *c = h->begin; c; c = c->next)
        {
            printf("%d\n", c->value);
        }*/
    return 0;
}