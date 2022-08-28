
struct _IO_FILE;
typedef struct _IO_FILE FILE;
int fprintf(FILE *__restrict __stream, const char *__restrict __fmt, ...);

//#include <stdlib.h>
typedef long unsigned int size_t;
extern void *calloc(size_t __nmemb, size_t __size);
//#include <stdio.h>
extern int printf(const char *__restrict __fmt, ...);
// extern int printf(const char * __fmt, ...);
//#include <string.h>
extern int strcmp(const char *__s1, const char *__s2);
//#include <stddef.h>
#define NULL ((void *)0)

#include "9cc.h"
/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

*/
FILE *tout2;
char *nodeKind[] = {
    "ND_CONTINUE",
    "ND_COND",
    "ND_EXPR",
    "ND_CASE",
    "ND_BREAK",
    "ND_SWITCH",
    "ND_OR",
    "ND_AND",
    "ND_MOD",
    "ND_CAST",
    "ND_STR",
    "ND_GVAR",
    "ND_DEREF",
    "ND_ADDR",
    "ND_FUNC",
    "ND_FUNCALL",
    "ND_EBLOCK",
    "ND_BLOCK",
    "ND_IF",
    "ND_ELSE",
    "ND_WHILE",
    "ND_FOR",
    "ND_ADD",
    "ND_SUB",
    "ND_MUL",
    "ND_DIV",
    "ND_NUM",
    "ND_LVAR",   // variable
    "ND_ASSIGN", // variable
    "ND_RETURN", // return
    "ND_LT",
    "ND_GT",
    "ND_EQ",
    "ND_NE",
    "ND_LE",
    "ND_GE",
};

