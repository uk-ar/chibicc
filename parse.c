#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>
#include "9cc.h"

Token *token; // current token

char *filename;
char *user_input;
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

void error_at(char *loc, char *fmt, ...)
{
        va_list ap;
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
        va_start(ap, fmt);
        vfprintf(stderr, fmt, ap);
        fprintf(stderr, "\n");
        exit(1);
}

Type *new_type(TypeKind ty, Type *ptr_to, size_t size)
{ //
        Type *type = calloc(1, sizeof(Type));
        type->kind = ty;
        type->ptr_to = ptr_to;
        type->size = size;
        return type;
}

Token *consume_Token(TokenKind kind)
{
        if (!token || token->kind != kind)
                return NULL;
        Token *ans = token;
        token = token->next;
        return ans;
}

Token *consume(char *op)
{ // if next == op, advance & return true;
        if (strncmp(op, token->pos, strlen(op)) != 0)
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
                error_at(token->pos, "token is not '%s'", op);
        if (strncmp(op, token->pos, strlen(op)) != 0)
                error_at(token->pos, "token is not '%s'", op);
        // printf("t:%s:%d\n",token->pos,token->len);
        token = token->next;
}

int expect_num()
{ //
        if (!token || token->kind != TK_NUM)
        {
                error_at(token->pos, "token is not number");
        }
        int ans = token->val;
        // printf("t:%s:%d\n",token->pos,token->len);
        token = token->next;
        return ans;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
        Token *tok = calloc(1, sizeof(Token));
        tok->kind = kind;
        tok->pos = str; // token->next=NULL;
        tok->len = len;
        cur->next = tok;
        tok->str = format("%.*s", len, str);
        return tok;
}
bool at_eof()
{
        return !token || token->kind == TK_EOF;
}
bool isIdent(char c)
{
        return isdigit(c) || isalpha(c);
}
LVar *strings = NULL;
extern LVar *find_string(Token *tok);
extern LVar *new_var(Token *tok, LVar *next, Type *t);
HashMap *structs, *types;

