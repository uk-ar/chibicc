
struct _IO_FILE;
typedef struct _IO_FILE FILE;
int fprintf(FILE *__restrict __stream, const char *__restrict __fmt, ...);

//#include <stdlib.h>
typedef long unsigned int size_t;
//#include <stdio.h>
extern int printf(const char *__restrict __fmt, ...);
// extern int printf(const char * __fmt, ...);
//#include <string.h>
extern int strcmp(const char *__s1, const char *__s2);
//#include <stddef.h>
#define NULL 0 // TODO:((void *)0)

#include "9cc.h"
HashMap *labels;
/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

https://freak-da.hatenablog.com/entry/2021/03/25/172248
System V AMD64 ABI
関数呼び出し
整数 or ポインタ RDI, RSI, RDX, RCX, R8, R9
不動小数点 XMM0, XMM1, XMM2, XMM3, XMM4, XMM5, XMM6 and XMM7
戻り値
整数64ビットまでRAX
整数128ビットまでRAX & RDX
不動小数点 XMM0 and XMM1
caller/callee-save
RBX, RSP, RBP, and R12–R15 : non-volatile (= callee save)
RAX, RCX, RDX, RSI, RDI, R8-R11 XMM0-XMM15, YMM0-YMM15, ZMM0-ZMM31: volatile (= caller save)
システムコールでは RCX の代わりに R10 を使用
*/

