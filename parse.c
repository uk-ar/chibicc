#define _GNU_SOURCE
//#include <stdio.h>
struct _IO_FILE;
typedef struct _IO_FILE FILE;

extern FILE *stderr; /* Standard error output stream.  */
typedef long unsigned int size_t;
//#include <stdbool.h>
// typedef char _Bool;
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

/*<postfix-expression> ::= <primary-expression>
                       | <postfix-expression> [ <expression> ]
                       | <postfix-expression> ( {<assignment-expression>}* )
                       | <postfix-expression> . <identifier>
                       | <postfix-expression> -> <identifier>
                       | <postfix-expression> ++
                       | <postfix-expression> --*/
/* postfix = primary ("->" postfix) ? */
Node *postfix()
{
        Token *tok = NULL;
        Node *ans = primary();
        for (;;)
        {
                if ((tok = consume("[")))
                {
                        if (ans->type->kind == TY_PTR)
                        {

                                ans = new_node_unary(ND_DEREF,
                                                     new_node_add(ans, expr(), tok, ans->type),
                                                     tok, ans->type->ptr_to);
                                expect("]"); // important
                                fprintf(tout, "array\n</%s>\n", __func__);
                                continue;
                        }
                        else if (ans->type->kind == TY_ARRAY)
                        {
                                Type *type = ans->type->ptr_to;
                                Node *rhs = expr(); // for debug
                                ans = new_node_unary(ND_DEREF,
                                                     new_node_add(new_node_unary(ND_ADDR, ans, ans->token, ans->type),
                                                                  rhs, tok,
                                                                  new_type_ptr(ans->type)),
                                                     tok, type);
                                // tok, ans->type->ptr_to);//Fixme cannot handle
                                expect("]"); // important
                                fprintf(tout, "array\n</%s>\n", __func__);
                                continue;
                        }
                        else
                        {
                                error_tok(token, "[ takes array or pointer only");
                        }
                }
                if ((tok = consume(".")))
                {
                        if (ans->type->kind != TY_STRUCT)
                                error_tok(token, "%s is not struct", ans->token->str);
                        tok = consume_ident();
                        if (!tok)
                                error_tok(token, "no ident defined in struct %s", ans->type->str);
                        Obj *var = get_hash(structs, ans->type->str);
                        if (!var)
                                error_tok(token, "no struct %s defined", ans->type->str);
                        Obj *field = find_var(tok->str, var);
                        if (!field)
                                error_tok(token, "no %s field defined in %s struct", tok->str, ans->type->str);
                        ans = new_node_unary(ND_MEMBER, ans, tok, field->type);
                        ans->member = field;
                        // ans->type = field->type;
                        // ans->offset -= field->offset;
                        continue;
                }
                if ((tok = consume("->")))
                {
                        // x->y is short for (*x).y
                        if (ans->type->kind != TY_PTR || ans->type->ptr_to->kind != TY_STRUCT)
                                error_tok(token, "%s is not pointer to struct", ans->token->str);
                        Obj *st_vars = get_hash(structs, ans->type->ptr_to->str); // vars for s1
                        if (!st_vars)
                                error_tok(token, "no %s defined", ans->type->ptr_to->str);
                        Token *right = consume_ident();
                        if (!right)
                                error_tok(token, "no ident defined in struct %s", ans->type->str);
                        Obj *field = find_var(right->str, st_vars);
                        if (!field)
                                error_tok(token, "no field defined %s", right->str);

                        ans = new_node_unary(ND_MEMBER,
                                             new_node_unary(ND_DEREF, ans, ans->token, ans->type->ptr_to),
                                             right, field->type);
                        ans->member = field;
                        continue;
                }
                if ((tok = consume("++")))
                {
                        // TODO:return non assign value
                        // Type *type = ans->type;
                        ans = new_node_binary(ND_EXPR,
                                              ans,
                                              new_node_binary(ND_ASSIGN,
                                                              ans,
                                                              new_node_add(ans, new_node_num(1, tok, ans->type), tok, ans->type),
                                                              tok, ans->type),
                                              tok,
                                              ans->type);

                        continue;
                }
                if ((tok = consume("--")))
                {
                        Type *type = ans->type;
                        ans = new_node_binary(ND_EXPR,
                                              ans,
                                              new_node_binary(ND_ASSIGN,
                                                              ans,
                                                              new_node_binary(ND_SUB, ans, new_node_num(1, tok, type), tok, type),
                                                              tok, type),
                                              tok,
                                              type);
                        continue;
                }
                // else
                // TODO: | <postfix-expression> ( {<assignment-expression>}* )?
                return ans;
        }
}
/*
direct-abstract-declarator:
        ( abstract-declarator )
        direct-abstract-declarator opt [ type-qualifier-list opt
                        assignment-expression opt ]
        direct-abstract-declarator opt [ static type-qualifier-list opt
                        assignment-expression ]
        direct-abstract-declarator opt [ type-qualifier-list static
                        assignment-expression ]
        direct-abstract-declarator opt [ * ]
        direct-abstract-declarator opt ( parameter-type-list opt )
*/
Type *direct_abstract_declarator(Type *t)
{
        //( abstract-declarator )
        if (consume("("))
        {
                if ((t = abstract_declarator(t)))
                {
                        expect(")");
                }
        }
        Token *tok = NULL;
        for (;;)
        {
                // direct-abstract-declarator opt ( parameter-type-list opt )
                if ((tok = consume("(")))
                {
                        t = parameter_type_list()->type;
                        expect(")");
                        continue;
                }
                // direct-abstract-declarator opt [ static type-qualifier-list opt assignment-expression ]
                if ((equal(token, "[") && equal(token->next, "static")))
                {
                        expect("[");
                        consume("static");
                        consume_Token(TK_TYPE_QUAL);
                        assign();
                        expect("]");
                        continue;
                }
                // direct-abstract-declarator opt [ * ]
                if ((equal(token, "[") && equal(token->next, "*")))
                {
                        expect("[");
                        expect("*");
                        expect("]");
                        continue;
                }
                // direct-abstract-declarator opt [ type-qualifier-list static assignment-expression ]
                if ((equal(token, "[") && equal_Token(token->next, TK_TYPE_QUAL) && equal(token->next->next, "static")))
                {
                        expect("[");
                        consume_Token(TK_TYPE_QUAL);
                        expect("static");
                        assign();
                        expect("]");
                        continue;
                }
                // direct-abstract-declarator opt [ type-qualifier-list opt assignment-expression opt ]
                if ((equal(token, "[")))
                {
                        expect("[");
                        consume_Token(TK_TYPE_QUAL);
                        Node *ans = assign();
                        t = new_type_array(t, ans->val);
                        expect("]");
                        continue;
                }
                return t;
        }
        return t;
}