Token *tokenize(char *p)
{
        Token head, *cur = &head;
        while (*p)
        {
                // fprintf(tout," t:%c\n",*p);
                if (*p == '"')
                {
                        p++;
                        char *s = p;
                        while (*p)
                        {
                                if (*p == '"' && *(p - 1) != '\\')
                                        break;
                                p++;
                        }
                        if (!(*p))
                                error_at(p, "'\"' is not closing");
                        cur = new_token(TK_STR, cur, s, p - s); // skip "
                        LVar *var = find_string(cur);
                        if (var)
                        {
                                p++;
                                continue;
                        }
                        int i = 0;
                        if (strings)
                                i = strings->offset + 1;
                        strings = new_var(cur, strings, NULL);
                        strings->offset = i;
                        p++;
                        continue;
                }
                if (*p == '\'')
                {
                        char *pre = p;
                        cur = new_token(TK_NUM, cur, p, 0);
                        p++;
                        if (*p == '\\')
                        {
                                p++;
                                if (*p == 'a')
                                {
                                        cur->val = 0x7;
                                }
                                else if (*p == 'b')
                                {
                                        cur->val = 0x8;
                                }
                                else if (*p == 'f')
                                {
                                        cur->val = 0xc;
                                }
                                else if (*p == 'n')
                                {
                                        cur->val = 0xa;
                                }
                                else if (*p == 'r')
                                {
                                        cur->val = 0xd;
                                }
                                else if (*p == 't')
                                {
                                        cur->val = 0x9;
                                }
                                else if (*p == 'v')
                                {
                                        cur->val = 0xb;
                                }
                                else if (*p == '\\' || *p == '\'' || *p == '\"' || *p == '\?')
                                {
                                        cur->val = *p;
                                }
                                else
                                {
                                        error_at(p, "cannot use \\%c in char", p);
                                }
                        }
                        else
                        {
                                cur->val = *p;
                        }
                        p++;
                        if (*p != '\'')
                                error_at(p, "'\'' is not closing");
                        p++;
                        cur->len = p - pre;
                }
                if (isspace(*p))
                {
                        p++;
                        continue;
                }
                if (!strncmp(p, "//", 2))
                {
                        while (*p && *p != '\n')
                                p++;
                        p++;
                        continue;
                }
                if (!strncmp(p, "/*", 2))
                {
                        char *q = strstr(p + 2, "*/");
                        if (!(*p))
                                error_at(p, "'/*' is not closing");
                        p = q + 2;
                        continue;
                }
                if (!strncmp(p, "sizeof", 6) && !isIdent(p[6]))
                {
                        cur = new_token(TK_SIZEOF, cur, p, 6);
                        p += 6;
                        continue;
                }
                if (!strncmp(p, "...", 3) && !isIdent(p[3]))
                {
                        cur = new_token(TK_RESERVED, cur, p, 3);
                        p += 3;
                        continue;
                }
                if (!strncmp(p, "int", 3) && !isIdent(p[3]))
                {
                        cur = new_token(TK_TYPE, cur, p, 3);
                        p += 3;
                        continue;
                }
                if (!strncmp(p, "char", 4) && !isIdent(p[4]))
                {
                        cur = new_token(TK_TYPE, cur, p, 4);
                        p += 4;
                        continue;
                }
                if (!strncmp(p, "long", 4) && !isIdent(p[4]))
                {
                        cur = new_token(TK_TYPE, cur, p, 4);
                        p += 4;
                        continue;
                }
                if (!strncmp(p, "struct", 6) && !isIdent(p[6]))
                {
                        cur = new_token(TK_TYPE, cur, p, 6);
                        p += 6;
                        continue;
                }
                if (!strncmp(p, "return", 6) && !isIdent(p[6]))
                {
                        cur = new_token(TK_RETURN, cur, p, 6);
                        p += 6;
                        continue;
                }
                if (!strncmp(p, "if", 2) && !isIdent(p[2]))
                {
                        cur = new_token(TK_IF, cur, p, 2);
                        p += 2;
                        continue;
                }
                if (!strncmp(p, "else", 4) && !isIdent(p[4]))
                {
                        cur = new_token(TK_ELSE, cur, p, 4);
                        p += 4;
                        continue;
                }
                if (!strncmp(p, "while", 5) && !isIdent(p[5]))
                {
                        cur = new_token(TK_WHILE, cur, p, 5);
                        p += 5;
                        continue;
                }
                if (!strncmp(p, "for", 3) && !isIdent(p[3]))
                {
                        cur = new_token(TK_FOR, cur, p, 3);
                        p += 3;
                        continue;
                }
                if (!strncmp(p, "<=", 2) || !strncmp(p, ">=", 2) ||
                    !strncmp(p, "==", 2) || !strncmp(p, "!=", 2) || !strncmp(p, "->", 2))
                {
                        cur = new_token(TK_RESERVED, cur, p, 2);
                        p += 2;
                        continue;
                }
                if (*p == '<' || *p == '>' || *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
                    *p == ')' || *p == '=' || *p == ';' || *p == '{' || *p == '}' || *p == ',' || *p == '&' ||
                    *p == '[' || *p == ']' || *p == '.')
                {
                        cur = new_token(TK_RESERVED, cur, p++, 1);
                        continue;
                }
                if (isdigit(*p))
                {
                        char *pre = p;
                        cur = new_token(TK_NUM, cur, p, 0);
                        cur->val = strtol(p, &p, 10);
                        cur->len = p - pre;
                        // printf("%d",p-pre);
                        continue;
                }
                if (isalpha(*p))
                {
                        char *pre = p;
                        while (isalpha(*p) || isdigit(*p) || *p == '_')
                        {
                                p++;
                        }
                        cur = new_token(TK_IDENT, cur, pre, p - pre);
                        continue;
                }
                // printf("eee");
                error_at(p, " can not tokenize '%c'", *p);
        }
        cur = new_token(TK_EOF, cur, p, 0);
        return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs, Token *token)
{
        Node *ans = calloc(1, sizeof(Node));
        ans->kind = kind;
        ans->lhs = lhs;
        ans->rhs = rhs;
        ans->token = token;
        if (lhs)
                ans->type = lhs->type;
        return ans;
}
Node *new_node_num(int val, Token *token, Type *type)
{
        // leaf node
        Node *ans = calloc(1, sizeof(Node));
        ans->kind = ND_NUM;
        ans->val = val;
        ans->token = token;
        ans->type = type;
        return ans;
}
void add_node(Node *node, Node *new_node)
{
        if (!node->head)
        {
                node->head = new_node;
                node->tail = new_node;
                return;
        }
        node->tail->next2 = new_node;
        node->tail = node->tail->next2;
        return;
}
LVar *locals = NULL;
LVar *globals = NULL;
// HashMap *globals=NULL;
LVar *lstack[100];
int lstack_i = 0;

