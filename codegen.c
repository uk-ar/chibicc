#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include "9cc.h"

FILE *tout2;
char *nodeKind[] = {
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
                abort();
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
// static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
Type *gen(Node *node)
{
        char *nodeK = nodeKind[node->kind];
        fprintf(tout2, "# <%s>\n", nodeK);
        if (node->kind == ND_STR)
        {
                LVar *var = find_string(node->token);
                printf("  mov rax , OFFSET FLAT:.LC%d\n", var->offset);
                printf(push("rax"));
                // printf("  push rax\n");
                return node->type;
        }
        if (node->kind == ND_FUNC)
        {
                printf("%s:\n", node->token->str);
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
                // printf("  push %d\n", node->val);
                printf(push(format("%d", node->val)));
                fprintf(tout2, "# %d</%s>\n", node->val, nodeKind[node->kind]);
                return node->type;
        }
        else if (node->kind == ND_LVAR || node->kind == ND_GVAR)
        {                                 // local value
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
                Type *t = gen_lval(node->lhs);
                gen(node->rhs);
                printf(pop("rbx")); // rhs
                printf(pop("rax")); // lhs
                if (node->type->kind == TY_CHAR)
                {
                        printf("  mov [rax],bl\n");
                } // TODO:Add short type
                else if (node->type->kind == TY_INT)
                {
                        printf("  mov DWORD PTR [rax],ebx\n");
                }
                else
                { // todo fix for struct
                        printf("  mov [rax],rbx\n");
                }
                printf(push("[rax]"));
                // printf("  push [rax]\n"); // save expression result(ex. a=b=c)
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return t;
        }
        else if (node->kind == ND_RETURN)
        {
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
                fprintf(tout2, "# <cond>\n");
                gen(node->cond);
                fprintf(tout2, "# </cond>\n");
                printf(pop("rax")); // move result to rax
                // printf("  pop rax\n"); // move result to rax
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
        else if (node->kind == ND_WHILE)
        {
                int num = count();
                printf(".Lbegin%d:\n", num);
                fprintf(tout2, "# <cond>\n");
                gen(node->cond);
                fprintf(tout2, "# </cond>\n");
                printf(pop("rax")); // move result to rax
                // printf("  pop rax\n"); // move result to rax
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
                printf(pop("rax")); // move result to rax
                // printf("  pop rax\n"); // move result to rax
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
                        gen(c);
                        if (c->kind != ND_IF && c->kind != ND_BLOCK)
                                printf(pop("rax")); // move result to rax
                                                    // printf("  pop rax\n"); // move result to remove
                }
                // printf("  push 0\n"); // same behavior as ({;})
                fprintf(tout2, "# </%s>\n", nodeK);
                return NULL;
        }
        else if (node->kind == ND_EBLOCK)
        {

                for (Node *c = node->head; c; c = c->next2)
                {
                        gen(c);
                        if (c->next2 && c->kind != ND_BLOCK && c->kind != ND_IF)
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
        {                            //"&"
                gen_lval(node->lhs); // address is in stack
                fprintf(tout2, "# </%s>\n", nodeK);
                return node->type;
        }
        else if (node->kind == ND_DEREF)
        {
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
                }
                else
                {
                        printf("  mov rax,[rdi]\n"); // get data from address
                }
                printf(push("rax"));
                // printf("  push rax\n"); // expression result */
                fprintf(tout2, "# </%s>\n", nodeK);
                return t->ptr_to;
        }
        Type *t = gen(node->lhs);
        gen(node->rhs);
        printf(pop("rdi")); // move result to rax
        printf(pop("rax")); // move result to rax
        // printf("  pop rdi\n"); // rhs
        // printf("  pop rax\n"); // lhs
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
        return t;
}
