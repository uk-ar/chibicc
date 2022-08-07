#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include "9cc.h"

FILE *tout;
char *nodeKind[]={
       "ND_ADD",
       "ND_SUB",
       "ND_MUL",
       "ND_DIV",
       "ND_NUM",
       "ND_LVAR",//variable
       "ND_ASSIGN",//variable
       "ND_LT",
       "ND_GT",
       "ND_EQ",
       "ND_NE",
       "ND_LE",
       "ND_GE",
};

void gen_lval(Node *node){
       if(node->kind!=ND_LVAR){
               printf("node not lvalue");
               abort();
       }
       printf("  mov rax, rbp\n");//base pointer
       printf("  sub rax, %d\n",node->offset);
       printf("  push rax\n");//save local variable address
}

void gen(Node *node){
       fprintf(tout,"g<%s>\n",nodeKind[node->kind]);
       if(node->kind==ND_NUM){
               printf("  push %d\n",node->val);
               fprintf(tout,"%d</%s>\n",node->val,nodeKind[node->kind]);
               return;
       }else if(node->kind==ND_LVAR){
               gen_lval(node);
               printf("  pop rax\n");//get address
               printf("  mov rax, [rax]\n");//get data from address
               printf("  push rax\n");//save local variable value
               fprintf(tout,"g</%s>\n",nodeKind[node->kind]);
               return;
       }else if(node->kind==ND_ASSIGN){
               gen_lval(node->lhs);
               gen(node->rhs);
               printf("  pop rdi\n");//rhs
               printf("  pop rax\n");//lhs
               printf("  mov [rax],rdi\n");
               printf("  push rdi\n");//expression result
               fprintf(tout,"g</%s>\n",nodeKind[node->kind]);
               return;
       }
       gen(node->lhs);
       gen(node->rhs);
       fprintf(tout,"g</%s>\n",nodeKind[node->kind]);
       printf("  pop rdi\n");//rhs
       printf("  pop rax\n");//lhs

              switch(node->kind){
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
}
