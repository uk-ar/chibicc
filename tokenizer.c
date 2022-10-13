typedef long unsigned int size_t;
extern int strncmp(const char *__s1, const char *__s2, size_t __n);
extern char *strstr(const char *__haystack, const char *__needle);
extern long int strtol(const char *__restrict __nptr,
                       char **__restrict __endptr, int __base);
extern void *calloc(size_t __nmemb, size_t __size);

#include "yucc.h"

Token *new_token(TokenKind kind, Token *cur, char *str, int len, int loc)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->pos = str; // token->next=NULL;
    tok->len = len;
    cur->next = tok;
    tok->str = format("%.*s", len, str);
    tok->loc = loc;
    return tok;
}

bool isIdent(char c)
{
    return isdigit(c) || isalpha(c);
}

static int loc = 1;
Token *tokenize(char *p)
{
    Token head, *cur = &head;
    while (*p)
    {
        // fprintf(tout," t:%c\n",*p);
        if (*p == '"')
        {
            char *s = p;
            p++;
            while (*p)
            {
                if (*p == '"' && *(p - 1) != '\\')
                    break;
                p++;
            }
            if (!(*p))
                error_at(p, "'\"' is not closing");
            cur = new_token(TK_STR, cur, s, p - s + 1, loc); // include " in order not to match consume
            p++;
            continue;
        }
        if (*p == '\'')
        {
            char *pre = p;
            char val = 0;
            p++;
            if (*p == '\\')
            {
                p++;
                if (*p == 'a')
                {
                    val = 0x7;
                }
                else if (*p == '0')
                {
                    val = 0x0;
                }
                else if (*p == 'b')
                {
                    val = 0x8;
                }
                else if (*p == 'f')
                {
                    val = 0xc;
                }
                else if (*p == 'n')
                {
                    val = 0xa;
                }
                else if (*p == 'r')
                {
                    val = 0xd;
                }
                else if (*p == 't')
                {
                    val = 0x9;
                }
                else if (*p == 'v')
                {
                    val = 0xb;
                }
                else if (*p == '\\' || *p == '\'' || *p == '\"' || *p == '\?')
                {
                    val = *p;
                }
                else
                {
                    error_at(p, "cannot use \\%c in char", p);
                }
            }
            else
            {
                val = *p;
            }
            p++;
            if (*p != '\'')
                error_at(p, "'\'' is not closing");
            p++;
            cur = new_token(TK_NUM, cur, pre, p - pre, loc);
            cur->val = val;
            continue;
        }
        if (isspace(*p))
        {
            if (*p == '\n')
                loc++;
            p++;
            continue;
        }
        if (!strncmp(p, "//", 2))
        {
            while (*p && *p != '\n')
                p++;
            p++;
            loc++;
            continue;
        }
        if (!strncmp(p, "/*", 2))
        {
            char *q = strstr(p + 2, "*/");
            char *r = strstr(p + 2, "\n");
            while (r < q)
            {
                loc++;
                r = strstr(r + 1, "\n");
            }
            if (!(*p))
                error_at(p, "'/*' is not closing");
            p = q + 2;
            continue;
        }
        if (!strncmp(p, "...", 3) && !isIdent(p[3]))
        {
            cur = new_token(TK_RESERVED, cur, p, 3, loc);
            p += 3;
            continue;
        }
        if (!strncmp(p, "<=", 2) || !strncmp(p, ">=", 2) ||
            !strncmp(p, "==", 2) || !strncmp(p, "!=", 2) || !strncmp(p, "->", 2) ||
            !strncmp(p, "++", 2) || !strncmp(p, "--", 2) ||
            !strncmp(p, "+=", 2) || !strncmp(p, "-=", 2) ||
            !strncmp(p, "/=", 2) || !strncmp(p, "*=", 2) ||
            !strncmp(p, "%=", 2) || !strncmp(p, "||", 2) || !strncmp(p, "&&", 2))
        {
            cur = new_token(TK_RESERVED, cur, p, 2, loc);
            p += 2;
            continue;
        }
        if (*p == '<' || *p == '>' || *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' ||
            *p == ')' || *p == '=' || *p == ';' || *p == '{' || *p == '}' || *p == ',' || *p == '&' ||
            *p == '[' || *p == ']' || *p == '.' || *p == '!' || *p == '%' || *p == ':' || *p == '?')
        {
            cur = new_token(TK_RESERVED, cur, p++, 1, loc);
            continue;
        }
        if (isdigit(*p))
        {
            char *pre = p;
            long val = strtol(p, &p, 0);
            cur = new_token(TK_NUM, cur, pre, p - pre, loc);
            cur->val = val;
            // printf("%d",p-pre);
            continue;
        }
        if (isalpha(*p) || *p == '_')
        {
            char *pre = p;
            while (isalpha(*p) || isdigit(*p) || *p == '_')
            {
                p++;
            }
            char *str = format("%.*s", p - pre, pre);
            TokenKind t = (TokenKind)get_hash(keyword2token, str);
            if (t == TK_NOT_SUPPORT)
            {
                continue;
            }
            if (t == TK_NOT_EXIST)
            {
                cur = new_token(TK_IDENT, cur, pre, p - pre, loc);
            }
            else
            {
                cur = new_token(t, cur, pre, p - pre, loc);
            }
            continue;
        }
        // printf("eee");
        error_at(p, " can not tokenize '%c'", *p);
    }
    cur = new_token(TK_EOF, cur, p, 0, loc);
    return head.next;
}
