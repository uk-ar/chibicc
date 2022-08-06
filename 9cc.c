#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

typedef enum {
       TK_RESERVED,//symbol
       TK_NUM,//int
       TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token{
       TokenKind kind;//type of token
       Token *next;//next input token
       int val;//token value if TK_NUM
       char *str;//token string
};

Token *token;//current token

void error(char *fmt,...){
       va_list ap;
       va_start(ap,fmt);
       vfprintf(stderr,fmt,ap);
       fprintf(stderr,"\n");
       exit(1);
}
bool consume(char op){//if next == op, advance & return true;
       if(!token || token->kind!=TK_RESERVED || token->str[0]!=op)
               return false;
       token=token->next;
       return true;
}
void expect(char op){//if next == op, advance
       if(!token || token->kind!=TK_RESERVED)
               error("token is not '%c'",op);
       token=token->next;
}
int expect_num(){//
       if(!token || token->kind!=TK_NUM)
               error("token is not number");
       int ans=token->val;
       token=token->next;
       return ans;
}
Token *new_token(TokenKind kind,Token *cur,char *str){
       Token*tok=calloc(1,sizeof(Token));
       tok->kind=kind;
       tok->str=str;//token->next=NULL;
       cur->next=tok;
       return tok;
}
bool at_eof(){
       return !token || token->kind==TK_EOF;
}
Token *tokenize(char *p){
       Token head,*cur=&head;
       while(*p){
               if(isspace(*p)){
                       p++;
                       continue;
               }
               if(*p=='+' || *p=='-'){
                       cur = new_token(TK_RESERVED,cur,p++);
                       continue;
               }
               if(isdigit(*p)){
                       cur = new_token(TK_NUM,cur,p);
                       cur->val=strtol(p,&p,10);
                       continue;
               }
               //printf("eee");
               error("can not tokenize");
       }
       cur = new_token(TK_EOF,cur,p);
       return head.next;
}

int main(int argc,char **argv){
    if(argc!=2){
        fprintf(stderr,"wrong number of argument\n.");
        return 1;
    }
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    char *p=argv[1];
    token=tokenize(p);
    printf("  mov rax, %d\n",expect_num());
    while(!at_eof()){
           if(consume('-')){
                   printf("  sub rax, %d\n",expect_num());
                   continue;
           }
           if(consume('+')){
                   printf("  add rax, %d\n",expect_num());
                   continue;
           }
           fprintf(stderr,"unexpected token\n.");
           exit(1);
    }
    printf("  ret\n");
    return 0;
}
