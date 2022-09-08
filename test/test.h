#define ASSERT(a,b) assert(a,b,#b,__FILE__)
#define DUMP_REGISTER()                 \
    {                                   \
        register void *esp asm("%rsp"); \
        printf("rsp:%p\n", rsp);        \
    }

extern int strcmp(const char *__s1, const char *__s2);