int align = 0;
char *push(char *reg)
{
        align++;
        return format("  push %s\n", reg);
}
char *pop(char *reg)
{
        align--;
        return format("  pop %s\n", reg);
}
Type *gen_lval(Node *node)
{ // push address
        fprintf(tout2, "#lvar <%s>\n", nodeKind[node->kind]);
        if (node->kind == ND_GVAR)
        {
                printf("  lea rax, %s[rip]\n", node->token->str); // base pointer
                // printf("  lea rax, rip[%s]\n",strndup(node->token->pos,node->token->len));//base pointer
                // printf("  mov rax, %s\n",strndup(node->token->pos,node->token->len));//base pointer
                // printf("  push rax\n"); // save local variable address
                printf(push("rax"));
                /* fprintf(tout2,"#lvar </%s>\n",nodeKind[node->kind]); */
                return node->type;
        }
        else if (node->kind == ND_LVAR)
        {
                printf("  mov rax, rbp\n"); // base pointer
                printf("  sub rax, %d\n", node->offset);
                // printf("  push rax\n"); // save local variable address
                printf(push("rax"));
                // printf(push(format("[rax-%d]",node->offset)));
                fprintf(tout2, "#lvar </%s>\n", nodeKind[node->kind]);
                return node->type;
        }
        else if (node->kind == ND_DEREF)
        {
                Type *t = gen(node->lhs); // address is in stack
                fprintf(tout2, "#lvar </%s>\n", nodeKind[node->kind]);
                return t;
        }
        else
        {
                error_at(node->token->pos, "token is not lvalue\n", node->token->str);
        }
}
int count()
{
        static int cnt = 0;
        return cnt++;
}
void dump()
{
        printf("  mov rsi, rsp\n");
        printf("  mov edi, OFFSET FLAT:.LCdebug\n");
        printf("  mov eax, 0\n");
        printf("  call printf\n");
}
extern LVar *find_string(Token *tok);
char *break_labels[100];
char *continue_labels[100];
int depth = 0;
// static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
Type *gen(Node *node)
{
        char *nodeK = nodeKind[node->kind];
        fprintf(tout2, "# <%s>\n", nodeK);
        if (node->kind == ND_STR)
        {
                LVar *var = find_string(node->token);
                // printf("  .loc 1 %d\n", node->token->loc);
                printf("  mov rax , OFFSET FLAT:.LC%d\n", var->offset);
                printf(push("rax"));
                // printf("  push rax\n");
                return node->type;
        }
        if (node->kind == ND_FUNC)
        {
                printf("  .text \n");
                printf("  .global %s\n", node->token->str);
                printf("  .type %s, @function\n", node->token->str);
                printf("%s:\n", node->token->str);
                // printf("  .loc 1 %d\n", node->token->loc);
                align = 0;
                // dump();
                printf("  push rbp\n");                                   // save base pointer
                printf("  mov rbp, rsp\n");                               // save stack pointer
                printf("  sub rsp, %d\n", (node->offset + 15) / 16 * 16); // num of vals*8byte
                Node *n = node->head;
                for (int i = 0; i < 6 && n; i++, n = n->next2)
                {
                        // printf("  mov rax, %s\n", argreg[i]); // args to local
                        // printf("  push rax\n");               // args to local
                        printf("  mov rbx, %s\n", argreg[i]); // args to local
                        if (n->type->kind == TY_CHAR)
                        {
                                printf("  mov BYTE PTR [rbp-%d], bl\n", n->offset); // get data from address
                        }
                        else if (n->type->kind == TY_INT)
                        {
                                printf("  mov DWORD PTR [rbp-%d], ebx\n", n->offset); // get data from address
                        }
                        else
                        {
                                printf("  mov QWORD PTR [rbp-%d], rbx\n", n->offset); // get data from address
                                // printf("  mov rax, rbx\n"); // get data from address
                        }
                        // printf("  push rax\n"); // args to local
                }
                // dump();
                gen(node->then);
                return NULL;
        }
        if (node->kind == ND_NUM)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                //  printf("  push %d\n", node->val);
                printf(push(format("%d", node->val)));
                fprintf(tout2, "# %d</%s>\n", node->val, nodeKind[node->kind]);
                return node->type;
        }
        else if (node->kind == ND_LVAR || node->kind == ND_GVAR)
        { // local value
                // printf("  .loc 1 %d\n", node->token->loc);
                Type *t = gen_lval(node); // get address
                if (t->kind == TY_ARRAY || t->kind == TY_STRUCT)
                {
                        return t;
                }
                printf(pop("rbx")); // get address
                // printf("  pop rbx\n"); // get address
                if (t->kind == TY_CHAR)
                {
                        printf("  movsx eax, BYTE PTR [rbx]\n"); // get data from address
                }
                else if (t->kind == TY_INT)
                {
                        printf("  mov eax, DWORD PTR [rbx]\n"); // get data from address
                        printf("  cdqe\n"); 
                }
                else
                {
                        printf("  mov rax, QWORD PTR [rbx]\n"); // get data from address
                }
                printf(push("rax"));
                // printf("  push rax\n"); // save local variable value
                fprintf(tout2, "# </%s>\n", nodeK);
                return t;
        }
        else if (node->kind == ND_ASSIGN)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                Type *t = gen_lval(node->lhs);
                gen(node->rhs);
                printf(pop("rax")); // rhs
                printf(pop("rbx")); // lhs                
                if (node->type->kind == TY_CHAR)
                {
                        printf("  mov BYTE PTR [rbx],al\n");
                } // TODO:Add short type
                else if (node->type->kind == TY_INT)
                {
                        printf("  mov DWORD PTR [rbx],eax\n");
                        printf("  cdqe\n");
                }
                else
                { // todo fix for struct
                        printf("  mov [rbx],rax\n");
                }
                printf(push("rax"));
                // printf("  push [rax]\n"); // save expression result(ex. a=b=c)
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return t;
        }
        else if (node->kind == ND_RETURN)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                if (node->rhs)
                {
                        gen(node->rhs);
                        printf(pop("rax")); // move result to rax
                }
                // printf("  pop rax\n");     //
                printf("  mov rsp,rbp\n"); // restore stack pointer
                printf("  pop rbp\n");     // restore base pointer
                printf("  ret\n");
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return NULL;
        }
        else if (node->kind == ND_IF)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                fprintf(tout2, "# <cond>\n");
                gen(node->cond);
                fprintf(tout2, "# </cond>\n");
                printf(pop("rax")); // move result to rax

                printf("  cmp rax, 0\n");
                int num = count();
                printf("  je .Lelse%d\n", num);
                fprintf(tout2, "# <then>\n");
                gen(node->then);
                fprintf(tout2, "# </then>\n");
                printf("  jmp .Lend%d\n", num);
                printf(".Lelse%d:\n", num);
                if (node->els)
                {
                        gen(node->els);
                }
                printf(".Lend%d:\n", num);
                // printf("  push 0\n", num);//
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_COND)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                fprintf(tout2, "# <cond>\n");
                gen(node->cond);
                fprintf(tout2, "# </cond>\n");
                printf(pop("rax")); // move result to rax
                printf("  cmp rax, 0\n");
                int num = count();

                printf("  je .Lelse%d\n", num);
                fprintf(tout2, "# <then>\n");
                gen(node->then);
                fprintf(tout2, "# </then>\n");
                printf("  jmp .Lend%d\n", num);

                printf(".Lelse%d:\n", num);
                gen(node->els);

                printf(".Lend%d:\n", num);
                // printf("  push 0\n", num);//
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_SWITCH)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                int num = count();
                gen(node->cond);
                printf(pop("rax")); // move result to rax
                for (HashNode *c = node->cases->begin; c; c = c->next)
                {
                        printf("  cmp rax, %d\n", (int)c->value);
                        printf("  je .Lcase%s\n", c->key);
                }
                if (node->els)
                {
                        printf("  jmp .Ldefault%d\n", node->els->val);
                }

                break_labels[depth] = format(".Lend%d", num);
                fprintf(tout2, "# <then>\n");
                depth++;
                gen(node->then);
                depth--;
                fprintf(tout2, "# </then>\n");

                printf("%s:\n", break_labels[depth]); // for break;
                // fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_BREAK)
        {
                int d = depth;
                while (d > 0 && !break_labels[d])
                        d--;
                if (!break_labels[d])
                        error_at(node->token->pos, "no break point");
                printf("  jmp %s\n", break_labels[d]);
                return NULL;
        }
        else if (node->kind == ND_CONTINUE)
        {
                int d = depth;
                while (d > 0 && !continue_labels[d])
                        d--;
                if (!continue_labels[d])
                        error_at(node->token->pos, "no break point");
                printf("  jmp %s\n", continue_labels[d]);
                return NULL;
        }
        else if (node->kind == ND_CASE)
        {
                if (node->lhs)
                        printf(".Lcase%d:\n", node->val);
                else
                        printf(".Ldefault%d:\n", node->val);
                return NULL;
        }
        else if (node->kind == ND_AND)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                gen(node->lhs);
                printf(pop("rax")); // move result to rax

                int num = count();
                printf("  cmp rax, 0\n");
                printf("  je .Lend%d\n", num);

                gen(node->rhs);     // result is in stack
                printf(pop("rax")); // move result to rax

                printf(".Lend%d:\n", num);
                printf(push("rax"));
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return NULL;
        }
        else if (node->kind == ND_OR)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                gen(node->lhs);
                printf(pop("rax")); // move result to rax

                int num = count();
                printf("  cmp rax, 0\n");
                printf("  jne .Lend%d\n", num);

                gen(node->rhs);     // result is in stack
                printf(pop("rax")); // move result to rax

                printf(".Lend%d:\n", num);
                printf(push("rax"));
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return NULL;
        }
        else if (node->kind == ND_WHILE)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                int num = count();
                continue_labels[depth] = format(".Lbegin%d", num);
                break_labels[depth] = format(".Lend%d", num);
                
                printf("%s:\n", continue_labels[depth]);
                fprintf(tout2, "# <cond>\n");
                gen(node->cond);
                fprintf(tout2, "# </cond>\n");
                printf(pop("rax")); // move result to rax
                printf("  cmp rax, 0\n");
                printf("  je %s\n", break_labels[depth]);

                fprintf(tout2, "# <then>\n");
                gen(node->then);
                fprintf(tout2, "# </then>\n");

                printf("  jmp %s\n", continue_labels[depth]);
                printf("%s:\n", break_labels[depth]);
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_FOR)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                int num = count();
                continue_labels[depth] = format(".Lnext%d", num);
                break_labels[depth] = format(".Lend%d", num);
                char *cond_label = format(".Lbegin%d", num);

                if (node->init)
                {
                        fprintf(tout2, "# <init>\n");
                        gen(node->init); // block
                        // printf(pop("rax")); // move result to rax
                        fprintf(tout2, "# <init>\n");
                }

                printf("%s:\n", cond_label);
                if (node->cond)
                {
                        fprintf(tout2, "# <cond>\n");
                        gen(node->cond);
                        printf(pop("rax")); // move result to rax
                        fprintf(tout2, "# </cond>\n");
                }

                // printf("  pop rax\n"); // move result to rax
                printf("  cmp rax, 0\n");
                printf("  je %s\n", break_labels[depth]);
                if (node->then)
                {
                        fprintf(tout2, "# <then>\n");
                        gen(node->then);
                        // block
                        fprintf(tout2, "# </then>\n");
                }

                printf("%s:\n", continue_labels[depth]);
                if (node->next)
                {
                        depth++;
                        fprintf(tout2, "# <next>\n");
                        gen(node->next);
                        printf(pop("rax")); // move result to rax
                        fprintf(tout2, "# </next>\n");
                        depth--;
                }

                printf("  jmp %s\n", cond_label);
                printf("%s:\n", break_labels[depth]);
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_BLOCK)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                for (Node *c = node->head; c; c = c->next2)
                {
                        gen(c);
                        if (c->kind != ND_IF && c->kind != ND_BLOCK && c->kind != ND_SWITCH && c->kind != ND_CASE && c->kind != ND_BREAK && c->kind != ND_FOR)
                                printf(pop("rax")); // move result to rax
                                                    // printf("  pop rax\n"); // move result to remove
                }
                // printf("  push 0\n"); // same behavior as ({;})
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_EBLOCK)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                for (Node *c = node->head; c; c = c->next2)
                {
                        gen(c);
                        if (c->next2 && c->kind != ND_BLOCK && c->kind != ND_IF && c->kind != ND_SWITCH && c->kind != ND_CASE && c->kind != ND_BREAK && c->kind != ND_FOR)
                                printf(pop("rax")); // move result to rax
                                                    // printf("  pop rax\n"); // move result to remove
                        /*if(c->kind!=ND_BLOCK && c->kind!=ND_ELSE && c->kind!=ND_FOR &&
                                c->kind!=ND_FUNC && c->kind!=ND_IF && c->kind!=ND_RETURN && c->kind!=ND_WHILE)
                                printf("  pop rax\n"); // move result to remove*/
                }
                // printf("  push 0\n"); // same behavior as ({;})
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_FUNCALL)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                int i;
                Node *n = node->head;
                // dump();
                // printf("  sub rsp, %d\n", ((align) % 2) * 8); // align
                for (i = 0; i < 6 && n; i++, n = n->next2)
                {
                        gen(n); // result is in stack
                }
                i--;
                for (; i >= 0; i--)
                {
                        printf(pop(argreg[i])); // arg
                        // printf("  pop %s\n", argreg[i]);
                }
                printf("  mov eax, 0\n"); // set al to 0 for printf
                // int offset = ((align) % 2) * 8;
                // int offset = 8;
                int offset = 0;
                if (offset != 0)
                        printf("  sub rsp, %d\n", offset);
                // dump();
                printf("  call %s\n", node->token->str);
                if (offset != 0)
                        printf("  add rsp, %d\n", offset);
                printf(push("rax"));
                // printf("  push rax\n"); // save result to sp
                fprintf(tout2, "# </%s>\n", nodeK);
                return node->type;
        }
        else if (node->kind == ND_ADDR)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                //"&"
                gen_lval(node->lhs); // address is in stack
                fprintf(tout2, "# </%s>\n", nodeK);
                return node->type;
        }
        else if (node->kind == ND_DEREF)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                Type *t = gen(node->lhs); // address is in stack
                printf(pop("rdi"));       // move result to rax
                // printf("  pop rdi\n");
                if (node->type->kind == TY_CHAR)
                {
                        printf("  movsx rax, BYTE PTR [rdi]\n"); // get data from address
                }
                else if (node->type->kind == TY_INT)
                {
                        printf("  mov eax, DWORD PTR [rdi]\n"); // get data from address
                        printf("  cdqe\n");
                }
                else
                {
                        printf("  mov rax,[rdi]\n"); // get data from address
                }
                printf(push("rax"));
                // printf("  push rax\n"); // expression result */
                fprintf(tout2, "# </%s>\n", nodeK);
                return node->type;
        }
        // printf("  .loc 1 %d\n", node->token->loc);
        Type *t = gen(node->lhs);
        gen(node->rhs);
        printf(pop("rdi")); // move result to rax
        printf(pop("rax")); // move result to rax
        if (node->kind == ND_EXPR)
        {
                printf(push("rdi"));
                return node->type;
        }
        // printf("  pop rdi\n"); // rhs
        // printf("  pop rax\n"); // lhs
        if (!t)
        {
                error_at(node->token->pos, "no type");
        }
        fprintf(tout2, "# ty:%d\n", t->kind);
        if (t && (t->kind == TY_PTR || t->kind == TY_ARRAY))
        {
                fprintf(tout2, "# ptr_to->ty:%d\n", t->ptr_to->kind);
                if (t->ptr_to->kind == TY_CHAR)
                {
                        printf("  imul rdi, 1\n");
                }
                else if (t->ptr_to->kind == TY_INT)
                {
                        printf("  imul rdi, 4\n");
                }
                else
                {
                        printf("  imul rdi, 8\n");
                }
        }
        switch (node->kind)
        {
        case ND_ADD:
                printf("  add rax, rdi\n");
                break;
        case ND_SUB:
                printf("  sub rax, rdi\n");
                break;
        case ND_MOD:
                printf("  cqo\n");
                printf("  idiv rdi\n");
                printf("  mov rax, rdx\n");
                break;
        case ND_MUL:
                printf("  imul rax, rdi\n");
                break;
        case ND_DIV:
                printf("  cqo\n");
                printf("  idiv rdi\n");
                break;
        case ND_LT:
                printf("  cmp rax, rdi\n");
                printf("  setl al\n");
                printf("  movzb rax, al\n");
                break;
        case ND_LE:
                printf("  cmp rax, rdi\n");
                printf("  setle al\n");
                printf("  movzb rax, al\n");
                break;
        case ND_EQ:
                printf("  cmp rax, rdi\n");
                printf("  sete al\n");
                printf("  movzb rax, al\n");
                break;
        case ND_NE:
                printf("  cmp rax, rdi\n");
                printf("  setne al\n");
                printf("  movzb rax, al\n");
                break;
        }
        // printf("  push rax\n");
        printf(push("rax"));
        fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
        return node->type;
}