LVar *find_var(Token *tok, LVar *var0)
{
        for (LVar *var = var0; var; var = var->next)
        {
                if (tok->len == var->len && !memcmp(tok->pos, var->name, tok->len))
                {
                        return var;
                }
        }
        return NULL;
}
LVar *find_lvar(Token *tok)
{
        return find_var(tok, locals);
}
LVar *find_lvar_all(Token *tok)
{
        LVar *ans = find_lvar(tok);
        if (ans)
                return ans;
        for (int i = lstack_i; i >= 0; i--)
        {
                LVar *ans = find_var(tok, lstack[i]);
                if (ans)
                {
                        return ans;
                }
        }
        return NULL;
}
LVar *find_gvar(Token *tok)
{
        // char *s=calloc(tok->len+1,sizeof(char));
        // snprintf(s,tok->len,tok->pos);
        // return get_hash(globals,s);
        return find_var(tok, globals);
}
LVar *find_string(Token *tok)
{
        return find_var(tok, strings);
}
LVar *new_var(Token *tok, LVar *next, Type *t)
{
        LVar *var;
        var = calloc(1, sizeof(LVar));
        var->next = next;
        var->name = tok->str;
        var->len = tok->len;
        var->type = t;
        return var;
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
/* mul     = unary ("*" unary | "/" unary)* */
/* unary   = "-"? primary | "+"? primary | "*" unary | "&" unary  */
/* primary = ident.ident | ident->ident | num | ident | ident "(" exprs? ")" | primary "[" expr "]" | "(" expr ")" | TK_STR*/
Node *expr();
Node *stmt();
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
                        Node *node = new_node(ND_EBLOCK, NULL, NULL, tok);
                        lstack[lstack_i++] = locals;
                        locals = calloc(1, sizeof(LVar));

                        while (!consume("}"))
                        {
                                add_node(node, stmt());
                        }

                        fprintf(tout, " }</%s>\n", __func__);
                        locals = lstack[--lstack_i];
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
        if ((tok = consume_Token(TK_STR)))
        {
                fprintf(tout, "<%s>\"\n", __func__);
                Node *ans = new_node(ND_STR, NULL, NULL, tok);
                fprintf(tout, "\"\n</%s>\n", __func__);
                return ans;
        }
        tok = consume_ident();
        if (tok)
        {
                if (consume("("))
                { // call
                        fprintf(tout, "<%s>funcall\n", __func__);
                        Node *ans = new_node(ND_FUNCALL, NULL, NULL, tok);
                        ans->params = NULL;
                        if (consume(")"))
                        {
                                fprintf(tout, " funcall</%s>\n", __func__);
                                return ans;
                        }
                        ans->params = calloc(6, sizeof(Node *));
                        int i = 0;
                        ans->params[i++] = expr();
                        for (; i < 6 && !consume(")"); i++)
                        {
                                consume(",");
                                ans->params[i] = expr();
                        }
                        fprintf(tout, "funcall</%s>\n", __func__);
                        return ans;
                }
                else
                { // var ref
                        fprintf(tout, "<%s>var\n", __func__);
                        LVar *var = find_lvar_all(tok);
                        Node *ans = NULL;
                        if (!var) // todo fix it for struct;
                        {
                                var = find_gvar(tok);
                                ans = new_node(ND_GVAR, NULL, NULL, tok);
                                if (!var)
                                        error_at(tok->pos, "token '%s' is not defined", tok->pos);
                        }
                        else
                        {
                                ans = new_node(ND_LVAR, NULL, NULL, tok);
                        }

                        ans->offset = var->offset;
                        ans->type = var->type;

                        if (consume("["))
                        {
                                Node *ans1 = new_node(ND_DEREF, new_node(ND_ADD, ans, expr(), NULL), NULL, NULL);
                                ans1->type = var->type->ptr_to;
                                expect("]"); // important
                                fprintf(tout, "array\n</%s>\n", __func__);
                                return ans1;
                        }
                        else if (consume("."))
                        {
                                tok = consume_ident();
                                var = get_hash(structs, ans->type->str);
                                LVar *field = find_var(tok, var);
                                // Node *ans1 = new_node(ND_DEREF, new_node(ND_ADD, ans, new_node_num(field->offset, NULL, new_type(TY_PTR, NULL, 8)), NULL), NULL, NULL);
                                // ans1->type = field->type;
                                // return ans1;
                                ans->type = field->type;
                                ans->offset -= field->offset;
                                return ans;
                        }
                        else if (consume("->"))
                        {
                                tok = consume_ident();                           // field
                                var = get_hash(structs, var->type->ptr_to->str); // vars for s1
                                LVar *field = find_var(tok, var);
                                Node *ans1 = new_node(ND_DEREF, new_node(ND_ADD, new_node_num(field->offset, NULL, new_type(TY_CHAR, NULL, 1)), ans, NULL), NULL, NULL);
                                // Node *ans1 = new_node(ND_ADD, new_node(ND_DEREF, ans, NULL, NULL), new_node_num(field->offset, NULL, new_type(TY_PTR, NULL, 8)), NULL);
                                //  Node *ans1 = new_node(ND_DEREF, ans, NULL, NULL);
                                //  ans1->type = field->type;
                                //  ans->offset += field->offset;
                                //   ans->type->ptr_to = field->type;
                                return ans1;
                                // return ans1;
                        }
                        fprintf(tout, "var\n</%s>\n", __func__);
                        // ans->offset=(tok->pos[0]-'a'+1)*8;
                        return ans;
                }
        }
        fprintf(tout, "<%s>num\n", __func__);
        Node *ans = new_node_num(expect_num(), NULL, new_type(TY_INT, NULL, 4));
        fprintf(tout, "num\n</%s>\n", __func__);
        return ans;
}
/* unary   = "-"? primary | "+"? primary
           | "*" unary | "&" unary  | "sizeof" unary */
