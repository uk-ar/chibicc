#define _GNU_SOURCE
//#include <stdio.h>
struct _IO_FILE;
typedef struct _IO_FILE FILE;

extern FILE *stderr; /* Standard error output stream.  */
typedef long unsigned int size_t;
//#include <stdbool.h>
// typedef char _Bool;
#define bool char
//#define bool _Bool
#define true 1
#define false 0
#define NULL (0)
/*int vfprintf(FILE *stream, const char *format, va_list arg);
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v, l) __builtin_va_arg(v, l)*/
void *calloc(size_t __nmemb, size_t __size);
int fprintf(FILE *__restrict __stream, const char *__restrict __fmt, ...);

//#include <stdlib.h>
//#include <string.h>
//#include <stdbool.h>
//#include <ctype.h>
//#include <stdarg.h>
//#include <assert.h>
void exit(int __status);
extern int strcmp(const char *__s1, const char *__s2);
extern size_t strlen(const char *__s);
// extern int vasprintf(char **ret, const char *format, va_list ap);
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
extern int memcmp(const void *__s1, const void *__s2, size_t __n);

#include "9cc.h"

// constant variables
char *filename;
char *user_input;
FILE *tout;

// global variables
Token *token; // current token
HashMap *structs, *types, *keyword2token, *type_alias, *enums;

// function prototypes
Obj *parameter_type_list();
Type *abstract_declarator(Type *t);
Obj *declaration();
int align_to(int offset, int size);
void function_definition(Obj *obj);

bool equal_Token(Token *tok, TokenKind kind) // 8=cf98,12
{
        if (!tok) // 6d4cb0
                return false;
        if ((tok->kind == kind) ||
            (tok->kind == TK_IDENT &&
             (get_hash(keyword2token, tok->str) == (void *)kind)
             //(get_hash(keyword2token, tok->str) == kind) //FIXME
             //(kind == get_hash(keyword2token, tok->str))
             )) // keyword2token=6d1ef0
                return true;
        return false;
}

Token *consume_Token(TokenKind kind)
{
        if (!equal_Token(token, kind))
                return NULL;
        Token *ans = token;
        token = token->next;
        return ans;
}

bool equal(Token *tok, char *op)
{
        int n = strlen(op);
        return (strncmp(op, tok->str, n + 1) == 0);
}

Token *consume(char *op)
{ // if next == op, advance & return true;
        if (!equal(token, op))
                return NULL;
        // printf("t:%s:%d\n",token->pos,token->len);
        Token *ans = token;
        token = token->next;
        return ans;
}

Token *consume_ident()
{ // if next == op, advance & return true;
        return consume_Token(TK_IDENT);
}

void expect(char *op)
{ // if next == op, advance
        if (!token || token->kind != TK_RESERVED)
                error_tok(token, "token is not '%s'", op);
        if (strncmp(op, token->pos, strlen(op)) != 0)
                error_tok(token, "token is not '%s'", op);
        // printf("t:%s:%d\n",token->pos,token->len);
        token = token->next;
}

long expect_num()
{ //
        if (equal_Token(token, TK_ENUM))
        {
                long ans = (long)get_hash(enums, token->str);
                token = token->next;
                return ans;
        }
        if (equal_Token(token, TK_NUM))
        {
                long ans = token->val;
                token = token->next;
                return ans;
        }
        error_tok(token, "token is not number");
        return 0;
}

bool at_eof()
{
        return !token || token->kind == TK_EOF;
}

extern Obj *new_obj(Token *tok, Obj *next, Type *t);

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs, Token *token, Type *type)
{
        Node *ans = new_node(kind, token, type);
        ans->lhs = lhs;
        ans->rhs = rhs;
        return ans;
}
Node *new_node_num(long val, Token *token, Type *type)
{
        // leaf node
        Node *ans = new_node(ND_NUM, token, type);
        ans->val = val;
        return ans;
}

