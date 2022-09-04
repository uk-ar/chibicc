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

Type *new_type(TypeKind ty, Type *ptr_to, size_t size, char *str)
{ //
        Type *type = calloc(1, sizeof(Type));
        type->kind = ty;
        type->ptr_to = ptr_to;
        type->size = size;
        type->str = str;
        return type;
}

bool equal_Token(Token *tok, TokenKind kind)
{
        if (!tok)
                return false;
        if ((tok->kind == kind) ||
            (tok->kind == TK_IDENT && (get_hash(keyword2token, tok->str) == (void *)kind)))
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
        return (strcmp(op, tok->str) == 0);
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

HashMap *strings;
extern Obj *new_obj(Token *tok, Obj *next, Type *t);

Node *new_node(NodeKind kind, Token *token, Type *type)
{
        Node *ans = calloc(1, sizeof(Node));
        ans->kind = kind;
        ans->token = token;
        ans->type = type;
        return ans;
}

Node *new_node_binary(NodeKind kind, Node *lhs, Node *rhs, Token *token, Type *type)
{
        Node *ans = new_node(kind, token, type);
        ans->lhs = lhs;
        ans->rhs = rhs;
        return ans;
}
Node *new_node_unary(NodeKind kind, Node *lhs, Token *token, Type *type)
{
        Node *ans = new_node(kind, token, type);
        ans->lhs = lhs;
        return ans;
}
Node *new_node_num(long val, Token *token, Type *type)
{
        // leaf node
        Node *ans = new_node(ND_NUM, token, type);
        ans->val = val;
        return ans;
}
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
Obj *locals = NULL;
Obj *globals = NULL;
// Obj *functions = NULL;
HashMap *cases = NULL;
Obj *lstack[100];     // local
HashMap *cstack[100]; // case
int lstack_i = 0;

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
        return find_var(tok->str, locals);
}
Obj *find_lvar_all(Token *tok)
{
        Obj *ans = find_lvar(tok);
        if (ans)
                return ans;
        for (int i = lstack_i; i >= 0; i--)
        {
                Obj *ans = find_var(tok->str, lstack[i]);
                if (ans)
                {
                        return ans;
                }
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
int loffset = 0;
Obj *new_local(Token *tok, Obj *next, Type *t)
{
        Obj *ans = new_obj(tok, next, t);
        ans->offset = align_to(loffset, t->size);
        ans->offset += t->size;
        return ans;
}
Obj *struct_declarator_list(Obj *lvar);
// in order to reset offset
int align_to(int offset, int size)
{
        offset = (offset + size - 1) / size * size;
        return offset;
}

Obj *struct_declaration(Type *type)
{
        Obj *st_vars = calloc(1, sizeof(Obj));
        // Type *type = new_type(TY_STRUCT, NULL, 0,NULL);
        // type->str = str1;
        // add_hash(types, format("%s %s", type_str, str1), type);
        int max_offset = 0;
        while (!consume("}"))
        {
                st_vars = struct_declarator_list(st_vars);
                max_offset = MAX(max_offset, st_vars->type->size);
                consume(";");
        }
        // add_hash(structs, str1, st_vars);
        type->size = align_to(loffset, max_offset);
        return st_vars;
}

Obj *enumerator_list()
{
        Obj *st_vars = calloc(1, sizeof(Obj));
        st_vars->offset = -1;
        while (!consume("}"))
        {
                Token *tok = consume_ident();
                st_vars = new_obj(tok, st_vars, ty_int);
                st_vars->offset = st_vars->next->offset + 1;
                add_hash(keyword2token, tok->str, (void *)TK_ENUM);
                add_hash(enums, tok->str, (void *)st_vars->offset);
                consume(",");
        }
        return st_vars;
}
Type *type_name();
Type *declaration_specifier() // bool declaration)
{
        Token *storage = consume_Token(TK_STORAGE);
        // Token *type_qual =
        consume_Token(TK_TYPE_QUAL);
        Token *type_spec = consume_Token(TK_TYPE_SPEC);
        // Type *type_spec = type_name();
        Token *identifier = NULL;
        // char *def_name = NULL;
        char *src_name = NULL;
        if (!type_spec)
                return NULL;
        // char *type_str = type_spec->str;

        char *type_str = type_spec->str;
        if (strncmp(type_str, "struct", 6) == 0)
        // if (type_spec->kind == TY_STRUCT)
        {
                Type *type = NULL;
                Obj *st_vars = NULL;
                if (consume("{"))
                { // anonymous
                        type = new_type(TY_STRUCT, NULL, 0, NULL);
                        st_vars = struct_declaration(type);
                }
                else if ((identifier = consume_ident())) // struct name
                {
                        src_name = format("%s %s", type_str, identifier->str);
                        if (equal(token, ";"))
                        {
                                // incomplete type
                                type = get_hash(types, src_name);
                                if (!type)
                                {
                                        add_hash(types, src_name, new_type(TY_STRUCT, NULL, 0, NULL));
                                }
                                return get_hash(types, src_name);
                        }
                        if (consume("{"))
                        {
                                type = new_type(TY_STRUCT, NULL, 0, NULL);
                                type->str = identifier->str;
                                if (!add_hash(types, src_name, type)) // for recursive field type
                                        error_tok(token, "redefine %s", src_name);
                                st_vars = struct_declaration(type);
                                add_hash(structs, src_name, st_vars);
                        }
                        else
                        { // reference
                                type = get_hash(types, src_name);
                                st_vars = get_hash(structs, src_name);
                        }
                }
                else
                {
                        error_tok(token, "need identifier for struct\n");
                }
                if (storage && (strncmp(storage->str, "typedef", 6) == 0))
                {
                        Token *declarator = consume_ident(); // defname
                        if (!declarator)
                                error_tok(token, "typedef need declarator for struct\n");
                        if (src_name)
                        {
                                add_hash(type_alias, declarator->str, src_name);
                        }
                        else if (type)
                        {
                                add_hash(types, declarator->str, type); // for recursive field type
                                add_hash(structs, declarator->str, st_vars);
                        }
                        else
                        {
                                error_tok(token, "typedef need identifier for struct\n");
                        }
                        add_hash(keyword2token, declarator->str, (void *)TK_TYPE_SPEC);
                }
                return type;
        }
        // if (type_spec->kind == TY_STRUCT)
        if (strcmp(type_str, "enum") == 0)
        {
                Type *type = NULL;
                Obj *st_vars = NULL;
                if (consume("{"))
                { // anonymous
                        // type = new_type(TY_STRUCT, NULL, 0,NULL);
                        // st_vars = struct_declaration(type);
                        st_vars = enumerator_list();
                }
                else if ((identifier = consume_ident())) // struct name
                {
                        src_name = format("%s %s", type_str, identifier->str);
                        if (consume("{"))
                        {
                                /*type = new_type(TY_STRUCT, NULL, 0,NULL);
                                type->str = identifier->str;
                                add_hash(types, src_name, type); // for recursive field type
                                st_vars = struct_declaration(type);
                                add_hash(structs, src_name, st_vars);*/
                                // type = new_type(TY_STRUCT, NULL, 0,NULL);
                                st_vars = enumerator_list();
                                add_hash(types, src_name, ty_int);
                                add_hash(structs, src_name, st_vars);
                        }
                        else
                        { // reference
                                type = get_hash(types, src_name);
                                st_vars = get_hash(structs, src_name);
                        }
                }
                else
                {
                        error_tok(token, "need identifier for struct\n");
                }
                if (storage && (strncmp(storage->str, "typedef", 6) == 0))
                {
                        Token *declarator = consume_ident(); // defname
                        if (!declarator)
                                error_tok(token, "typedef need declarator for struct\n");
                        if (src_name)
                        {
                                add_hash(type_alias, declarator->str, src_name);
                        }
                        add_hash(types, declarator->str, ty_int);
                        add_hash(keyword2token, declarator->str, (void *)TK_TYPE_SPEC);
                }
                return type;
        }
        else
        {
                src_name = type_str;
                if (storage && (strncmp(storage->str, "typedef", 6) == 0))
                {
                        while (!equal(token->next, ";"))
                        {
                                src_name = format("%s %s", src_name, token->str);
                                if (!consume_ident())
                                        consume_Token(TK_TYPE_SPEC);
                        }
                        Token *declarator = consume_ident();
                        if (!declarator) // || !type || !st_vars)
                                error_tok(token, "need declarator for\n");
                        // src_name = format("%s %s", type_str, identifier->str);
                        // add_hash(types, declarator->str, type);
                        add_hash(keyword2token, declarator->str, (void *)TK_TYPE_SPEC);
                        add_hash(type_alias, declarator->str, src_name);
                }
                Token *tok = NULL;
                while ((tok = consume_Token(TK_TYPE_SPEC)))
                {
                        src_name = format("%s %s", src_name, tok->str);
                }
                while (get_hash(type_alias, src_name))
                {
                        src_name = get_hash(type_alias, src_name);
                }
                return get_hash(types, src_name);
        }
}
Obj *enter_scope()
{
        lstack[lstack_i++] = locals;
        locals = calloc(1, sizeof(Obj));
        return locals;
}
void leave_scope() // return loffset?
{
        lstack[lstack_i] = NULL;
        locals = lstack[--lstack_i];
}
// https://cs.wmich.edu/~gupta/teaching/cs4850/sumII06/The%20syntax%20of%20C%20in%20Backus-Naur%20form.htm
/* program    = stmt* */
/* stmt       = expr ";"
              | "{" stmt* "}"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | "int" ident ";"
              | "int" ident "[" expr "]" ";"
              | "return" expr ";" */
/* exprs      = expr ("," expr)* */
/* expr       = assign | "{" stmt "}" */
/* assign     = equality ("=" assign)? */
/* equality   = relational ("==" relational | "!=" relational)* */
/* relational = add ("<" add | "<=" add | ">" add | ">=" add)* */
/* add        = mul ("+" mul | "-" mul)* */
/* mul     = cast ("*" cast | "/" cast)* */
/* cast    = (type-name) cast | unary */
/* unary   = "-"? postfix | "+"? postfix | "*" postfix | "&" postfix  */
/* postfix = primary ("->" postfix) ? */
/* primary = ident.ident | ident->ident | num | ident | ident "(" exprs? ")" | primary "[" expr "]" | "(" expr ")" | TK_STR*/
Node *expr();
Node *stmt();
Node *assign();
/* primary = num | ident ("(" exprs? ")")? | "(" expr ")" */
/* exprs      = expr ("," expr)* */
Node *primary()
{
        Token *tok = NULL;
        if (consume("("))
        {
                // gnu extension
                if ((tok = consume("{")))
                {
                        fprintf(tout, " <%s>{\n", __func__);
                        Node *node = new_node(ND_EBLOCK, tok, NULL);

                        enter_scope();

                        while (!consume("}"))
                        {
                                add_node(node, stmt());
                        }

                        fprintf(tout, " }</%s>\n", __func__);

                        leave_scope();

                        expect(")"); // important
                        return node;
                }
                fprintf(tout, "<%s>(\n", __func__);
                fprintf(tout, "(");
                Node *ans = expr();
                expect(")"); // important
                fprintf(tout, ")</%s>\n", __func__);
                return ans;
        }
        else if ((tok = consume_Token(TK_STR)))
        {
                fprintf(tout, "<%s>\"\n", __func__);
                // Node *ans = new_node(ND_STR, tok, NULL);
                Node *ans = new_node_string(tok->str, tok);
                fprintf(tout, "\"\n</%s>\n", __func__);
                return ans;
        }                                        // tk_num
        else if ((tok = consume_Token(TK_ENUM))) // constant
        {
                fprintf(tout, "<%s>\"\n", __func__);
                Node *ans = new_node_num((long)get_hash(enums, tok->str), tok, ty_int);
                fprintf(tout, "\"\n</%s>\n", __func__);
                return ans;
        } // tk_num
        else if ((tok = consume_ident()))
        {
                if (consume("("))
                { // call
                        fprintf(tout, "<%s>funcall\n", __func__);

                        Type *t = NULL;
                        Obj *var = find_gvar(tok);
                        if (var)
                        {
                                t = var->type;
                        }
                        else
                        {
                                t = ty_int;
                        }
                        Node *ans = new_node(ND_FUNCALL, tok, t);
                        if (consume(")"))
                        {
                                fprintf(tout, " funcall</%s>\n", __func__);
                                return ans;
                        }
                        for (int i = 0; i < 6 && !consume(")"); i++)
                        {
                                add_node(ans, assign());
                                consume(",");
                        }
                        fprintf(tout, "funcall</%s>\n", __func__);
                        return ans;
                }
                else
                { // var ref
                        fprintf(tout, "<%s>var\n", __func__);
                        Obj *var = NULL;
                        Node *ans = NULL;
                        if ((var = find_lvar_all(tok)))
                        {
                                ans = new_node(ND_LVAR, tok, var->type);
                        }
                        else if ((var = find_gvar(tok)))
                        {
                                ans = new_node(ND_GVAR, tok, var->type);
                        }
                        else
                        {
                                error_tok(tok, "token '%s' is not defined", tok->str);
                        }
                        ans->offset = var->offset;

                        fprintf(tout, "var\n</%s>\n", __func__);
                        // ans->offset=(tok->pos[0]-'a'+1)*8;
                        return ans;
                }
        }
        Type *t = declaration_specifier(); // constant
        if (t)
        {
                Node *ans = new_node_num(0, token, t);
                return ans;
        }
        fprintf(tout, "<%s>num\n", __func__);
        // TODO:add enum
        Node *ans = new_node_num(expect_num(), token, ty_int);
        fprintf(tout, "num\n</%s>\n", __func__);
        return ans;
}
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
                                Type *type = ans->type->ptr_to;
                                ans = new_node_unary(ND_DEREF,
                                                     new_node_binary(ND_ADD, ans, expr(), tok, ans->type),
                                                     tok, type);
                                expect("]"); // important
                                fprintf(tout, "array\n</%s>\n", __func__);
                                continue;
                        }
                        else if (ans->type->kind == TY_ARRAY)
                        {
                                Type *type = ans->type->ptr_to;
                                ans = new_node_unary(ND_DEREF,
                                                     new_node_binary(ND_ADD,
                                                                     new_node_unary(ND_ADDR, ans, ans->token, ans->type),
                                                                     expr(), tok, new_type(TY_PTR, ans->type, 8, "from array")),
                                                     tok, type);
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
                        Obj *var = get_hash(structs, format("struct %s", ans->type->str));
                        if (!var)
                                error_tok(token, "no struct %s defined", ans->type->str);
                        Obj *field = find_var(tok->str, var);
                        if (!field)
                                error_tok(token, "no %s field defined in %s struct", tok->str, ans->type->str);
                        ans->type = field->type;
                        ans->offset -= field->offset;
                        continue;
                }
                if ((tok = consume("->")))
                {
                        // x->y is short for (*x).y
                        if (ans->type->kind != TY_PTR || ans->type->ptr_to->kind != TY_STRUCT)
                                error_tok(token, "%s is not pointer to struct", ans->token->str);
                        Obj *st_vars = get_hash(structs, format("struct %s", ans->type->ptr_to->str)); // vars for s1
                        if (!st_vars)
                                error_tok(token, "no struct %s defined", ans->type->str);
                        Token *right = consume_ident();
                        if (!right)
                                error_tok(token, "no ident defined in struct %s", ans->type->str);
                        Obj *field = find_var(right->str, st_vars);
                        if (!field)
                                error_tok(token, "no field defined %s", right->str);
                        Node *lhs = new_node_num(field->offset, tok, new_type(TY_CHAR, NULL, 1, "char"));
                        ans = new_node_unary(ND_DEREF,
                                             new_node_binary(ND_ADD,
                                                             lhs, // ans
                                                             ans, // lhs
                                                             tok, lhs->type),
                                             tok, field->type);
                        continue;
                }
                if ((tok = consume("++")))
                {
                        // TODO:return non assign value
                        Type *type = ans->type;
                        ans = new_node_binary(ND_EXPR,
                                              ans,
                                              new_node_binary(ND_ASSIGN,
                                                              ans,
                                                              new_node_binary(ND_ADD, ans, new_node_num(1, tok, type), tok, type),
                                                              tok, type),
                                              tok,
                                              type);

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

Type *direct_abstract_declarator(Type *t)
{
        // not used?
        if (consume("("))
        {
                if ((t = abstract_declarator(t)))
                {
                        expect(")");
                }
                else
                {
                        t = parameter_type_list()->type;
                }
                return t;
        }
        // TODO:constant-expression(conditional-expression : ?)
        // TODO:parameter-type-list
        return t;
}
Type *abstract_declarator(Type *t)
{
        while (consume("*"))
                t = new_type(TY_PTR, t, 8, "ptr");
        t = direct_abstract_declarator(t);
        return t;
}
Type *type_name() // TODO:need non consume version?
{
        // specifier-qualifier
        consume_Token(TK_TYPE_QUAL);
        Token *t = consume_Token(TK_TYPE_SPEC);
        if (!t)
                return NULL;
        char *str = t->str;
        while (t)
        {
                t = consume_Token(TK_TYPE_SPEC);
                if (t)
                        str = format("%s %s", str, t->str);
        }
        while (get_hash(type_alias, str))
        {
                str = get_hash(type_alias, str);
        }
        if (strcmp(str, "struct") == 0)
        {
                Token *id = consume_ident();
                str = format("%s %s", str, id->str);
        }
        return abstract_declarator(get_hash(types, str));
}
/* unary   = "-"? primary | "+"? primary
           | "*" unary | "&" unary  | "sizeof" unary */
Node *unary()
{
        Token *tok = NULL;
        Node *ans = NULL;
        if ((tok = consume_Token(TK_SIZEOF)))
        {
                fprintf(tout, " <%s>\"\n", __func__);
                Type *t = NULL;
                if (equal(token, "(") && equal_Token(token->next, TK_TYPE_SPEC))
                {
                        consume("(");
                        t = type_name();
                        expect(")");
                        return new_node_num(t->size, token, t);
                }
                if (equal_Token(token->next, TK_TYPE_SPEC))
                {
                        t = type_name();
                        fprintf(tout, " sizeof %d\n</%s>\n", t->kind, __func__);
                        return new_node_num(t->size, token, t);
                }
                t = unary()->type;
                fprintf(tout, " sizeof %d\n</%s>\n", t->kind, __func__);
                return new_node_num(t->size, token, t);
        }
        if ((tok = consume("++")))
        {
                ans = unary();
                Type *type = ans->type;
                return new_node_binary(ND_ASSIGN,
                                       ans,
                                       new_node_binary(ND_ADD, new_node_num(1, tok, type), ans, tok, type),
                                       tok, type);
        }
        if ((tok = consume("--")))
        {
                ans = unary();
                Type *type = ans->type;
                return new_node_binary(ND_ASSIGN,
                                       ans,
                                       new_node_binary(ND_SUB, ans, new_node_num(1, tok, type), tok, type),
                                       tok, type);
        }
        if ((tok = consume("+")))
        {
                fprintf(tout, " <%s>+\"\n", __func__);
                ans = unary();
                fprintf(tout, " +\n</%s>\n", __func__);
                return ans;
        }
        if ((tok = consume("-")))
        {
                // important
                fprintf(tout, " <%s>-\"\n", __func__);
                Type *type = ty_int;
                ans = new_node_binary(ND_SUB,
                                      new_node_num(0, tok, type),
                                      unary(), tok, type); // 0-primary()
                return ans;
                fprintf(tout, " -\n</%s>\n", __func__);
        }
        if ((tok = consume("*"))) // TODO:move?
        {
                fprintf(tout, " deref\n<%s>\n", __func__);
                Node *lhs = unary();
                Node *node = new_node_unary(ND_DEREF, lhs, tok, lhs->type->ptr_to);
                fprintf(tout, " deref\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("&")))
        {
                fprintf(tout, " ref\n<%s>\n", __func__);
                Node *lhs = unary();
                return new_node_unary(ND_ADDR, lhs, tok, lhs->type);
        }
        if ((tok = consume("!"))) // TODO:~
        {
                fprintf(tout, " <%s>\n", __func__);
                Node *lhs = new_node_num(0, tok, ty_int);
                ans = new_node_binary(ND_EQ, unary(), lhs, tok, lhs->type);
                return ans;
        }
        return postfix();
}

Node *cast()
{
        if (equal(token, "(") &&
            token->next &&
            equal_Token(token->next, TK_TYPE_SPEC)) // TODO: fix to is_typename
        {
                expect("(");
                Type *type = type_name();
                expect(")");
                Node *node = cast();
                node->type = type;
                return node;
        }
        return unary();
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
                        node = new_node_binary(ND_ADD, node, mul(), tok, node->type);
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
Node *assign()
{
        Token *tok = NULL;
        Node *node = constant_expr();
        if ((tok = consume("+="))) //右結合
        {
                fprintf(tout, " ass\n<%s>\n", __func__);
                node = new_node_binary(ND_ASSIGN,
                                       node,
                                       new_node_binary(ND_ADD, node, equality(), tok, node->type),
                                       tok, node->type);
                fprintf(tout, " ass\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("-=")))
        {
                fprintf(tout, " ass\n<%s>\n", __func__);
                node = new_node_binary(ND_ASSIGN,
                                       node,
                                       new_node_binary(ND_SUB, node, assign(), tok, node->type),
                                       tok, node->type);
                fprintf(tout, " ass\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("/=")))
        {
                fprintf(tout, " ass\n<%s>\n", __func__);
                node = new_node_binary(ND_ASSIGN,
                                       node,
                                       new_node_binary(ND_DIV, node, assign(), tok, node->type),
                                       tok, node->type);
                fprintf(tout, " ass\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("*=")))
        {
                fprintf(tout, " ass\n<%s>\n", __func__);
                node = new_node_binary(ND_ASSIGN,
                                       node,
                                       new_node_binary(ND_MUL, node, assign(), tok, node->type),
                                       tok, node->type);
                fprintf(tout, " ass\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("%=")))
        {
                fprintf(tout, " ass\n<%s>\n", __func__);
                node = new_node_binary(ND_ASSIGN,
                                       node,
                                       new_node_binary(ND_MOD, node, assign(), tok, node->type),
                                       tok, node->type);
                fprintf(tout, " ass\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("=")))
        {
                fprintf(tout, " ass\n<%s>\n", __func__);
                node = new_node_binary(ND_ASSIGN, node, assign(), tok, node->type);
                fprintf(tout, " ass\n</%s>\n", __func__);
                return node;
        }
        return node;
}
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

Node *compound_statement(Token *tok, Obj *scope)
{
        fprintf(tout, " {\n<%s>\n", __func__);

        if (!scope)
                enter_scope();

        Node *node = new_node(ND_BLOCK, tok, NULL);

        while (!consume("}"))
        {
                add_node(node, stmt());
        }

        fprintf(tout, " }\n</%s>\n", __func__);

        if (!scope)
                leave_scope();

        return node;
}

Node *default_node = NULL;
Node *stmt()
{
        /* stmt       = expr ";"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" expr? ";" expr? ";" expr? ")" stmt
           | "int" ident ";"
           | "return" expr ";" */
        Node *node = NULL;
        Token *tok = NULL;
        Type *base_t = declaration_specifier();
        while (base_t)
        {
                Type *t = base_t;
                fprintf(tout, " var decl\n<%s>\n", __func__);
                while (consume("*"))
                        t = new_type(TY_PTR, t, 8, "ptr");
                Token *tok = consume_ident(); // ident
                if (!tok)
                {
                        error_tok(token, "need identifier");
                }
                Obj *var = find_lvar(tok); //
                if (var)
                {
                        error_tok(tok, "token '%s' is already defined", tok->str);
                }
                else if (consume("["))
                {
                        int n = expect_num();
                        locals = new_local(tok, locals, new_type(TY_ARRAY, t, n * t->size, "array"));
                        expect("]");
                }
                else
                {
                        locals = new_local(tok, locals, t);
                }

                loffset = locals->offset;
                fprintf(tout, " var decl\n</%s>\n", __func__);
                if (consume("="))
                {
                        if (!node)
                        {
                                node = new_node(ND_BLOCK, tok, NULL);
                        }
                        Node *lnode = new_node(ND_LVAR, tok, t);
                        lnode->offset = locals->offset;
                        add_node(node, new_node_binary(ND_ASSIGN, lnode, assign(), tok, lnode->type));
                }
                if (consume(","))
                {
                        continue;
                }
                expect(";");
                if (node)
                        return node;
                return stmt();
                // skip token
        }
        if ((tok = consume_Token(TK_RETURN))) // jump-statement
        {
                fprintf(tout, "ret \n<%s>\n", __func__);
                if (consume(";"))
                        return new_node_unary(ND_RETURN, NULL, tok, NULL);
                node = new_node_unary(ND_RETURN, expr(), tok, NULL);
                expect(";");
                fprintf(tout, "ret \n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("if"))) // selection-statement
        {
                fprintf(tout, " if\n<%s>\n", __func__);
                expect("(");
                node = new_node(ND_IF, tok, NULL);
                node->cond = expr();
                expect(")");
                node->then = stmt();
                if (consume_Token(TK_ELSE))
                {
                        node->els = stmt();
                }
                fprintf(tout, " if\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("switch"))) // selection-statement
        {
                fprintf(tout, " switch\n<%s>\n", __func__);
                expect("(");
                node = new_node(ND_SWITCH, tok, NULL);
                node->cond = expr();
                expect(")");

                cstack[lstack_i++] = cases;
                cases = new_hash(100);
                default_node = NULL;
                node->then = stmt();
                node->cases = cases;
                node->els = default_node;
                cases = cstack[--lstack_i];

                fprintf(tout, " switch\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("case"))) // labeled-statement
        {
                fprintf(tout, " case\n<%s>\n", __func__);
                long n = expect_num();
                node = new_node_unary(ND_CASE,
                                      new_node_num(n, tok, ty_int),
                                      tok, NULL);
                node->val = count();
                add_hash(cases, format("%d", node->val), (void *)n);
                expect(":");
                fprintf(tout, " case\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("default"))) // labeled-statement
        {
                fprintf(tout, " default\n<%s>\n", __func__);
                node = new_node(ND_CASE,
                                tok, NULL);
                node->val = count();
                default_node = node;
                // add_hash(cases, format("%d", node->val), n);
                expect(":");
                fprintf(tout, " default\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("break"))) // jump-statement
        {
                fprintf(tout, " break\n<%s>\n", __func__);
                expect(";");
                node = new_node(ND_BREAK, tok, NULL);
                fprintf(tout, " break\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("continue"))) // jump-statement
        {
                fprintf(tout, " break\n<%s>\n", __func__);
                expect(";");
                node = new_node(ND_CONTINUE, tok, NULL);
                fprintf(tout, " break\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume_Token(TK_WHILE))) // iteration-statement
        {
                fprintf(tout, " while\n<%s>\n", __func__);
                expect("(");
                node = new_node(ND_WHILE, tok, NULL);
                node->cond = expr();
                expect(")");
                node->then = stmt();
                fprintf(tout, " while\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume_Token(TK_FOR))) // iteration-statement
        {
                /* Node *init;//for init */
                /* Node *cond;//if,while,for cond */
                /* Node *next;//for next */
                /* Node *then;//if,while,for then */
                fprintf(tout, " <for>\n");
                expect("(");
                node = new_node(ND_FOR, tok, NULL);

                enter_scope();

                fprintf(tout, " <init>\n");
                if (!consume(";"))
                {
                        node->init = stmt();
                        // expect(";");
                }
                fprintf(tout, " </init>\n");
                fprintf(tout, " <cond>\n");
                if (!consume(";"))
                {
                        node->cond = expr();
                        expect(";");
                }
                fprintf(tout, " </cond>\n");
                fprintf(tout, " <next>\n");
                if (!consume(")"))
                {
                        node->inc = expr();
                        expect(")");
                }
                fprintf(tout, " </next>\n");
                node->then = stmt(); // composed statement
                fprintf(tout, " </for>\n");
                leave_scope();
                return node;
        }
        if ((tok = consume("{"))) // compound-statement
                return compound_statement(tok, NULL);
        fprintf(tout, " <%s>\n", __func__);
        node = expr(); // expression-statement
        expect(";");
        fprintf(tout, " </%s>\n", __func__);
        return node;
}
Node *parameter_declaration()
{
        fprintf(tout, " \n<%s>\n", __func__);

        Type *base_t = declaration_specifier();
        if (!base_t)
                return NULL;
        // error_at(token, "declaration should start with \"type\"");

        /*Type *t = base_t;
        //while (consume("*"))
                t = new_type(TY_PTR, t, 8,"ptr");*/
        Type *t = abstract_declarator(base_t);
        Token *tok = consume_ident();
        Node *ans = new_node(ND_LVAR, tok, t);
        if (!tok) // type only
                return ans;
        Obj *var = find_lvar(tok);
        if (var)
        {
                error_tok(tok, "dumpicate param name '%s'", tok->str);
        }
        locals = new_local(tok, locals, t);

        // init_declarator側でクリアされる
        loffset = locals->offset;
        ans->offset = locals->offset; // TODO: shadow
        fprintf(tout, " \n</%s>\n", __func__);
        return ans;
}
Obj *struct_declarator_list(Obj *lvar)
{
        Type *base_t = declaration_specifier();
        if (!base_t)
                error_tok(token, "declaration should start with \"type\"");

        while (base_t)
        {
                Type *t = base_t;
                t = abstract_declarator(t);
                Token *tok = consume_ident();
                if (!tok)
                {
                        break;
                }
                fprintf(tout, " \n<%s>\n", __func__);
                Obj *var = find_lvar(tok); //

                if (var)
                {
                        error_tok(tok, "token '%s' is already defined", tok->str);
                }
                else if (consume("["))
                {
                        int n = expect_num();
                        lvar = new_obj(tok, lvar, new_type(TY_ARRAY, t, n * t->size, "array"));
                        expect("]");
                }
                else
                {
                        lvar = new_obj(tok, lvar, t);
                }
                loffset = align_to(loffset, lvar->type->size);
                lvar->offset = loffset;
                // struct_declaration 側でクリアされる
                loffset += lvar->type->size;

                if (consume(","))
                {
                        continue;
                }
                fprintf(tout, " \n</%s>\n", __func__);
        }

        return lvar;
}
Obj *parameter_type_list()
{
        /*Type *t = declaration_specifier();

        if (consume(","))
                return parameter_type_list();
        return t;*/

        if (equal(token, "void") && equal(token->next, ")"))
        {
                consume("void");
                consume(")");
                return NULL;
        }

        // Obj *ans = NULL;
        for (int i = 0; i < 6 && !consume(")"); i++)
        {
                if (consume("..."))
                {
                        if (consume(")"))
                                break;
                        else
                                error_tok(token, "va arg error\n");
                }
                parameter_declaration();
                consume(",");
        }
        return locals;
}
/*
<direct-declarator> ::= <identifier>
                      | ( <declarator> )
                      | <direct-declarator> [ {<constant-expression>}? ]
                      | <direct-declarator> ( <parameter-type-list> )
                      | <direct-declarator> ( {<identifier>}* )
<declarator> ::= {<pointer>}? <direct-declarator>
*/
Obj *declarator(Type *base_t);
Obj *direct_declarator()
{
        Token *tok;
        if ((tok = consume("(")))
        {
                // Obj *ans = declarator();
                expect(")");
        }
}
Obj *declarator(Type *base_t)
{
        // declarator
        Type *t = base_t;
        while (consume("*"))
                t = new_type(TY_PTR, t, 8, "ptr");
        Token *tok = consume_ident();
        if (!tok)
                return NULL;
        Obj *obj = new_obj(tok, NULL, t);
        if (consume("[")) // declarator?
        {
                if (consume("]"))
                {
                        obj->type = new_type(TY_ARRAY, obj->type, 0, "array");
                }
                else
                {
                        int n = expect_num();
                        obj->type = new_type(TY_ARRAY, obj->type, n * obj->type->size, "array");
                        expect("]");
                }
        }
        // todo: merge funcall?
        else
        {
                // already created
        }
        return obj;
}
/*
<init-declarator> ::= <declarator>
                    | <declarator> = <initializer>
<initializer> ::= <assignment-expression>
                | { <initializer-list> }
                | { <initializer-list> , }
<initializer-list> ::= <initializer>
                     | <initializer-list> , <initializer>
*/
int initializer(Type *type, Obj *obj)
{
        consume("&");
        char *p = token->pos;
        consume_ident();
        consume("+");
        Token *tok;
        if (!obj->init)
        {
                obj->init = new_list();
        }
        if ((tok = consume_Token(TK_STR)))
        {
                if (type->kind == TY_ARRAY)
                {
                        add_list(obj->init, format("%s", tok->str));
                        return tok->len;
                }
                else
                {
                        add_list(obj->init, format(".LC%d", get_string_offset(tok->str)));
                        return tok->len; // todo:count without escape charactor
                }
        }
        else if (consume("("))
        {
                add_list(obj->init, format("%d", expect_num()));
                expect(")");
                return 1;
        }
        else
        {
                consume_Token(TK_NUM);
                int n = token->pos - p;
                add_list(obj->init, format("%.*s", n, p));
                return 1;
        }
}
Obj *initializer_list(Obj *obj)
{
        Token *tok = NULL;
        int cnt = 0;
        if ((tok = consume("{")))
        {
                while (!consume("}"))
                {
                        initializer(obj->type->ptr_to, obj);
                        consume(",");
                        cnt++;
                }
        }
        else
        {
                cnt = initializer(obj->type, obj);
        }
        if (obj->type->kind == TY_ARRAY)
        {
                obj->type->size = MAX(obj->type->size, cnt * obj->type->ptr_to->size); // todo fix for escape charactors
        }
}
Obj *init_declarator(Obj *declarator)
{
        fprintf(tout, " \n<%s>\n", __func__);
        Obj *var = find_gvar(declarator->token);
        if (var)
        {
                declarator = var;
        }
        else
        {
                declarator->next = globals;
                globals = declarator;
        }
        if (consume("="))
        {
                initializer_list(declarator);
        }
        fprintf(tout, " \n</%s>\n", __func__);
        return declarator;
}
/*
<external-declaration> ::= <function-definition>
                         | <declaration>

<function-definition> ::= {<declaration-specifier>}* <declarator> {<declaration>}* <compound-statement>
<declaration> ::=  {<declaration-specifier>}+ {<init-declarator>}* ;
<init-declarator> ::= <declarator>
                    | <declarator> = <initializer>
*/
void external_declaration()
{
        loffset = 0;
        Type *base_t = declaration_specifier();
        if (consume(";"))
                return;
        if (!base_t)
                error_tok(token, "declaration should start with \"type\"");

        Obj *obj = declarator(base_t);
        if (equal(token, "("))
        {
                function_definition(obj);
                return;
        }

        init_declarator(obj);
        while (!consume(";"))
        {
                expect(","); // should consume anywhere else
                obj = declarator(base_t);
                init_declarator(obj);
        }
        return;
}
// ND_FUNC
void function_definition(Obj *declarator)
{
        expect("(");
        Obj *scope = enter_scope(); // scope for function
        parameter_type_list();
        declarator->params = locals;
        declarator->is_function = true;
        //move to
        Obj *var = find_gvar(declarator->token);
        if (var)
        {
                declarator = var;
        }
        else
        {
                declarator->next = globals;
                globals = declarator;
        }
        if (consume(";"))
        { // prototype only
                // TODO:check mismatch
                // Obj *var = find_gvar(declarator->token);
                // if(var->type!=declarator->type)
                leave_scope();
                return;
        }
        Token *tok = consume("{");
        if (!tok)
                error_tok(token, "need block\n");

        // TODO:check mismatch
        if (declarator->body)
                error_tok(declarator->token, "token '%s' is already defined", declarator->token->str);

        enter_scope(); // scope for func
        Node *node = new_node(ND_BLOCK, tok, NULL);
        Type *t = new_type(TY_PTR, ty_char, 8, "char *");
        locals = new_local(tok, locals, t);
        locals->name = "__func__";
        locals->len = strlen(locals->name);

        Node *func_name = new_node(ND_LVAR, tok, t);
        func_name->offset = locals->offset;
        add_node(node,
                 new_node_binary(ND_ASSIGN, func_name,
                                 new_node_string(format("\"%s\"", declarator->name), declarator->token),
                                 tok, t));

        add_node(node, compound_statement(tok, scope));

        // if(loffset != locals->offset)
        //         error_at(token, "loffset != locals->offset");

        leave_scope();
        declarator->body = node;
        // force insert return
        add_node(declarator->body, new_node(ND_RETURN, tok, NULL));

        declarator->stacksize = loffset;

        fprintf(tout, " \n</%s>\n", __func__);
        return;
}
// Node *code[10000] = {0};
Obj *program()
{
        fprintf(tout, " \n<%s>\n", __func__);
        fprintf(tout, " %s\n", user_input);
        while (!at_eof())
        {
                external_declaration();
        }
        fprintf(tout, " \n</%s>\n", __func__);

        return globals;
}
