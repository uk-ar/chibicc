/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>
*/
#include "9cc.h"

#define NULL 0 //((void *)0) TODO:SUPPORT
struct _IO_FILE;
typedef struct _IO_FILE FILE;
extern FILE *stderr; /* Standard error output stream.  */
extern FILE *stdout; /* Standard error output stream.  */
extern int *__errno_location(void);
char *strerror(int __errnum);
#define errno (*__errno_location())
#define SEEK_SET 0 /* Seek from beginning of file.  */
#define SEEK_CUR 1 /* Seek from current position.  */
#define SEEK_END 2 /* Seek from end of file.  */
typedef long unsigned int size_t;
extern FILE *fopen(const char *__restrict __filename,
                   const char *__restrict __modes);
long int ftell(FILE *__stream);
extern void *calloc(size_t __nmemb, size_t __size);
extern size_t fread(void *__restrict __ptr, size_t __size, size_t __n,
                    FILE *__restrict __stream);
extern int fclose(FILE *__stream);
extern int fprintf(FILE *__restrict __stream,
                   const char *__restrict __format, ...);
extern int fseek(FILE *__stream, long int __off, int __whence);
extern int printf(const char *__restrict __format, ...);

extern FILE *tout;
extern FILE *tout2;
extern char *user_input;
extern char *filename;
extern Token *token;
// extern Node *code[];
extern HashMap *strings, *labels;
//, *functions;
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

Type *ty_int = NULL;  //&(Type){TY_INT, NULL, 4, "int"};//TODO:SUPPORT
Type *ty_char = NULL; //&(Type){TY_CHAR, NULL, 1, "char"};
Type *ty_long = NULL; //&(Type){TY_LONG, NULL, 8, "long"};
Type *ty_bool = NULL;

int main(int argc, char **argv)
{
        if (argc != 2)
        {
                fprintf(stderr, "wrong number of argument\n.");
                return 1;
        }
        tout2 = stdout; // debug
        // tout=stderr;
        // hashmap_test();
        tout = fopen("tmp.xml", "w");
        scope = new_scope(NULL, 0);

        // calloc(1, sizeof(Obj));
        strings = new_hash(1000);
        structs = new_hash(100);
        labels = new_hash(100);

        types = new_hash(100);
        ty_int = new_type(TY_INT, NULL, 4, "int", 4);
        ty_char = new_type(TY_CHAR, NULL, 1, "char", 1);
        ty_long = new_type(TY_LONG, NULL, 8, "long", 8);
        ty_bool = new_type(TY_BOOL, NULL, 1, "bool", 1);
        add_hash(types, "int", ty_int);
        add_hash(types, "char", ty_char);
        add_hash(types, "long", ty_long);
        add_hash(types, "_Bool", ty_bool);
        add_hash(types, "long int", ty_long);
        add_hash(types, "long long int", ty_long);
        add_hash(types, "void", ty_int);

        type_alias = new_hash(100);

        keyword2token = new_hash(100);
        add_hash(keyword2token, "enum", (void *)TK_TYPE_SPEC);
        add_hash(keyword2token, "void", (void *)TK_TYPE_SPEC);
        add_hash(keyword2token, "int", (void *)TK_TYPE_SPEC);
        add_hash(keyword2token, "char", (void *)TK_TYPE_SPEC);
        add_hash(keyword2token, "long", (void *)TK_TYPE_SPEC);
        add_hash(keyword2token, "_Bool", (void *)TK_TYPE_SPEC);
        add_hash(keyword2token, "struct", (void *)TK_TYPE_SPEC);
        add_hash(keyword2token, "unsigned", (void *)TK_TYPE_SPEC);
        // add_hash(keyword2token, "__builtin_va_list", (void *)TK_TYPE_SPEC);

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
        add_hash(keyword2token, "__restrict", (void *)TK_NOT_SUPPORT);

        add_hash(keyword2token, "switch", (void *)TK_IF);
        add_hash(keyword2token, "if", (void *)TK_IF);

        add_hash(keyword2token, "case", (void *)TK_RESERVED);
        add_hash(keyword2token, "default", (void *)TK_RESERVED);
        add_hash(keyword2token, "break", (void *)TK_RESERVED);
        add_hash(keyword2token, "continue", (void *)TK_RESERVED);
        add_hash(keyword2token, "_Alignof", (void *)TK_RESERVED);
        // add_hash(keyword2token, "...", (void *)TK_RESERVED);// conflict with .

        add_hash(keyword2token, "sizeof", (void *)TK_SIZEOF);
        add_hash(keyword2token, "return", (void *)TK_RETURN);
        add_hash(keyword2token, "else", (void *)TK_ELSE);
        add_hash(keyword2token, "while", (void *)TK_WHILE);
        add_hash(keyword2token, "for", (void *)TK_FOR);

        enums = new_hash(100);

        filename = argv[1];
        // fprintf(tout,"# %s\n",filename);
        user_input = read_file(filename);

        token = tokenize(user_input);
        program();
        // assert(lstack_i == 0);
        fclose(tout);

        codegen(scope->locals, filename);

        return 0;
}