//*/
//#include <assert.h>
void abort(void);
Type *gen_expr(Node *node);
FILE *tout2;
char *nodeKind[] = {
    "ND_MEMBER",
    "ND_NOP",
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
extern HashMap *strings;
int align = 0;
int lines[100];
int _push(char *reg, int loc)
{
        lines[align] = loc;
        align++;
        return printf("  push %s\n", reg);
}
#define push(a) _push(a, __LINE__)
int pop(char *reg)
{
        align--;
        return printf("  pop %s\n", reg);
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
                push("rax");
                /* fprintf(tout2,"#lvar </%s>\n",nodeKind[node->kind]); */
                return node->type;
        }
        else if (node->kind == ND_LVAR)
        {
                printf("  mov rax, rbp\n"); // base pointer
                printf("  sub rax, %d\n", node->offset);
                // printf("  push rax\n"); // save local variable address
                push("rax");
                // push(format("[rax-%d]",node->offset));
                fprintf(tout2, "#lvar </%s>\n", nodeKind[node->kind]);
                return node->type;
        }
        else if (node->kind == ND_DEREF)
        {
                Type *t = gen_expr(node->lhs); // address is in stack
                fprintf(tout2, "#lvar </%s>\n", nodeKind[node->kind]);
                return t;
        }
        else if (node->kind == ND_MEMBER)
        {
                Type *t = gen_expr(node->lhs); // address is in stack
                if (t->kind != TY_STRUCT)
                        abort();
                pop("rax");
                printf("  add rax, %ld\n", node->member->offset);
                // printf("  add rax, %d\n", node->offset);
                push("rax");
                fprintf(tout2, "#lvar </%s>\n", nodeKind[node->kind]);
                return node->member->type;
        }
        else
        {
                error_tok(node->token, "token is not lvalue\n", node->token->str);
                return NULL;
        }
}
int cnt = 1;
int count()
{
        // static int cnt = 1;
        return cnt++;
}
void dump()
{
        printf("  mov rsi, rsp\n");
        printf("  mov edi, OFFSET FLAT:.LCdebug\n");
        printf("  mov eax, 0\n");
        printf("  call printf\n");
}
char *break_label = NULL;
char *continue_label = NULL;
// static char *argreg[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
static char *argreg[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
Type *gen_stmt(Node *node)
{
        char *nodeK = nodeKind[node->kind];
        int pre = align;
        Type *t = NULL;
        if (node->kind == ND_NOP)
        {
        }
        else if (node->kind == ND_RETURN)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                // printf("  .loc 1 %d\n", node->token->loc);
                if (node->lhs)
                {
                        gen_expr(node->lhs);
                        pop("rax"); // move result to rax
                }
                printf("  leave\n"); // restore stack pointer
                // printf("  mov rsp,rbp\n"); // restore stack pointer
                // printf("  pop rbp\n");     // restore base pointer
                printf("  ret\n");
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
        }
        else if (node->kind == ND_IF)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                // int pre = align;
                // printf("  .loc 1 %d\n", node->token->loc);
                fprintf(tout2, "# <cond>\n");
                gen_expr(node->cond);
                fprintf(tout2, "# </cond>\n");
                pop("rax"); // move result to rax

                printf("  cmp rax, 0\n");
                int num = count();
                printf("  je .Lelse%d\n", num);
                fprintf(tout2, "# <then>\n");
                gen_stmt(node->then);
                fprintf(tout2, "# </then>\n");
                printf("  jmp .Lend%d\n", num);

                printf(".Lelse%d:\n", num);
                if (node->els)
                {
                        gen_stmt(node->els);
                }
                printf(".Lend%d:\n", num);
                if (!add_hash(labels, format(".Lend%d:\n", num), (void *)1))
                        abort();
                // printf("  push 0\n", num);//
                fprintf(tout2, "# </%s>\n", nodeK);
        }
        else if (node->kind == ND_SWITCH)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                // int pre = align;
                //  printf("  .loc 1 %d\n", node->token->loc);
                int num = count();
                gen_expr(node->cond);
                pop("rax"); // move result to rax
                for (HashNode *c = node->cases->begin; c; c = c->next)
                {
                        printf("  cmp rax, %ld\n", (long)c->value);
                        printf("  je .Lcase%s\n", c->key);
                }
                if (node->els)
                {
                        printf("  jmp .Ldefault%ld\n", node->els->val);
                }
                char *parent_break_label = break_label;
                break_label = format(".Lend%d", num);

                fprintf(tout2, "# <then>\n");
                gen_stmt(node->then); // block
                fprintf(tout2, "# </then>\n");

                /*if (!add_hash(labels, format("%s:\n", break_labels[depth]), 1))
                        abort();*/

                printf("%s:\n", break_label); // for break;
                break_label = parent_break_label;

                fprintf(tout2, "# </%s>\n", nodeK);
        }
        else if (node->kind == ND_BREAK)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                if (!break_label)
                        error_tok(node->token, "no break point");
                printf("  jmp %s\n", break_label);
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
        }
        else if (node->kind == ND_CONTINUE)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                if (!continue_label)
                        error_tok(node->token, "no continue point");
                printf("  jmp %s\n", continue_label);
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
        }
        else if (node->kind == ND_CASE)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                if (node->lhs)
                        printf(".Lcase%ld:\n", node->val);
                else
                        printf(".Ldefault%ld:\n", node->val);
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
        }
        else if (node->kind == ND_FOR)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                // int pre = align;
                //  printf("  .loc 1 %d\n", node->token->loc);
                int num = count();
                char *parent_continue_label = continue_label;
                continue_label = format(".Lnext%d", num);
                char *parent_break_label = break_label;
                break_label = format(".Lend%d", num);
                char *cond_label = format(".Lbegin%d", num);

                if (node->init)
                {
                        fprintf(tout2, "# <init>\n");
                        gen_stmt(node->init); // block
                        // pop("rax"); // move result to rax
                        fprintf(tout2, "# <init>\n");
                }

                printf("%s:\n", cond_label);
                if (node->cond)
                {
                        fprintf(tout2, "# <cond>\n");
                        gen_expr(node->cond);
                        pop("rax"); // move result to rax
                        printf("  cmp rax, 0\n");
                        printf("  je %s\n", break_label);
                        fprintf(tout2, "# </cond>\n");
                }

                // printf("  pop rax\n"); // move result to rax
                if (node->then)
                {
                        fprintf(tout2, "# <then>\n");
                        gen_stmt(node->then); // block
                        fprintf(tout2, "# </then>\n");
                }

                printf("%s:\n", continue_label);
                if (node->inc)
                {
                        fprintf(tout2, "# <next>\n");
                        gen_expr(node->inc);
                        pop("rax"); // move result to rax
                        fprintf(tout2, "# </next>\n");
                }

                printf("  jmp %s\n", cond_label);
                printf("%s:\n", break_label);
                break_label = parent_break_label;
                continue_label = parent_continue_label;
                // if (!add_hash(labels, format("%s:\n", break_labels[depth]), 1))
                //         abort();
                fprintf(tout2, "# </%s>\n", nodeK);
        }
        else if (node->kind == ND_BLOCK)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                // printf("  .loc 1 %d\n", node->token->loc);
                for (Node *c = node->head; c; c = c->next)
                {
                        gen_stmt(c);
                }
                //  printf("  push 0\n"); // same behavior as ({;})
                fprintf(tout2, "# </%s>\n", nodeK);
        }
        else if (node->kind == ND_WHILE)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                // int pre = align;
                //  printf("  .loc 1 %d\n", node->token->loc);
                int num = count();
                char *parent_continue_label = continue_label;
                continue_label = format(".Lnext%d", num);
                char *parent_break_label = break_label;
                break_label = format(".Lend%d", num);
                // char *cond_label = format(".Lbegin%d", num);

                printf("%s:\n", continue_label);
                fprintf(tout2, "# <cond>\n");
                gen_expr(node->cond);
                fprintf(tout2, "# </cond>\n");
                pop("rax"); // move result to rax
                printf("  cmp rax, 0\n");
                printf("  je %s\n", break_label);

                fprintf(tout2, "# <then>\n");
                gen_stmt(node->then); // block
                fprintf(tout2, "# </then>\n");

                printf("  jmp %s\n", continue_label);
                printf("%s:\n", break_label);

                continue_label = parent_continue_label;
                break_label = parent_break_label;
                fprintf(tout2, "# </%s>\n", nodeK);
        }
        else
        {
                t = gen_expr(node);
                pop("rax");
        }
        if (align != pre)
                error_tok(node->token, "stack position is wrong");
        return t;
}
Type *gen_expr(Node *node)
{
        char *nodeK = nodeKind[node->kind];
        fprintf(tout2, "# <%s>\n", nodeK);
        if (node->kind == ND_STR)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                printf("  mov rax , OFFSET FLAT:.LC%d\n", node->offset);
                push("rax");
                // printf("  push rax\n");
                return node->type;
        }
        if (node->kind == ND_NUM)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                //  printf("  push %d\n", node->val);
                push(format("%d", node->val));
                fprintf(tout2, "# %ld</%s>\n", node->val, nodeKind[node->kind]);
                return node->type;
        }
        else if (node->kind == ND_LVAR || node->kind == ND_GVAR || node->kind == ND_MEMBER)
        { // local value
                // printf("  .loc 1 %d\n", node->token->loc);
                Type *t = gen_lval(node); // get address
                if (t->kind == TY_ARRAY || t->kind == TY_STRUCT)
                {
                        return t;
                }
                pop("rdi"); // get address
                // printf("  pop rdi\n"); // get address
                if (t->kind == TY_CHAR || t->kind == TY_BOOL)
                {
                        printf("  movsx eax, BYTE PTR [rdi]\n"); // get data from address
                }
                else if (t->kind == TY_INT)
                {
                        printf("  mov eax, DWORD PTR [rdi]\n"); // get data from address
                        printf("  cdqe\n");
                }
                else
                {
                        printf("  mov rax, QWORD PTR [rdi]\n"); // get data from address
                }
                push("rax");
                // printf("  push rax\n"); // save local variable value
                fprintf(tout2, "# </%s>\n", nodeK);
                return t;
        }
        else if (node->kind == ND_ASSIGN)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                Type *t = gen_lval(node->lhs);
                gen_expr(node->rhs);
                pop("rax"); // rhs
                pop("rdi"); // lhs
                if (node->type->kind == TY_BOOL)
                {
                        printf("  cmp al, 0\n");
                        printf("  setne al\n");
                        printf("  mov BYTE PTR [rdi],al\n");
                }
                else if (node->type->kind == TY_CHAR)
                {
                        printf("  mov BYTE PTR [rdi],al\n");
                } // TODO:Add short type
                else if (node->type->kind == TY_INT)
                {
                        printf("  mov DWORD PTR [rdi],eax\n");
                        printf("  cdqe\n");
                }
                else
                { // todo fix for struct
                        printf("  mov [rdi],rax\n");
                }
                push("rax");
                // printf("  push [rax]\n"); // save expression result(ex. a=b=c)
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return t;
        }
        else if (node->kind == ND_CAST)
        {
                // TODO:merge ND_ASSIGN
                Type *t = gen_expr(node->lhs); // value is in stack
                if (t->kind == TY_ARRAY || t->kind == TY_STRUCT)
                        return node->type;
                // abort();//TODO:FIXME
                if (t->size == node->type->size)
                        return node->type;
                pop("rax");
                if (node->type->kind == TY_BOOL)
                {
                        printf("  cmp al, 0\n");
                        printf("  setne al\n");
                        printf("  movzx eax, al\n");
                }
                else if (node->type->kind == TY_CHAR)
                {
                        printf("  movsx rax, al\n");
                } // TODO:Add short type
                // printf("  movsx rax, ax\n");
                else if (node->type->kind == TY_INT)
                {
                        printf("  cdqe\n");
                }
                else
                {
                        // nop
                }
                push("rax");
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return node->type;
        }
        else if (node->kind == ND_COND)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                fprintf(tout2, "# <cond>\n");
                gen_expr(node->cond);
                fprintf(tout2, "# </cond>\n");
                pop("rax"); // move result to rax
                printf("  cmp rax, 0\n");
                int num = count();

                printf("  je .Lelse%d\n", num);
                fprintf(tout2, "# <then>\n");
                gen_expr(node->then);
                pop("rax"); // move result to rax
                fprintf(tout2, "# </then>\n");
                printf("  jmp .Lend%d\n", num);

                printf(".Lelse%d:\n", num);
                Type *t = gen_expr(node->els);
                pop("rax"); // move result to rax

                printf(".Lend%d:\n", num);
                if (!add_hash(labels, format(".Lend%d:\n", num), (void *)1))
                        abort();
                // printf("  push 0\n", num);//
                fprintf(tout2, "# </%s>\n", nodeK);
                push("rax"); // push result
                return t;
        }
        else if (node->kind == ND_AND)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                gen_expr(node->lhs);
                pop("rax"); // move result to rax

                int num = count();
                printf("  cmp rax, 0\n");
                printf("  je .Lend%d\n", num);

                Type *t = gen_expr(node->rhs); // result is in stack
                pop("rax");                    // move result to rax

                if (!add_hash(labels, format(".Lend%d:\n", num), (void *)1))
                        abort();
                printf(".Lend%d:\n", num);
                push("rax");
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return t;
        }
        else if (node->kind == ND_OR)
        {
                fprintf(tout2, "# <%s>\n", nodeKind[node->kind]);
                gen_expr(node->lhs);
                pop("rax"); // move result to rax

                int num = count();
                printf("  cmp rax, 0\n");
                printf("  jne .Lend%d\n", num);

                Type *t = gen_expr(node->rhs); // result is in stack
                pop("rax");                    // move result to rax

                if (!add_hash(labels, format(".Lend%d:\n", num), (void *)1))
                        abort();
                printf(".Lend%d:\n", num);
                push("rax");
                fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
                return t;
        }
        else if (node->kind == ND_EBLOCK)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                Type *t;
                for (Node *c = node->head; c; c = c->next)
                {
                        t = gen_stmt(c);
                }
                push("rax");
                // printf("  push 0\n"); // same behavior as ({;})
                fprintf(tout2, "# </%s>\n", nodeK);
                return t;
        }
        else if (node->kind == ND_FUNCALL)
        {
                // printf("  .loc 1 %d\n", node->token->loc);
                int i;
                Node *n = node->head;
                // dump();
                // printf("  sub rsp, %d\n", ((align) % 2) * 8); // align
                for (i = 0; n && i < 6; i++, n = n->next)
                {
                        gen_expr(n); // result is in stack
                }
                i--;
                for (; i >= 0; i--)
                {
                        pop(argreg[i]); // arg
                        // printf("  pop %s\n", argreg[i]);
                }
                printf("  mov eax, 0\n"); // set al to 0 for printf
                int offset = ((align) % 2) * 8;
                // int offset = 8;
                // int offset = 0;
                if (offset != 0)
                        printf("  sub rsp, %d\n", offset);
                // dump();
                printf("  call %s\n", node->token->str);
                if (offset != 0)
                        printf("  add rsp, %d\n", offset);
                push("rax");
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
                gen_expr(node->lhs); // address is in stack
                pop("rdi");          // move result to rax
                // printf("  pop rdi\n");
                if (node->type->kind == TY_STRUCT || node->type->kind == TY_STRUCT)
                {
                        printf("  mov rax, rdi\n"); // get data from address
                }
                else if (node->type->kind == TY_BOOL)
                {
                        printf("  movsx rax, BYTE PTR [rdi]\n"); // get data from address
                }
                else if (node->type->kind == TY_CHAR)
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
                push("rax");
                // printf("  push rax\n"); // expression result */
                fprintf(tout2, "# </%s>\n", nodeK);
                return node->type;
        }
        // printf("  .loc 1 %d\n", node->token->loc);
        Type *t = gen_expr(node->lhs);
        gen_expr(node->rhs);
        pop("rdi");                // move result to rax
        pop("rax");                // move result to rax
        if (node->kind == ND_EXPR) // comma
        {
                push("rax"); // return left
                return node->type;
        }
        if (!t)
        {
                error_tok(node->token, "no type");
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
        default:
                break;
                // nop;
        }
        // printf("  push rax\n");
        push("rax");
        fprintf(tout2, "# </%s>\n", nodeKind[node->kind]);
        return node->type;
}

