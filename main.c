#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include "9cc.h"

extern FILE *tout;
extern char* user_input;
extern Token* token;
extern Node *code[];
extern LVar *locals;
int main(int argc,char **argv){
       //tout=stdout;
       tout=stderr;
    if(argc!=2){
        fprintf(stderr,"wrong number of argument\n.");
        return 1;
    }
    char *p=argv[1];
    locals=calloc(1,sizeof(LVar));
    user_input=argv[1];
    token=tokenize(p);
    program();

    //header
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    //prepare variables
    printf("  push rbp\n");//save base pointer
    printf("  mov rbp, rsp\n");//save stack pointer
    printf("  sub rsp, %d\n",locals->offset);//num of vals*8byte

    for(int i=0;code[i];i++){
           //rfprintf(stderr,"c0:%d\n",i);
            //fprintf(stderr,"c0:%d:%d\n",i,code[i]->kind);
            gen(code[i]);

            //pop each result in order not to over flow
            printf("  pop rax\n");
    }

    printf("  mov rsp,rbp\n");//restore stack pointer
    printf("  pop rbp\n");//restore base pointer
    printf("  ret\n");
    return 0;
}
