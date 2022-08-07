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
extern LVar *locals,*globals;

int main(int argc,char **argv){
       tout=stdout;//debug
       //tout=stderr;
    if(argc!=2){
        fprintf(stderr,"wrong number of argument\n.");
        return 1;
    }
    char *p=argv[1];
    fprintf(tout,"# %s\n",p);
    locals=calloc(1,sizeof(LVar));
    user_input=argv[1];
    token=tokenize(p);
    program();

    //header
    printf(".intel_syntax noprefix\n");

    for(LVar *var=globals;var;var=var->next){//gvar
           //https://github.com/rui314/chibicc/commit/a4d3223a7215712b86076fad8aaf179d8f768b14
           printf(".data\n");
           printf(".global %s\n",var->name);
           printf("%s:\n",var->name);
           if(var->type->kind==TY_INT){
                   printf("  .zero 4\n");
           }else if(var->type->kind==TY_PTR){
                   printf("  .zero 8\n");
           }else{
                   printf("  .zero %d\n",var->type->array_size*4);
           }
    }

    printf(".global main\n");
    for(int i=0;code[i];i++){
           //rfprintf(stderr,"c0:%d\n",i);
            //fprintf(stderr,"c0:%d:%d\n",i,code[i]->kind);
            gen(code[i]);

            //pop each result in order not to over flow
            printf("  pop rax\n");
    }

    /* printf("  mov rsp,rbp\n");//restore stack pointer */
    /* printf("  pop rbp\n");//restore base pointer */
    /* printf("  ret\n"); */
    return 0;
}