Node *unary()
{
        Token *tok = NULL;
        Node *ans = NULL;
        if ((tok = consume_Token(TK_STR)))
        {
                fprintf(tout, " <%s>\"\n", __func__);
                Node *ans = new_node(ND_STR, NULL, NULL, tok);
                fprintf(tout, "\"\n</%s>\n", __func__);
                return ans;
        }
        if ((tok = consume_Token(TK_SIZEOF)))
        {
                fprintf(tout, " <%s>\"\n", __func__);
                Node *node = unary();
                Type *t = node->type;
                fprintf(tout, " sizeof %d\n</%s>\n", t->kind, __func__);
                return new_node_num(t->size, NULL, t);
        }
        if ((tok = consume("+")))
        {
                fprintf(tout, " <%s>+\"\n", __func__);
                ans = primary();
                fprintf(tout, " +\n</%s>\n", __func__);
                return ans;
        }
        if ((tok = consume("-")))
        {
                // important
                fprintf(tout, " <%s>-\"\n", __func__);
                ans = new_node(ND_SUB, new_node_num(0, NULL, new_type(TY_INT, NULL, 4)), primary(), tok); // 0-primary()
                return ans;
                fprintf(tout, " -\n</%s>\n", __func__);
        }
        if ((tok = consume("*")))
        {
                fprintf(tout, " deref\n<%s>\n", __func__);
                Node *lhs = unary();
                Node *node = new_node(ND_DEREF, lhs, NULL, tok);
                node->type = lhs->type->ptr_to;
                fprintf(tout, " deref\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume("&")))
        {
                fprintf(tout, " ref\n<%s>\n", __func__);
                Node *lhs = unary();
                return new_node(ND_ADDR, lhs, NULL, tok);
        }
        return primary();
}
Node *mul()
{
        Token *tok = NULL;
        Node *node = unary();
        for (;;)
        {
                if ((tok = consume("*")))
                {
                        fprintf(tout, " mul\n<%s>\n", __func__);
                        node = new_node(ND_MUL, node, unary(), tok);
                        fprintf(tout, " mul\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("/")))
                {
                        fprintf(tout, " div\n<%s>\n", __func__);
                        node = new_node(ND_DIV, node, unary(), token);
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
                        node = new_node(ND_SUB, node, mul(), tok);
                        fprintf(tout, " sub\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("+")))
                {
                        fprintf(tout, " plus\n<%s>\n", __func__);
                        node = new_node(ND_ADD, node, mul(), tok);
                        fprintf(tout, " plus\n</%s>\n", __func__);
                        continue;
                }
                return node;
        }
}
Node *relational()
{
        Token *tok = NULL;
        Node *node = add();
        for (;;)
        {
                if ((tok = consume("<=")))
                {
                        fprintf(tout, " le\n<%s>\n", __func__);
                        node = new_node(ND_LE, node, add(), tok);
                        fprintf(tout, " le\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume(">=")))
                {
                        fprintf(tout, " le\n<%s>\n", __func__);
                        node = new_node(ND_LE, add(), node, tok); // swap!
                        fprintf(tout, " le\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("<")))
                {
                        fprintf(tout, " lt\n<%s>\n", __func__);
                        node = new_node(ND_LT, node, add(), tok);
                        fprintf(tout, " lt\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume(">")))
                {
                        fprintf(tout, " lt\n<%s>\n", __func__);
                        node = new_node(ND_LT, add(), node, tok); // swap!
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
                        node = new_node(ND_EQ, node, relational(), tok);
                        fprintf(tout, " eq\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("!=")))
                {
                        fprintf(tout, " ne\n<%s>\n", __func__);
                        node = new_node(ND_NE, node, relational(), tok);
                        fprintf(tout, " ne\n</%s>\n", __func__);
                        continue;
                }
                return node;
        }
}

Node *assign()
{
        Token *tok = NULL;
        Node *node = equality();
        if ((tok = consume("=")))
        {
                fprintf(tout, " ass\n<%s>\n", __func__);
                node = new_node(ND_ASSIGN, node, assign(), tok);
                fprintf(tout, " ass\n</%s>\n", __func__);
                return node;
        }
        return node;
}
extern Node *stmt();
Node *expr()
{
        Node *node = NULL;
        node = assign();
        return node;
}

LVar *var_decl(LVar *lvar);
// in order to reset offset
int loffset = 0;
Type *type_specifier()
{
        Token *tok0 = consume_Token(TK_TYPE);
        if (!tok0)
                return NULL;
        char *str0 = tok0->str;
        if (strncmp(str0, "struct", 6) != 0)
                // TODO:union typedef
                return get_hash(types, str0);
        Token *tok1 = consume_ident();
        if (!tok1)
                error_at(tok0, "need identifier for struct\n");
        char *str1 = tok1->str;
        if (!consume("{"))
                // struct reference
                return get_hash(types, format("%s %s", str0, str1));
        LVar *st_vars = calloc(1, sizeof(LVar));
        Type *type = new_type(TY_STRUCT, NULL, 0);
        type->str = str1;
        add_hash(types, format("%s %s", str0, str1), type);
        while (!consume("}"))
        {
                st_vars = var_decl(st_vars);
                consume(";");
        }
        add_hash(structs, str1, st_vars);
        type->size = loffset;
        loffset = 0;
        return type;
}

Node *stmt()
{
        /* stmt       = expr ";"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" expr? ";" expr? ";" expr? ")" stmt
           | "int" ident ";"
           | "return" expr ";" */
        Node *node = NULL;
        Token *tok = consume_Token(TK_TYPE);
        if (tok)
        {
                char *str = tok->str;
                Type *base_t = NULL;
                if (strncmp(tok->str, "struct", 6) == 0)
                {
                        Token *toke = consume_ident();
                        str = format("%s %s", tok->str, toke->str);
                }
                base_t = get_hash(types, str);
                while (base_t)
                {
                        Type *t = base_t;
                        fprintf(tout, " var decl\n<%s>\n", __func__);
                        while (consume("*"))
                                t = new_type(TY_PTR, t, 8);
                        Token *tok = consume_ident(); // ident
                        LVar *var = find_lvar(tok);   //
                        if (var)
                        {
                                error_at(tok->pos, "token '%s' is already defined", tok->pos);
                        }
                        else if (consume("["))
                        {
                                int n = expect_num();
                                locals = new_var(tok, locals, new_type(TY_ARRAY, t, n * t->size));
                                locals->type->array_size = n;
                                int size = locals->type->size;
                                locals->offset = (loffset + size - 1) / size * size; // ajust
                                locals->offset += locals->type->size * n;
                                // locals->offset = loffset + locals->type->size * n; // TODO:fix it
                                expect("]");
                        }
                        else
                        {
                                locals = new_var(tok, locals, t);
                                int size = locals->type->size;
                                locals->offset = (loffset + size - 1) / size * size; // ajust
                                locals->offset += locals->type->size;
                                // locals->offset = loffset + locals->type->size; // TODO:fix it
                        }

                        loffset = locals->offset;
                        fprintf(tout, " var decl\n</%s>\n", __func__);
                        if (consume("="))
                        {
                                if (!node)
                                        node = new_node(ND_BLOCK, NULL, NULL, NULL);
                                Node *lnode = new_node(ND_LVAR, NULL, NULL, tok);
                                lnode->type = t;
                                lnode->offset = locals->offset;
                                add_node(node, new_node(ND_ASSIGN, lnode, assign(), NULL));
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
        }
        if ((tok = consume_Token(TK_RETURN)))
        {
                fprintf(tout, "ret \n<%s>\n", __func__);
                node = new_node(ND_RETURN, node, expr(), tok);
                expect(";");
                fprintf(tout, "ret \n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume_Token(TK_IF)))
        {
                fprintf(tout, " if\n<%s>\n", __func__);
                expect("(");
                node = new_node(ND_IF, NULL, NULL, tok);
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
        if ((tok = consume_Token(TK_WHILE)))
        {
                fprintf(tout, " while\n<%s>\n", __func__);
                expect("(");
                node = new_node(ND_WHILE, NULL, NULL, tok);
                node->cond = expr();
                expect(")");
                node->then = stmt();
                fprintf(tout, " while\n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume_Token(TK_FOR)))
        {
                /* Node *init;//for init */
                /* Node *cond;//if,while,for cond */
                /* Node *next;//for next */
                /* Node *then;//if,while,for then */
                fprintf(tout, " <for>\n");
                expect("(");
                node = new_node(ND_FOR, NULL, NULL, tok);
                fprintf(tout, " <init>\n");
                if (!consume(";"))
                {
                        node->init = expr();
                        expect(";");
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
                        node->next = expr();
                        expect(")");
                }
                fprintf(tout, " </next>\n");
                node->then = stmt();
                fprintf(tout, " </for>\n");
                return node;
        }
        // "{" stmt* "}"
        if ((tok = consume("{")))
        {
                fprintf(tout, " {\n<%s>\n", __func__);
                lstack[lstack_i++] = locals;
                locals = calloc(1, sizeof(LVar));
                node = new_node(ND_BLOCK, NULL, NULL, tok);

                while (!consume("}"))
                {
                        add_node(node, stmt());
                }

                fprintf(tout, " }\n</%s>\n", __func__);
                locals = lstack[--lstack_i];
                return node;
        }
        fprintf(tout, " <%s>\n", __func__);
        node = expr();
        expect(";");
        fprintf(tout, " </%s>\n", __func__);
        return node;
}
Node *arg()
{
        fprintf(tout, " \n<%s>\n", __func__);

        Token *tok = consume_Token(TK_TYPE);
        char *str = tok->str;
        Type *base_t = NULL;
        if (strncmp(tok->str, "struct", 6) == 0)
        {
                Token *toke = consume_ident();
                str = format("%s %s", tok->str, toke->str);
        }
        base_t = get_hash(types, str);
        if (!base_t)
                error_at(token->pos, "declaration should start with \"type\"");

        Type *t = base_t;
        while (consume("*"))
                t = new_type(TY_PTR, t, 8);

        tok = consume_ident();
        Node *ans = new_node(ND_LVAR, NULL, NULL, tok);
        ans->type = t;
        LVar *var = find_lvar(tok);
        if (!var)
        {
                locals = new_var(tok, locals, t);
                // locals->offset = loffset + t->size; // last offset+1;
                int size = t->size;
                locals->offset = (loffset + size - 1) / size * size; // ajust
                locals->offset += size;

                var = locals;
        }
        loffset = locals->offset;
        ans->offset = var->offset; // TODO: shadow
        fprintf(tout, " \n</%s>\n", __func__);
        return ans;
}

LVar *var_decl(LVar *lvar)
{
        Type *base_t = type_specifier();
        if (!base_t)
                error_at(token->pos, "declaration should start with \"type\"");
        /*if (consume(";"))
                return decl();*/
        while (base_t)
        {
                Type *t = base_t;
                while (consume("*"))
                        t = new_type(TY_PTR, t, 8);
                Token *tok = consume_ident();
                if (!tok)
                        return lvar;
                fprintf(tout, " \n<%s>\n", __func__);
                LVar *var = find_lvar(tok); //

                if (var)
                {
                        error_at(tok->pos, "token '%s' is already defined", tok->pos);
                }
                else if (consume("["))
                {
                        int n = expect_num();
                        lvar = new_var(tok, lvar, new_type(TY_ARRAY, t, n * t->size));
                        lvar->type->array_size = n;
                        expect("]");
                }
                else
                {
                        lvar = new_var(tok, lvar, t);
                }
                int size = lvar->type->size;
                lvar->offset = (loffset + size - 1) / size * size;
                loffset = lvar->offset + size;
                if (consume(","))
                {
                        continue;
                }
                fprintf(tout, " \n</%s>\n", __func__);
        }
        return lvar;
}

Node *decl()
{
        Type *base_t = type_specifier();
        if (!base_t)
                error_at(token->pos, "declaration should start with \"type\"");
        if (consume(";"))
                return decl();

        while (base_t)
        {
                Type *t = base_t;
                while (consume("*"))
                        t = new_type(TY_PTR, t, 8);
                Token *tok = consume_ident();
                if (!tok)
                        return NULL;
                fprintf(tout, " \n<%s>\n", __func__);
                LVar *var = find_gvar(tok); //
                if (var)
                {
                        error_at(tok->pos, "token '%s' is already defined", tok->pos);
                }
                if (consume("("))
                { // function declaration
                        Node *ans = new_node(ND_FUNC, NULL, NULL, tok);
                        Node **params = NULL;
                        params = calloc(6, sizeof(Node *));

                        lstack[lstack_i++] = locals;
                        locals = calloc(1, sizeof(LVar));

                        int i = 0;
                        // int off=locals->offset;
                        if (!consume(")"))
                        {
                                params[i++] = arg();
                                for (; i < 6 && !consume(")"); i++)
                                {
                                        consume(",");
                                        params[i] = arg();
                                }
                        }
                        ans->params = params;
                        ans->then = stmt(); // block
                        // ans->then->params=params;
                        ans->offset = loffset;
                        loffset = 0;

                        locals = lstack[--lstack_i];

                        fprintf(tout, " \n</%s>\n", __func__);
                        return ans;
                }
                else if (consume("["))
                {
                        if (consume("]"))
                        {
                                globals = new_var(tok, globals, new_type(TY_ARRAY, t, 0));
                        }
                        else
                        {
                                int n = expect_num();
                                globals = new_var(tok, globals, new_type(TY_ARRAY, t, n * t->size));
                                globals->type->array_size = n;
                                expect("]");
                        }
                        /*expect(";");
                        fprintf(tout," \n</%s>\n",__func__);
                        return decl();*/
                }
                else
                {
                        globals = new_var(tok, globals, t);
                }
                if (consume("="))
                {
                        consume("&");
                        char *p = token->pos;
                        consume_ident();
                        consume("+");
                        if ((tok = consume_Token(TK_STR)))
                        {
                                if (globals->type->kind == TY_ARRAY)
                                {
                                        int n = tok->len;
                                        globals->init = calloc(n + 3, sizeof(char));
                                        snprintf(globals->init, n + 3, "\"%s\"", p);
                                }
                                else
                                {
                                        // int n=token->pos-p;
                                        int n = 15;
                                        globals->init = calloc(n + 1, sizeof(char));
                                        snprintf(globals->init, n, ".LC%d", find_string(tok)->offset);
                                        // globals->init = format(".LC%d", find_string(tok)->offset);
                                        globals->type->array_size = MAX(globals->type->array_size, tok->len);                   // todo fix for escape charactors
                                        globals->type->size = MAX(globals->type->size, tok->len * globals->type->ptr_to->size); // todo fix for escape charactors
                                }
                        }
                        else
                        {
                                consume_Token(TK_NUM);
                                int n = token->pos - p + 1;
                                globals->init = format("%.*s", n, p);
                                // globals->init = calloc(n, sizeof(char));
                                // snprintf(globals->init, n, p);
                        }
                }
                if (consume(","))
                {
                        continue;
                }
                expect(";");
                fprintf(tout, " \n</%s>\n", __func__);
                return decl();
        }
        return NULL;
}

Node *code[100] = {0};
void program()
{
        int i = 0;
        fprintf(tout, " \n<%s>\n", __func__);
        fprintf(tout, " %s\n", user_input);
        while (!at_eof())
        {
                // fprintf(tout," c:%d:%s\n",i,token->pos);
                // fprintf(tout," c:%d:%d\n",i,code[i]->kind);
                code[i++] = decl();
        }
        fprintf(tout, " \n</%s>\n", __func__);
        code[i] = NULL;
}
