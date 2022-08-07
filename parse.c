#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include "9cc.h"

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
bool consume(char *op){//if next == op, advance & return true;
       if(!token || token->kind!=TK_RESERVED)
               return false;
       //printf("%s:%s:%d\n",op,token->str,token->len);
       if(strncmp(op,token->str,token->len)!=0)
               return false;
       token=token->next;
       return true;
}
void expect(char *op){//if next == op, advance
       if(!token || token->kind!=TK_RESERVED)
               error_at(token->str,"token is not '%s'",op);
       if(strncmp(op,token->str,token->len)!=0)
               error_at(token->str,"token is not '%s'",op);
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
Token *new_token(TokenKind kind,Token *cur,char *str,int len){
       Token*tok=calloc(1,sizeof(Token));
       tok->kind=kind;
       tok->str=str;//token->next=NULL;
       tok->len=len;
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
               if(!strncmp(p,"<=",2) || !strncmp(p,">=",2) ||
                  !strncmp(p,"==",2) || !strncmp(p,"!=",2)){
                       cur = new_token(TK_RESERVED,cur,p,2);
                       p+=2;
                       continue;
               }
               if(*p=='<' || *p=='>' || *p=='+' || *p=='-' || *p=='*' || *p=='/' || *p=='(' || *p==')'){
                       cur = new_token(TK_RESERVED,cur,p++,1);
                       continue;
               }
               if(isdigit(*p)){
                       char *pre=p;
                       cur = new_token(TK_NUM,cur,p,0);
                       cur->val=strtol(p,&p,10);
                       cur->len=p-pre;
                       //printf("%d",p-pre);
                       continue;
               }
               //printf("eee");
               error_at(p,"can not tokenize");
       }
       cur = new_token(TK_EOF,cur,p,0);
       return head.next;
}

char *nodeKind[]={
       "ND_ADD",
       "ND_SUB",
       "ND_MUL",
       "ND_DIV",
       "ND_NUM",
       "ND_LT",
       "ND_GT",
       "ND_EQ",
       "ND_NE",
       "ND_LE",
       "ND_GE",
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
/* expr       = equality */
/* equality   = relational ("==" relational | "!=" relational)* */
/* relational = add ("<" add | "<=" add | ">" add | ">=" add)* */
/* add        = mul ("+" mul | "-" mul)* */
/* mul     = unary ("*" unary | "/" unary)* */
/* unary   = ( "-" | "+" )? primary */
/* primary = num | "(" expr ")" */
Node *expr();
Node *primary(){
       if(consume("(")){
               Node*ans = expr();
               expect(")");//important
               return ans;
       }
       return  new_node_num(expect_num());
}
Node *unary(){
       if(consume("+")){
               return primary();
       }
       if(consume("-")){
               //important
               return new_node(ND_SUB,new_node_num(0),primary());//0-primary()
       }
       return primary();
}
Node *mul(){
       Node *node=unary();
       for(;;){
               if(consume("*")){
                       node=new_node(ND_MUL,node,unary());
               }
               if(consume("/")){
                       node=new_node(ND_DIV,node,unary());
               }
               return node;
       }
}
Node *add(){
       Node *node=mul();
       for(;;){
               if(consume("-")){
                       node=new_node(ND_SUB,node,mul());
                       continue;
               }
               if(consume("+")){
                       node=new_node(ND_ADD,node,mul());
                       continue;
               }
               return node;
       }
}
Node *relational(){
       Node *node=add();
       for(;;){
               if(consume("<=")){
                       node=new_node(ND_LE,node,add());
                       continue;
               }
               if(consume(">=")){
                       node=new_node(ND_LE,add(),node);//swap!
                       continue;
               }
               if(consume("<")){
                       node=new_node(ND_LT,node,add());
                       continue;
               }
               if(consume(">")){
                       node=new_node(ND_LT,add(),node);//swap!
                       continue;
               }
               return node;
       }
}
Node *equality(){
       Node *node=relational();
       for(;;){
               if(consume("==")){
                       node=new_node(ND_EQ,node,relational());
                       continue;
               }
               if(consume("!=")){
                       node=new_node(ND_NE,node,relational());
                       continue;
               }
               return node;
       }
}
Node *expr(){
       return equality();
}



