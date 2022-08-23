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

HashMap *structs, *types, *keyword2token, *type_alias;
Type *new_type(TypeKind ty, Type *ptr_to, size_t size)
{ //
        Type *type = calloc(1, sizeof(Type));
        type->kind = ty;
        type->ptr_to = ptr_to;
        type->size = size;
        return type;
}

bool equal_Token(Token *tok, TokenKind kind)
{
        if (!tok)
                return false;
        if ((tok->kind != kind) && ((get_hash(keyword2token, tok->str) != (void *)kind)))
                return false;
        return true;
}

Token *consume_Token(TokenKind kind)
{
        if (!equal_Token(token, kind))
                return NULL;
        Token *ans = token;
        token = token->next;
        return ans;
}
bool equal(char *op)
{
        return (strncmp(op, token->pos, strlen(op)) == 0);
}

Token *consume(char *op)
{ // if next == op, advance & return true;
        if (!equal(op))
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
                        cur = new_token(TK_TYPE_SPEC, cur, p, 3);
                        p += 3;
                        continue;
                }
                if (!strncmp(p, "char", 4) && !isIdent(p[4]))
                {
                        cur = new_token(TK_TYPE_SPEC, cur, p, 4);
                        p += 4;
                        continue;
                }
                if (!strncmp(p, "long", 4) && !isIdent(p[4]))
                {
                        cur = new_token(TK_TYPE_SPEC, cur, p, 4);
                        p += 4;
                        continue;
                }
                if (!strncmp(p, "struct", 6) && !isIdent(p[6]))
                {
                        cur = new_token(TK_TYPE_SPEC, cur, p, 6);
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
                        char *str = format("%.*s", p - pre, pre);
                        TokenKind t;
                        if ((t = (TokenKind)get_hash(keyword2token, str)))
                        {
                                cur = new_token(t, cur, pre, p - pre);
                        }
                        else
                        {
                                cur = new_token(TK_IDENT, cur, pre, p - pre);
                        }
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
LVar *struct_declarator_list(LVar *lvar);
// in order to reset offset
int loffset = 0;

LVar *struct_declaration(Type *type)
{
        LVar *st_vars = calloc(1, sizeof(LVar));
        // Type *type = new_type(TY_STRUCT, NULL, 0);
        // type->str = str1;
        // add_hash(types, format("%s %s", type_str, str1), type);
        while (!consume("}"))
        {
                st_vars = struct_declarator_list(st_vars);
                consume(";");
        }
        // add_hash(structs, str1, st_vars);
        type->size = loffset;
        loffset = 0;
        return st_vars;
}

LVar *enumerator_list()
{
        LVar *st_vars = calloc(1, sizeof(LVar));
        st_vars->offset = -1;
        while (!consume("}"))
        {
                Token *tok = consume_ident();
                st_vars = new_var(tok, st_vars, new_type(TY_INT, NULL, 4));
                st_vars->offset = st_vars->next->offset + 1;
                // consume_Token(TK_NUM);
                // int n = token->pos - p + 1;
                globals = new_var(tok, globals, new_type(TY_INT, NULL, 4));
                globals->init = format("%d", st_vars->offset);
                // add_hash(keyword2token, st_vars->name, TK_NUM);
                //  st_vars = var_decl(st_vars);
                consume(",");
        }
        // add_hash(structs, str1, st_vars);
        // loffset = 0;
        return st_vars;
}

Type *declaration_specifier() // bool declaration)
{
        Token *storage = consume_Token(TK_STORAGE);
        //Token *type_qual = consume_Token(TK_TYPE_QUAL);
        Token *type_spec = consume_Token(TK_TYPE_SPEC);
        Token *identifier = NULL;
        //char *def_name = NULL;
        char *src_name = NULL;
        if (!type_spec)
                return NULL;
        char *type_str = type_spec->str;
        if (strncmp(type_str, "struct", 6) == 0)
        {
                Type *type = NULL;
                LVar *st_vars = NULL;
                if (consume("{"))
                { // anonymous
                        type = new_type(TY_STRUCT, NULL, 0);
                        st_vars = struct_declaration(type);
                }
                else if ((identifier = consume_ident())) // struct name
                {
                        src_name = format("%s %s", type_str, identifier->str);
                        if (consume("{"))
                        {
                                type = new_type(TY_STRUCT, NULL, 0);
                                type->str = identifier->str;
                                add_hash(types, src_name, type); // for recursive field type
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
                        error_at(token->pos, "need identifier for struct\n");
                }
                if (storage && (strncmp(storage->str, "typedef", 6) == 0))
                {
                        Token *declarator = consume_ident(); // defname
                        if (!declarator)
                                error_at(token->pos, "typedef need declarator for struct\n");
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
                                error_at(token->pos, "typedef need identifier for struct\n");
                        }
                        add_hash(keyword2token, declarator->str, (void *)TK_TYPE_SPEC);
                }
                return type;
        }
        if (strcmp(type_str, "enum") == 0)
        {
                Type *type = NULL;
                LVar *st_vars = NULL;
                if (consume("{"))
                { // anonymous
                        // type = new_type(TY_STRUCT, NULL, 0);
                        // st_vars = struct_declaration(type);
                        st_vars = enumerator_list();
                }
                else if ((identifier = consume_ident())) // struct name
                {
                        src_name = format("%s %s", type_str, identifier->str);
                        if (consume("{"))
                        {
                                /*type = new_type(TY_STRUCT, NULL, 0);
                                type->str = identifier->str;
                                add_hash(types, src_name, type); // for recursive field type
                                st_vars = struct_declaration(type);
                                add_hash(structs, src_name, st_vars);*/
                                // type = new_type(TY_STRUCT, NULL, 0);
                                st_vars = enumerator_list();
                                add_hash(types, src_name, new_type(TY_INT, NULL, 4));
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
                        error_at(token->pos, "need identifier for struct\n");
                }
                if (storage && (strncmp(storage->str, "typedef", 6) == 0))
                {
                        Token *declarator = consume_ident(); // defname
                        if (!declarator)
                                error_at(token->pos, "typedef need declarator for struct\n");
                        if (src_name)
                        {
                                add_hash(type_alias, declarator->str, src_name);
                        }
                        add_hash(types, declarator->str, new_type(TY_INT, NULL, 4));
                        add_hash(keyword2token, declarator->str, (void *)TK_TYPE_SPEC);
                }
                return type;
        }
        else
        {
                if (storage && (strncmp(storage->str, "typedef", 6) == 0))
                {
                        Token *declarator = consume_ident();
                        if (!declarator) // || !type || !st_vars)
                                error_at(token->pos, "need declarator for struct\n");
                        // src_name = format("%s %s", type_str, identifier->str);
                        // add_hash(types, declarator->str, type);
                        add_hash(keyword2token, declarator->str, (void *)TK_TYPE_SPEC);
                        add_hash(type_alias, declarator->str, src_name);
                }
                while (get_hash(type_alias, type_str))
                {
                        type_str = get_hash(type_alias, type_str);
                }
                return get_hash(types, type_str); // support typedef
        }
        /*return get_hash(types, type_str); // support typedef
                                          // TODO:union typedef

        Token *identifier = consume_ident();
        if (!identifier)
        { // anonymouse
                Type *type = new_type(TY_STRUCT, NULL, 0);
                return struct_declaration(type);
                error_at(token->pos, "need identifier for struct\n");
        }
        char *str1 = identifier->str;

        if (!consume("{"))
                // struct reference
                return get_hash(types, format("%s %s", type_str, str1));
        Type *type = new_type(TY_STRUCT, NULL, 0);
        return struct_declaration(type);*/
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
/* unary   = "-"? primary | "+"? primary | "*" unary | "&" unary  */
/* postfix */
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
        else if ((tok = consume_Token(TK_STR)))
        {
                fprintf(tout, "<%s>\"\n", __func__);
                Node *ans = new_node(ND_STR, NULL, NULL, tok);
                fprintf(tout, "\"\n</%s>\n", __func__);
                return ans;
        }
        else if ((tok = consume_ident()))
        {
                if (consume("("))
                { // call
                        fprintf(tout, "<%s>funcall\n", __func__);
                        Node *ans = new_node(ND_FUNCALL, NULL, NULL, tok);
                        if (consume(")"))
                        {
                                fprintf(tout, " funcall</%s>\n", __func__);
                                return ans;
                        }
                        for (int i = 0; i < 6 && !consume(")"); i++)
                        {
                                add_node(ans, expr());
                                consume(",");
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
                                        error_at(tok->pos, "token '%s' is not defined", tok->str);
                        }
                        else
                        {
                                ans = new_node(ND_LVAR, NULL, NULL, tok);
                        }

                        ans->offset = var->offset;
                        ans->type = var->type;

                        fprintf(tout, "var\n</%s>\n", __func__);
                        // ans->offset=(tok->pos[0]-'a'+1)*8;
                        return ans;
                }
        }
        Type *t = declaration_specifier();
        if (t)
        {
                Node *ans = new_node_num(0, NULL, t);
                return ans;
        }
        fprintf(tout, "<%s>num\n", __func__);
        // TODO:add enum
        Node *ans = new_node_num(expect_num(), NULL, new_type(TY_INT, NULL, 4));
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
Node *postfix()
{
        Token *tok = NULL;
        Node *ans = primary();
        if ((tok = (consume("["))))
        {
                Node *ans1 = new_node(ND_DEREF, new_node(ND_ADD, ans, expr(), NULL), NULL, NULL);
                ans1->type = ans->type->ptr_to;
                expect("]"); // important
                fprintf(tout, "array\n</%s>\n", __func__);
                return ans1;
        }
        else if ((tok = (consume("."))))
        {
                tok = consume_ident();
                LVar *var = get_hash(structs, format("struct %s", ans->type->str));
                LVar *field = find_var(tok, var);
                // Node *ans1 = new_node(ND_DEREF, new_node(ND_ADD, ans, new_node_num(field->offset, NULL, new_type(TY_PTR, NULL, 8)), NULL), NULL, NULL);
                // ans1->type = field->type;
                // return ans1;
                ans->type = field->type;
                ans->offset -= field->offset;
                return ans;
        }
        else if ((tok = consume("->")))
        {
                tok = consume_ident();                                                      // field
                LVar *var = get_hash(structs, format("struct %s", ans->type->ptr_to->str)); // vars for s1
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
        // TODO:++
        // TODO:--
        return ans;
}
Type *parameter_type_list(Node *ans);
Type *abstract_declarator(Type *t);
Type *direct_abstract_declarator(Type *t)
{
        if (consume("("))
        {
                if ((t = abstract_declarator(t)))
                {
                        expect(")");
                }
                else
                {
                        t = parameter_type_list(NULL);
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
                t = new_type(TY_PTR, t, 8);
        t = direct_abstract_declarator(t);
        return t;
}

Type *type_name()
{
        // specifier-qualifier
        consume_Token(TK_TYPE_QUAL);
        Token *t = consume_Token(TK_TYPE_SPEC);
        if (t)
                return abstract_declarator(get_hash(types, t->str));
        return abstract_declarator(NULL);
}

/* unary   = "-"? primary | "+"? primary
           | "*" unary | "&" unary  | "sizeof" unary */
Node *unary()
{
        Token *tok = NULL;
        Node *ans = NULL;
        if ((tok = consume_Token(TK_STR))) // TODO:fix it
        {
                fprintf(tout, " <%s>\"\n", __func__);
                Node *ans = new_node(ND_STR, NULL, NULL, tok);
                fprintf(tout, "\"\n</%s>\n", __func__);
                return ans;
        }
        if ((tok = consume_Token(TK_SIZEOF)))
        {
                fprintf(tout, " <%s>\"\n", __func__);
                Type *t = NULL;
                if (equal("(") && equal_Token(token->next, TK_TYPE_SPEC))
                {
                        consume("(");
                        t = type_name();
                        expect(")");
                        return new_node_num(t->size, NULL, t);
                }
                if (equal_Token(token->next, TK_TYPE_SPEC))
                {
                        t = type_name();
                        fprintf(tout, " sizeof %d\n</%s>\n", t->kind, __func__);
                        return new_node_num(t->size, NULL, t);
                }
                t = unary()->type;
                fprintf(tout, " sizeof %d\n</%s>\n", t->kind, __func__);
                return new_node_num(t->size, NULL, t);
        }
        // TODO:++ unary
        // TODO:-- unary
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
                ans = new_node(ND_SUB, new_node_num(0, NULL, new_type(TY_INT, NULL, 4)), unary(), tok); // 0-primary()
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
        return postfix();
}

Node *cast()
{
        if (equal("(") &&
            token->next &&
            equal_Token(token->next, TK_TYPE_SPEC))//TODO: fix to is_typename
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
                        node = new_node(ND_MUL, node, mul(), tok);
                        fprintf(tout, " mul\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("/")))
                {
                        fprintf(tout, " div\n<%s>\n", __func__);
                        node = new_node(ND_DIV, node, mul(), token);
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
                        node = new_node(ND_SUB, node, add(), tok);
                        fprintf(tout, " sub\n</%s>\n", __func__);
                        continue;
                }
                if ((tok = consume("+")))
                {
                        fprintf(tout, " plus\n<%s>\n", __func__);
                        node = new_node(ND_ADD, node, add(), tok);
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

Node *compound_statement()
{
        fprintf(tout, " {\n<%s>\n", __func__);
        lstack[lstack_i++] = locals;
        locals = calloc(1, sizeof(LVar));
        Node *node = new_node(ND_BLOCK, NULL, NULL, NULL);

        while (!consume("}"))
        {
                add_node(node, stmt());
        }

        fprintf(tout, " }\n</%s>\n", __func__);
        locals = lstack[--lstack_i];
        return node;
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
        Token *tok = NULL;
        Type *base_t = declaration_specifier();
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
                        error_at(tok->pos, "token '%s' is already defined", tok->str);
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
                /*{
                        if (!node)
                                node = new_node(ND_BLOCK, NULL, NULL, NULL);
                        initializer(node, t, tok);
                }*/
                {
                        if (!node)
                        {
                                node = new_node(ND_BLOCK, NULL, NULL, NULL);
                                // node->type = t;
                        }
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
        if ((tok = consume_Token(TK_RETURN))) // jump-statement
        {
                fprintf(tout, "ret \n<%s>\n", __func__);
                node = new_node(ND_RETURN, node, expr(), tok);
                expect(";");
                fprintf(tout, "ret \n</%s>\n", __func__);
                return node;
        }
        if ((tok = consume_Token(TK_IF))) // selection-statement
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
        if ((tok = consume_Token(TK_WHILE))) // iteration-statement
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
        if ((tok = consume_Token(TK_FOR))) // iteration-statement
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
        if ((tok = consume("{"))) // compound-statement
                return compound_statement();
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
        // error_at(token->pos, "declaration should start with \"type\"");

        /*Type *t = base_t;
        //while (consume("*"))
                t = new_type(TY_PTR, t, 8);*/
        Type *t = abstract_declarator(base_t);
        Token *tok = consume_ident();
        Node *ans = new_node(ND_LVAR, NULL, NULL, tok);
        ans->type = t;
        if (!tok) // type only
                return ans;
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
LVar *struct_declarator_list(LVar *lvar)
{
        Type *base_t = declaration_specifier();
        if (!base_t)
                error_at(token->pos, "declaration should start with \"type\"");
        while (base_t)
        {
                Type *t = base_t;
                t = abstract_declarator(t);
                Token *tok = consume_ident();
                if (!tok)
                        return lvar;
                fprintf(tout, " \n<%s>\n", __func__);
                LVar *var = find_lvar(tok); //

                if (var)
                {
                        error_at(tok->pos, "token '%s' is already defined", tok->str);
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
void initilizer(bool top)
{
        consume("&");
        char *p = token->pos;
        consume_ident();
        consume("+");
        Token *tok;
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

Type *parameter_type_list(Node *ans) // it should return LVar*?
{
        /*Type *t = declaration_specifier();

        if (consume(","))
                return parameter_type_list();
        return t;*/
        lstack[lstack_i++] = locals;
        locals = calloc(1, sizeof(LVar));
        // int off=locals->offset;
        Type *t = NULL;
        if (!consume("void"))
        {
                for (int i = 0; i < 6 && !consume(")"); i++)
                {
                        if (consume("..."))
                        {
                                if (consume(")"))
                                        break;
                                else
                                        error_at(token->pos, "va arg error\n");
                        }
                        if (ans)
                                add_node(ans, parameter_declaration());
                        else
                        {
                                Node *n = parameter_declaration();
                                if (n)
                                        t = n->type;
                        }
                        consume(",");
                }
        }
        else if (!consume(")"))
        {
                error_at(token->pos, "arg void\n");
        }
        return t;
}

Node *declaration(bool top);
Node *init_declarator(Type *base_t, bool top)
{
        // LVar *vars = globals;
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
                error_at(tok->pos, "token '%s' is already defined", tok->str);
        }
        if (consume("("))
        { // function declaration
                Node *ans = new_node(ND_FUNC, NULL, NULL, tok);
                parameter_type_list(ans);
                if (consume(";"))
                        return declaration(top);
                if (!consume("{"))
                        error_at(token->pos, "need block\n");
                ans->then = compound_statement(); // block
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
        }
        else
        {
                globals = new_var(tok, globals, t);
        }
        if (consume("="))
        {
                initilizer(top);
        }
        if (consume(","))
        {
                return init_declarator(base_t, top);
        }
        expect(";");
        fprintf(tout, " \n</%s>\n", __func__);
        return declaration(top);
}

Node *declaration(bool top)
{
        Type *base_t = declaration_specifier();
        if (consume(";"))
                return declaration(top);
        if (!base_t)
                error_at(token->pos, "declaration should start with \"type\"");

        return init_declarator(base_t, top);
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
                code[i++] = declaration(true);
        }
        fprintf(tout, " \n</%s>\n", __func__);
        code[i] = NULL;
}
