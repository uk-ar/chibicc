#include <stdio.h>
#include <stdlib.h>

int main(int argc,char **argv){
    if(argc!=2){
        fprintf(stderr,"wrong number of argument\n.");
        return 1;
    }
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    char *p=argv[1];
    printf("  mov rax, %d\n",strtol(p,&p,10));
    while(*p){
        if(*p=='-'){
            p++;
            printf("  sub rax, %d\n",strtol(p,&p,10));
            continue;
        }
        if(*p=='+'){
            p++;
            printf("  add rax, %d\n",strtol(p,&p,10));
            continue;
        }
        fprintf(stderr,"unexpected token\n.");
    }
    printf("  ret\n");
    return 0;
}
