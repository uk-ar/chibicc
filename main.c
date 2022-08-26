#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
#include "9cc.h"

extern FILE *tout;
extern FILE *tout2;
extern char *user_input;
extern char *filename;
extern Token *token;
extern Node *code[];
extern LVar *locals, *globals, *strings, *functions;
extern int lstack_i;
extern HashMap *structs, *types, *keyword2token, *type_alias, *enums;
// struct name->LVars,type name->Type,,defname->(char*)type name,(char*)enum id->int

char *read_file(char *path)
{
        FILE *fp = fopen(path, "r");
        if (!fp)
                fprintf(stderr, "cannot open %s:%s", path, strerror(errno));

        if (fseek(fp, 0, SEEK_END) == -1)
                fprintf(stderr, "%s:fseek:%s", path, strerror(errno));
        size_t size = ftell(fp);
        if (fseek(fp, 0, SEEK_SET) == -1)
                fprintf(stderr, "%s:fseek:%s", path, strerror(errno));
        char *buf = calloc(sizeof(char), size + 2);
        fread(buf, size, 1, fp);

        // make sure that buffer end in \n\0
        if (size == 0 || buf[size - 1] != '\n')
                buf[size++] = '\n';
        buf[size] = '\0';
        fclose(fp);
        return buf;
}

char *global_types[] = {".byte", ".long", ".quad", ".quad"};
extern Type *new_type(TypeKind ty, Type *ptr_to, size_t size, char *str);
extern HashNode *add_hash(HashMap *h, char *key, void *value);

int main(int argc, char **argv)
{
        tout2 = stdout; // debug
        // tout=stderr;
        // hashmap_test();
        if (argc != 2)
        {
                fprintf(stderr, "wrong number of argument\n.");
                return 1;
        }
        tout = fopen("tmp.xml", "w");
        locals = calloc(1, sizeof(LVar));
        // lstack[lstack_i]=locals;
        structs = new_hash(100);

        types = new_hash(100);
        add_hash(types, "int", new_type(TY_INT, NULL, 4, "int"));
        add_hash(types, "char", new_type(TY_CHAR, NULL, 1, "char"));
        add_hash(types, "long", new_type(TY_LONG, NULL, 8, "long"));
        add_hash(types, "long int", new_type(TY_LONG, NULL, 8, "long int"));
        add_hash(types, "long long int", new_type(TY_LONG, NULL, 8, "long long int"));
        add_hash(types, "void", new_type(TY_INT, NULL, 4, "void"));

        type_alias = new_hash(100);

        keyword2token = new_hash(100);
        add_hash(keyword2token, "enum", (void *)TK_TYPE_SPEC);
        add_hash(keyword2token, "void", (void *)TK_TYPE_SPEC);

        add_hash(keyword2token, "auto", (void *)TK_STORAGE);
        add_hash(keyword2token, "register", (void *)TK_STORAGE);
        add_hash(keyword2token, "static", (void *)TK_STORAGE);
        add_hash(keyword2token, "extern", (void *)TK_STORAGE);
        add_hash(keyword2token, "typedef", (void *)TK_STORAGE);

        add_hash(keyword2token, "const", (void *)TK_TYPE_QUAL);
        add_hash(keyword2token, "restrict", (void *)TK_TYPE_QUAL);
        add_hash(keyword2token, "volatile", (void *)TK_TYPE_QUAL);
        add_hash(keyword2token, "_Atomic", (void *)TK_TYPE_QUAL);

        add_hash(keyword2token, "__extension__", (void *)TK_NOT_SUPPORT);
        add_hash(keyword2token, "__attribute__", (void *)TK_NOT_SUPPORT);
        add_hash(keyword2token, "unsigned", (void *)TK_NOT_SUPPORT);
        add_hash(keyword2token, "__restrict", (void *)TK_NOT_SUPPORT);

        enums = new_hash(100);

        filename = argv[1];
        // fprintf(tout,"# %s\n",filename);
        user_input = read_file(filename);

        token = tokenize(user_input);
        program();
        assert(lstack_i == 0);
        fclose(tout);

        // header
        printf(".file \"%s\"\n", filename);
        //printf(".file 1 \"%s\"\n", filename); unable to debug tms.s
        printf(".intel_syntax noprefix\n");

        for (LVar *var = strings; var; var = var->next)
        {
                printf("  .text \n");
                //printf("  .section      .rodata \n");
                printf(".LC%d:\n", var->offset);
                printf("  .string \"%s\"\n", var->name);
        }
        // for debug
        printf(".LCdebug:\n");
        printf("  .string \"%s\"\n", "rsp:%p\\n");

        for (LVar *var = globals; var; var = var->next)
        { // gvar
                // https://github.com/rui314/chibicc/commit/a4d3223a7215712b86076fad8aaf179d8f768b14
                printf(".data\n");
                printf(".global %s\n", var->name);
                printf("%s:\n", var->name);
                char *p = var->init;
                if (!p)
                {
                        printf("  .zero %ld\n", var->type->size);
                }
                else
                {
                        if (var->type->kind == TY_ARRAY)
                        {
                                printf("  .string %s\n", p);
                        }
                        else
                        {
                                printf("  %s %s\n", global_types[var->type->kind], p);
                        }
                }
        }
        for (int i = 0; code[i]; i++)
        {
                gen(code[i]);

                // pop each result in order not to over flow
                printf("  pop rax\n");
        }

        return 0;
}
