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
int main(int argc,char **argv){
       tout=stdout;
    if(argc!=2){
        fprintf(stderr,"wrong number of argument\n.");
        return 1;
    }
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    char *p=argv[1];
    user_input=argv[1];
    token=tokenize(p);
    Node *root=expr();
    gen(root);

    printf("  pop rax\n");//lhs
    printf("  ret\n");
    return 0;
}