Node *mul()
{
        Token *tok = NULL;
        Node *node = cast();
        for (;;)
        {
                if ((tok = consume("*")))
                {
                        fprintf(tout, " mul\n<%s>\n", __func__);
                        node = new_node_binary(ND_MUL, node, cast(), tok, node->type);
                        fprintf(tout, " mul\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("/")))
                {
                        fprintf(tout, " div\n<%s>\n", __func__);
                        node = new_node_binary(ND_DIV, node, cast(), token, node->type);
                        fprintf(tout, " div\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("%")))
                {
                        fprintf(tout, " div\n<%s>\n", __func__);
                        node = new_node_binary(ND_MOD, node, cast(), token, node->type);
                        fprintf(tout, " div\n</%s>\n", __func__);
                        continue;
                }
                return node;
        }
}

Node *add()
{
        Token *tok = NULL;
        Node *node = mul();
        for (;;)
        {
                if ((tok = consume("-")))
                {
                        fprintf(tout, " sub\n<%s>\n", __func__);
                        //左結合なのでmulを再帰する！
                        // addを再帰すると右結合になってしまう！
                        node = new_node_binary(ND_SUB, node, mul(), tok, node->type);
                        fprintf(tout, " sub\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("+")))
                {
                        fprintf(tout, " plus\n<%s>\n", __func__);
                        node = new_node_add(node, mul(), tok, node->type);
                        fprintf(tout, " plus\n</%s>\n", __func__);
                        continue;
                }
                return node;
        }
}
// TODO:shift()
Node *relational()
{
        Token *tok = NULL;
        Node *node = add();
        for (;;)
        {
                if ((tok = consume("<=")))
                {
                        fprintf(tout, " le\n<%s>\n", __func__);
                        node = new_node_binary(ND_LE, node, add(), tok, node->type);
                        fprintf(tout, " le\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume(">=")))
                {
                        fprintf(tout, " le\n<%s>\n", __func__);
                        Node *lhs = add();
                        node = new_node_binary(ND_LE, lhs, node, tok, lhs->type); // swap!
                        fprintf(tout, " le\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("<")))
                {
                        fprintf(tout, " lt\n<%s>\n", __func__);
                        node = new_node_binary(ND_LT, node, add(), tok, node->type);
                        fprintf(tout, " lt\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume(">")))
                {
                        fprintf(tout, " lt\n<%s>\n", __func__);
                        Node *lhs = add();
                        node = new_node_binary(ND_LT, lhs, node, tok, lhs->type); // swap!
                        fprintf(tout, " lt\n</%s>\n", __func__);
                        continue;
                }
                return node;
        }
}
Node *equality()
{
        Token *tok = NULL;
        Node *node = relational();
        for (;;)
        {
                if ((tok = consume("==")))
                {
                        fprintf(tout, " eq\n<%s>\n", __func__);
                        node = new_node_binary(ND_EQ, node, relational(), tok, node->type);
                        fprintf(tout, " eq\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("!=")))
                {
                        fprintf(tout, " ne\n<%s>\n", __func__);
                        node = new_node_binary(ND_NE, node, relational(), tok, node->type);
                        fprintf(tout, " ne\n</%s>\n", __func__);
                        continue;
                }
                // else
                return node;
        }
}
Node *logical_expr()
{
        Token *tok = NULL;
        Node *node = equality();
        for (;;)
        {
                if ((tok = consume("&&")))
                {
                        fprintf(tout, " eq\n<%s>\n", __func__);
                        node = new_node_binary(ND_AND, node, equality(), tok, node->type);
                        fprintf(tout, " eq\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("||")))
                {
                        fprintf(tout, " ne\n<%s>\n", __func__);
                        node = new_node_binary(ND_OR, node, equality(), tok, node->type);
                        fprintf(tout, " ne\n</%s>\n", __func__);
                        continue;
                }
                // else
                return node;
        }
}
Node *constant_expr()
{
        Node *node = NULL;
        node = logical_expr();
        Token *tok = NULL;
        for (;;)
        {
                if ((tok = consume("?")))
                {
                        Node *then = expr();
                        Node *cond = node;
                        node = new_node(ND_COND, tok, then->type);

                        node->cond = cond;
                        node->then = then;

                        expect(":");
                        node->els = constant_expr();
                }
                return node;
        }
}

//easy to debug
extern Node *stmt();
Node *expr()
{
        Node *node = assign();
        Token *tok = NULL;

        for (;;)
        {
                if ((tok = consume(",")))
                {
                        node = new_node_binary(ND_EXPR, node, assign(), tok, node->type);
                        continue;
                }
                return node;
        }
        return node;
}

