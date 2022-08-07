#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include "9cc.h"

FILE *tout;

void gen(Node *node){
       //printf("%d:%d\n",node->kind,node->val);//debug
       if(node->kind==ND_NUM){
               //fprintf(tout,"<%s>%d</%s>\n",nodeKind[node->kind],node->val,nodeKind[node->kind]);
               printf("  push %d\n",node->val);
               return;
       }
       //fprintf(tout,"<%s>\n",nodeKind[node->kind]);
       gen(node->lhs);
       gen(node->rhs);
       //fprintf(tout,"</%s>\n",nodeKind[node->kind]);
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
               printf("  cgo\n");
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
