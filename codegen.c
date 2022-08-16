#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include "9cc.h"

FILE *tout2;
char *nodeKind[] = {
    "ND_STR",
    "ND_GVAR",
    "ND_DEREF",
    "ND_ADDR",
    "ND_FUNC",
    "ND_FUNCALL",
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

Type *gen_lval(Node *node)
{ // push address
        fprintf(tout2, "#lvar <%s>\n", nodeKind[node->kind]);
        if (node->kind == ND_GVAR)
        {
                printf("  lea rax, %s[rip]\n", node->name); // base pointer
                // printf("  lea rax, rip[%s]\n",strndup(node->token->pos,node->token->len));//base pointer
                // printf("  mov rax, %s\n",strndup(node->token->pos,node->token->len));//base pointer
                printf("  push rax\n"); // save local variable address
                /* fprintf(tout2,"#lvar </%s>\n",nodeKind[node->kind]); */
                return node->type;
        }
        else if (node->kind == ND_LVAR)
        {
                printf("  mov rax, rbp\n"); // base pointer
                printf("  sub rax, %d\n", node->offset);
                printf("  push rax\n"); // save local variable address
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
                error_at(node->token->pos, "token is not lvalue\n", node->token->pos);
                abort();
        }
}
int count()
{
        static int cnt = 0;
        return cnt++;
}
extern LVar *find_string(Token *tok);
static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
Type *gen(Node *node)
{
        char *nodeK = nodeKind[node->kind];
        fprintf(tout2, "# <%s>\n", nodeK);
        if (node->kind == ND_STR)
        {
                LVar *var = find_string(node->token);
                printf("  mov rax , OFFSET FLAT:.LC%d\n", var->offset);
                printf("  push rax\n");
                return node->type;
        }
        if (node->kind == ND_FUNC)
        {
                printf("%s:\n", node->name);
                printf("  push rbp\n");     // save base pointer
                printf("  mov rbp, rsp\n"); // save stack pointer
                for (int i = 0; i < 6 && node->params[i]; i++)
                {
                        printf("  mov rax, %s\n", argreg[i]); // args to local
                        printf("  push rax\n");               // args to local
                }
                printf("  sub rsp, %d\n", node->offset); // num of vals*8byte
                gen(node->then);
                return NULL;
        }
        if (node->kind == ND_NUM)
        {
                printf("  push %d\n", node->val);
                fprintf(tout2, "# %d</%s>\n", node->val, nodeKind[node->kind]);
                return node->type;
        }
        else if (node->kind == ND_LVAR || node->kind == ND_GVAR)
        {                                 // local value
                Type *t = gen_lval(node); // get address
                if (t->kind == TY_ARRAY)
                {
                        return t;
                }
                printf("  pop rax\n"); // get address
                if (t->kind == TY_CHAR)
                {
                        printf("  movsx eax, BYTE PTR [rax]\n"); // get data from address
                }
                else
                {
                        printf("  mov rax, [rax]\n"); // get data from address
                }
                printf("  push rax\n"); // save local variable value
                fprintf(tout2, "# </%s>\n", nodeK);
                return t;
        }
        else if (node->kind == ND_ASSIGN)
        {
                Type *t = gen_lval(node->lhs);
                gen(node->rhs);
                printf("  pop rbx\n"); // rhs
                printf("  pop rax\n"); // lhs
                if (t->kind == TY_CHAR || (t->kind == TY_ARRAY && t->ptr_to->kind == TY_CHAR))
                {
                        printf("  mov [rax],bl\n");
                }
                else
                {
                        printf("  mov [rax],rbx\n");
                }
                printf("  push [rax]\n"); // save expression result(ex. a=b=c)
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return t;
        }
        else if (node->kind == ND_RETURN)
        {
                gen(node->rhs);
                printf("  pop rax\n");     // move result to rax
                printf("  mov rsp,rbp\n"); // restore stack pointer
                printf("  pop rbp\n");     // restore base pointer
                printf("  ret\n");
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return NULL;
        }
        else if (node->kind == ND_IF)
        {
                fprintf(tout2, "# <cond>\n");
                gen(node->cond);
                fprintf(tout2, "# </cond>\n");
                printf("  pop rax\n"); // move result to rax
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
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_WHILE)
        {
                int num = count();
                printf(".Lbegin%d:\n", num);
                fprintf(tout2, "# <cond>\n");
                gen(node->cond);
                fprintf(tout2, "# </cond>\n");
                printf("  pop rax\n"); // move result to rax
                printf("  cmp rax, 0\n");
                printf("  je .Lend%d\n", num);
                fprintf(tout2, "# <then>\n");
                gen(node->then);
                fprintf(tout2, "# </then>\n");
                printf("  jmp .Lbegin%d\n", num);
                printf(".Lend%d:\n", num);
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_FOR)
        {
                int num = count();
                if (node->init)
                        gen(node->init);
                printf(".Lbegin%d:\n", num);
                fprintf(tout2, "# <cond>\n");
                if (node->cond)
                        gen(node->cond);
                fprintf(tout2, "# </cond>\n");
                printf("  pop rax\n"); // move result to rax
                printf("  cmp rax, 0\n");
                printf("  je .Lend%d\n", num);
                fprintf(tout2, "# <then>\n");
                gen(node->then);
                if (node->next)
                        gen(node->next);
                fprintf(tout2, "# </then>\n");
                printf("  jmp .Lbegin%d\n", num);
                printf(".Lend%d:\n", num);
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_BLOCK)
        {

                for (Node *c = node->head; c; c = c->next2)
                {
                        if (c != node->head)
                                printf("  pop rax\n"); // move result to remove
                        gen(c);
                }
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_FUNCALL)
        {
                int i;
                for (i = 0; i < 6 && node->params && node->params[i]; i++)
                {
                        gen(node->params[i]);
                }
                i--;
                for (; i >= 0; i--)
                {
                        // TODO: align rsp
                        printf("  pop %s\n", argreg[i]);
                }
                printf("  mov eax, 0\n"); // set al to 0 for printf
                printf("  call %s\n", node->name);
                printf("  push rax\n"); // save result to sp
                fprintf(tout2, "# </%s>\n", nodeK);
                return node->type;
        }
        else if (node->kind == ND_ADDR)
        {                            //"&"
                gen_lval(node->lhs); // address is in stack
                fprintf(tout2, "# </%s>\n", nodeK);
                return node->type;
        }
        else if (node->kind == ND_DEREF)
        {
                Type *t = gen(node->lhs); // address is in stack
                printf("  pop rdi\n");
                if (t->kind == TY_CHAR || ((t->kind == TY_ARRAY || t->kind == TY_PTR) && t->ptr_to->kind == TY_CHAR))
                {
                        printf("  movsx rax, BYTE PTR [rdi]\n"); // get data from address
                }
                else
                {
                        printf("  mov rax,[rdi]\n"); // get data from address
                }
                printf("  push rax\n"); // expression result */
                return t->ptr_to;
        }
        Type *t = gen(node->lhs);
        gen(node->rhs);
        printf("  pop rdi\n"); // rhs
        printf("  pop rax\n"); // lhs
        if (t)
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
        printf("  push rax\n");
        fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
        return t;
}
