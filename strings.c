#define _GNU_SOURCE
//#include <stdio.h>
struct _IO_FILE;
typedef struct _IO_FILE FILE;
// typedef __builtin_va_list va_list;
/*typedef struct
{
        unsigned int gp_offset;
        unsigned int fp_offset;
        void *overflow_arg_area;
        void *req_save_area;
} va_list[1];*/
extern FILE *stderr; /* Standard error output stream.  */
typedef long unsigned int size_t;

#include <stdarg.h>

#define bool _Bool
#define true 1
#define false 0
#define NULL ((void *)0)
int vfprintf(FILE *stream, const char *format, va_list arg);
/*#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)*/
void *calloc(size_t __nmemb, size_t __size);
int fprintf(FILE *__restrict __stream, const char *__restrict __fmt, ...);

//#include <stdlib.h>
//#include <string.h>
//#include <stdbool.h>
//#include <ctype.h>

//#include <assert.h>
void exit(int __status);
extern int strcmp(const char *__s1, const char *__s2);
extern size_t strlen(const char *__s);
extern int vasprintf(char **ret, const char *format, va_list ap);
//#define _ISbit(bit) ((bit) < 8 ? ((1 << (bit)) << 8) : ((1 << (bit)) >> 8))
extern int strncmp(const char *__s1, const char *__s2, size_t __n);
/*enum
{
        _ISupper = _ISbit(0),
        _ISlower = _ISbit(1),
        _ISalpha = _ISbit(2),
        _ISdigit = _ISbit(3),
        _ISxdigit = _ISbit(4),
        _ISspace = _ISbit(5),
        _ISprint = _ISbit(6),
        _ISgraph = _ISbit(7),
        _ISblank = _ISbit(8),
        _IScntrl = _ISbit(9),
        _ISpunct = _ISbit(10),
        _ISalnum = _ISbit(11)
};
#define isalpha(c) __isctype((c), _ISalpha)
#define isdigit(c) __isctype((c), _ISdigit)
#define isspace(c) __isctype((c), _ISspace)*/
extern char *strstr(const char *__haystack, const char *__needle);
extern long int strtol(const char *__restrict __nptr,
                       char **__restrict __endptr, int __base);
extern int memcmp(const void *__s1, const void *__s2, size_t __n);

#include "yucc.h"

Token *token; // current token

FILE *tout;

char *format(char *fmt, ...)
{
        char *ptr;
        // not working on x86_64 on arm
        // FILE* out = open_memstream(&ptr, &size);

        va_list ap;
        va_start(ap, fmt);
        // vfprintf(out, fmt, ap);
        vasprintf(&ptr, fmt, ap);
        va_end(ap);
        // fclose(out);
        return ptr;
}
static void verror_at(char *s, char *fmt, va_list ap)
{
        char *loc = s;
        char *start = loc;
        while (user_input < start && start[-1] != '\n') //*(start-1)
                start--;

        char *end = loc;
        while (*end && *end != '\n')
                end++;

        int line_num = 1;
        for (char *p = user_input; p < loc; p++)
        {
                if (*p == '\n')
                        line_num++;
        }

        int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
        fprintf(stderr, "%.*s\n", (int)(end - start), start);

        int pos = loc - start + indent;
        fprintf(stderr, "%*s^ ", pos, " ");
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        exit(1);
}
void error_at(char *s, char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        verror_at(s, fmt, ap);
}
void error_tok(Token *tok, char *fmt, ...)
{
        va_list ap;
        va_start(ap, fmt);
        verror_at(tok->pos, fmt, ap);
}
