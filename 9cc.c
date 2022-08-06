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
       char *str;//token position
};

Token *token;//current token

char *user_input;
void error_at(char *loc,char *fmt,...){
       va_list ap;
       fprintf(stderr,"%s\n",user_input);
       int pos=loc-user_input;
       fprintf(stderr,"%*s",pos," ");//output white space
       fprintf(stderr,"^ ");//output white space
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
               error_at(token->str,"token is not '%c'",op);
       token=token->next;
}
int expect_num(){//
       if(!token || token->kind!=TK_NUM){
               error_at(token->str,"token is not number");
       }
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
               if(*p=='+' || *p=='-' || *p=='*' || *p=='/' || *p=='(' || *p==')'){
                       cur = new_token(TK_RESERVED,cur,p++);
                       continue;
               }
               if(isdigit(*p)){
                       cur = new_token(TK_NUM,cur,p);
                       cur->val=strtol(p,&p,10);
                       continue;
               }
               //printf("eee");
               error_at(p,"can not tokenize");
       }
       cur = new_token(TK_EOF,cur,p);
       return head.next;
}

typedef enum {
       ND_ADD,
       ND_SUB,
       ND_MUL,
       ND_DIV,
       ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node{//binary tree node
       NodeKind kind;
       Node *lhs;//left hand side;
       Node *rhs;//left hand side;
       int val; // enable iff kind == ND_NUM
};
Node *new_node(NodeKind kind,Node *lhs,Node *rhs){
       Node *ans=calloc(1,sizeof(Node));
       ans->kind=kind;
       ans->lhs=lhs;
       ans->rhs=rhs;
       return ans;
}
Node *new_node_num(int val){
       //leaf node
       Node *ans=calloc(1,sizeof(Node));
       ans->kind=ND_NUM;
       ans->val=val;
       return ans;
}
/* expr    = mul ("+" mul | "-" mul)* */
/* mul     = unary ("*" unary | "/" unary)* */
/* unary   = ( "-" | "+" )? primary */
/* primary = num | "(" expr ")" */
Node *expr();
Node *primary(){
       if(consume('(')){
               Node*ans = expr();
               expect(')');//important
               return ans;
       }
       return  new_node_num(expect_num());
}
Node *unary(){
       if(consume('+')){
               return primary();
       }
       if(consume('-')){
               //important
               return new_node(ND_SUB,new_node_num(0),primary());//0-primary()
       }
       return primary();
}
Node *mul(){
       Node *node=unary();
       for(;;){
               if(consume('*')){
                       node=new_node(ND_MUL,node,unary());
               }
               if(consume('/')){
                       node=new_node(ND_DIV,node,unary());
               }
               return node;
       }
}
Node *expr(){
       Node *node=mul();
       for(;;){
               if(consume('-')){
                       node=new_node(ND_SUB,node,mul());
                       continue;
               }
               if(consume('+')){
                       node=new_node(ND_ADD,node,mul());
                       continue;
               }
               return node;
       }
}
void gen(Node *node){
       if(node->kind==ND_NUM){
               printf("  push %d\n",node->val);
               return;
       }

       gen(node->lhs);
       gen(node->rhs);

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
       }
       printf("  push rax\n");
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
    user_input=argv[1];
    token=tokenize(p);
    Node *root=expr();
    gen(root);

    printf("  pop rax\n");//lhs
    printf("  ret\n");
    return 0;
}