// Fixme
Node *new_node(NodeKind kind, Token *token, Type *type)
{
        Node *ans = calloc(1, sizeof(Node));
        ans->kind = kind;
        ans->token = token;
        ans->type = type;
        return ans;
}

Node *new_node_add(Node *lhs, Node *rhs, Token *token, Type *type)
{
        if (lhs->type->kind == TY_ARRAY || lhs->type->kind == TY_PTR)
        {
                return new_node_binary(ND_ADD, lhs,
                                       new_node_binary(ND_MUL, rhs,
                                                       new_node_num(lhs->type->ptr_to->size, token, type),
                                                       lhs->token, rhs->type),
                                       token, type);
        }
        return new_node_binary(ND_ADD, lhs, rhs, token, type);
}
Node *new_node_sub(Node *lhs, Node *rhs, Token *token, Type *type)
{
        if (lhs->type->kind == TY_ARRAY || lhs->type->kind == TY_PTR)
        {
                return new_node_binary(ND_SUB, lhs,
                                       new_node_binary(ND_MUL, rhs,
                                                       new_node_num(lhs->type->ptr_to->size, token, type),
                                                       lhs->token, rhs->type),
                                       token, type);
        }
        return new_node_binary(ND_SUB, lhs, rhs, token, type);
}
Node *new_node_unary(NodeKind kind, Node *lhs, Token *token, Type *type)
{
        Node *ans = new_node(kind, token, type);
        ans->lhs = lhs;
        return ans;
}

HashMap *strings;
long get_string_offset(char *s)
{
        long var = (long)get_hash(strings, s); //
        // get_hash should return HashNode
        if (!var)
        {
                var = count();
                add_hash(strings, s, (void *)var);
        }
        return var;
}
Node *new_node_string(char *s, Token *token)
{
        // token not required
        Node *ans = new_node(ND_STR, token, NULL);
        ans->offset = get_string_offset(s);
        return ans;
}
void add_node(Node *node, Node *new_node)
{
        if (!new_node)
                error_tok(token, "node empty");
        if (!node->head)
        {
                node->head = new_node;
                node->tail = new_node;
                return;
        }
        node->tail->next = new_node;
        node->tail = node->tail->next;
        return;
}

// Obj *scope->locals = NULL;
Obj *globals = NULL;
// Obj *functions = NULL;

Obj *find_var(char *str, Obj *var0)
{
        int n = strlen(str);
        for (Obj *var = var0; var; var = var->next)
        {
                if ((n == var->len) && !memcmp(str, var->name, n))
                {
                        return var;
                }
        }
        return NULL;
}
Obj *find_lvar(Token *tok)
{
        return find_var(tok->str, scope->locals);
}
Obj *find_lvar_all(Token *tok)
{
        Obj *ans = find_lvar(tok);
        if (ans)
                return ans;
        /*for (int i = lstack_i; i >= 0; i--)
        {
                Obj *ans = find_var(tok->str, lstack[i]);
                if (ans)
                {
                        return ans;
                }
        }*/
        Scope *cur = scope;
        while (cur)
        {
                Obj *ans = find_var(tok->str, cur->locals);
                if (ans)
                {
                        return ans;
                }
                cur = cur->next;
        }
        return NULL;
}
Obj *find_gvar(Token *tok)
{
        return find_var(tok->str, globals);
}
Obj *new_obj(Token *tok, Obj *next, Type *t)
{
        Obj *var;
        var = calloc(1, sizeof(Obj));
        var->next = next;
        var->token = token;
        var->name = tok->str;
        var->len = tok->len;
        var->type = t;
        return var;
}
// int scope->offset = 0;
Obj *new_obj_local(Token *tok, Obj *next, Type *t)
{
        Obj *ans = new_obj(tok, next, t);
        ans->offset = align_to(scope->offset, t->align);
        scope->offset = ans->offset += t->align;
        return ans;
}
Obj *struct_declarator_list(Obj *lvar);
// in order to reset offset
int align_to(int offset, int size)
{
        offset = (offset + size - 1) / size * size;
        return offset;
}