void function(Obj *func)
{
        printf("  .text \n");
        printf("  .global %s\n", func->name);
        printf("  .type %s, @function\n", func->name);
        printf("%s:\n", func->name);
        // printf("  .loc 1 %d\n", node->token->loc);
        // dump();

        printf("  push rbp\n");                                      // save base pointer
        printf("  mov rbp, rsp\n");                                  // save stack pointer
        printf("  sub rsp, %d\n", (func->stacksize + 15) / 16 * 16); // num of vals*8byte
        Obj *n = func->params;
        for (int i = 0; n && n->type && i < 6; i++, n = n->next)
        {
                // printf("  mov rax, %s\n", argreg[i]); // args to local
                // printf("  push rax\n");               // args to local
                printf("  mov rdi, %s\n", argreg[i]); // args to local
                if (n->type->kind == TY_BOOL)
                {
                        printf("  mov BYTE PTR [rbp-%ld], dil\n", n->offset); // get data from address
                }
                else if (n->type->kind == TY_CHAR)
                {
                        printf("  mov BYTE PTR [rbp-%ld], dil\n", n->offset); // get data from address
                }
                else if (n->type->kind == TY_INT)
                {
                        printf("  mov DWORD PTR [rbp-%ld], edi\n", n->offset); // get data from address
                }
                else
                {
                        printf("  mov QWORD PTR [rbp-%ld], rdi\n", n->offset); // get data from address
                        // printf("  mov rax, rdi\n"); // get data from address
                }
                // printf("  push rax\n"); // args to local
        }
        // dump();
        gen_stmt(func->body);
}
// char *global_types[] = {".byte", ".long", ".quad", ".quad"};//
// char *global_types[] = {".byte", ".long", ".quad", ".quad", ".byte"};
char *global_types[] = {".byte", ".byte", ".long", ".quad", ".quad"};
//  TY_BOOL,TY_CHAR
void codegen(Obj *code, char *filename)
{
        // header
        printf(".file \"%s\"\n", filename);
        // printf(".file 1 \"%s\"\n", filename); unable to debug tms.s
        printf(".intel_syntax noprefix\n");

        for (HashNode *var = strings->begin; var; var = var->next)
        {
                printf("  .text \n");
                // printf("  .section      .rodata \n");
                printf(".LC%ld:\n", var->value);
                printf("  .string %s\n", var->key);
        }
        // for debug
        printf(".LCdebug:\n");
        printf("  .string \"%s\"\n", "rsp:%p\\n");
        for (Obj *var = code; var; var = var->next)
        { // gvar
                if (var->is_function)
                        continue;
                // https://github.com/rui314/chibicc/commit/a4d3223a7215712b86076fad8aaf179d8f768b14
                printf(".data\n");
                printf(".global %s\n", var->name);

                long align = (var->type->kind == TY_ARRAY && var->type->size >= 16) ? MAX(16, var->type->align) : var->type->align;
                // common symbol
                if (!var->init)
                {
                        printf("  .comm %s, %ld, %ld\n", var->name, var->type->size, align);
                        continue;
                }

                printf("  .align %ld\n", var->type->align);
                printf("%s:\n", var->name);
                list *p = var->init;
                if (!p)
                {
                        printf("  .zero %ld\n", var->type->size);
                }
                else if (p->size == 1) // TODO:clean up
                {
                        if (var->type->kind == TY_ARRAY)
                        {
                                printf("  .string %s\n", (char *)p->head->value);
                        }
                        else
                        {
                                printf("  %s %s\n", global_types[var->type->kind], (char *)p->head->value);
                        }
                }
                else
                {
                        for (listnode *n = p->head; n; n = n->next)
                        {
                                if (var->type->ptr_to->kind == TY_ARRAY)
                                {
                                        printf("  .string %s\n", (char *)n->value);
                                }
                                else
                                {
                                        printf("  %s %s\n", global_types[var->type->ptr_to->kind], (char *)n->value);
                                }
                        }
                }
        }
        for (Obj *var = code; var; var = var->next)
        { // gvar
                if (!var->is_function || !var->body)
                        continue;
                function(var);
        }
}