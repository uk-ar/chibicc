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
       //printf("t:%s:%d\n",token->str,token->len);
       if(!token || token->kind!=TK_RESERVED)
               return false;
       if(strncmp(op,token->str,strlen(op))!=0)
               return false;
       //printf("t:%s:%d\n",token->str,token->len);
       token=token->next;
       return true;
}

Token *consume_ident(){//if next == op, advance & return true;
       if(!token || token->kind!=TK_IDENT)
               return false;
       //printf("t:%s:%d\n",token->str,token->len);
       Token*ans=token;
       token=token->next;
       return ans;
}
void expect(char *op){//if next == op, advance
       if(!token || token->kind!=TK_RESERVED)
               error_at(token->str,"token is not '%s'",op);
       if(strncmp(op,token->str,strlen(op))!=0)
               error_at(token->str,"token is not '%s'",op);
       //printf("t:%s:%d\n",token->str,token->len);
       token=token->next;
}
int expect_num(){//
       if(!token || token->kind!=TK_NUM){
               error_at(token->str,"token is not number");
       }
       int ans=token->val;
       //printf("t:%s:%d\n",token->str,token->len);
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
               if(*p=='<' || *p=='>' || *p=='+' || *p=='-' || *p=='*' || *p=='/' || *p=='(' || *p==')' || *p=='=' || *p==';'){
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
               if(isalpha(*p)){
                       cur = new_token(TK_IDENT,cur,p++,1);
                       continue;
               }
               //printf("eee");
               error_at(p," can not tokenize '%c'",*p);
       }
       cur = new_token(TK_EOF,cur,p,0);
       return head.next;
}

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
/* program    = stmt* */
/* stmt       = expr ";" */
/* expr       = assign */
/* assign     = equality ("=" assign)? */
/* equality   = relational ("==" relational | "!=" relational)* */
/* relational = add ("<" add | "<=" add | ">" add | ">=" add)* */
/* add        = mul ("+" mul | "-" mul)* */
/* mul     = unary ("*" unary | "/" unary)* */
/* unary   = ( "-" | "+" )? primary */
/* primary = num | ident | "(" expr ")" */
Node *expr();
Node *primary(){
       fprintf(tout,"<%s>\n",__func__);
       if(consume("(")){
               Node*ans = expr();
               expect(")");//important
               fprintf(tout,"</%s>\n",__func__);
               return ans;
       }
       Token* tok=consume_ident();
       if(tok){
               Node*ans = new_node(ND_LVAR,NULL,NULL);
               ans->offset=(tok->str[0]-'a'+1)*8;
               fprintf(tout,"</%s>\n",__func__);
               return ans;
       }
       fprintf(tout,"</%s>\n",__func__);
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
       fprintf(tout,"<%s>\n",__func__);
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
               fprintf(tout,"</%s>\n",__func__);
               return node;
       }
}
Node *equality(){
       fprintf(tout,"<%s>\n",__func__);
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
               fprintf(tout,"</%s>\n",__func__);
               return node;
       }
}

Node *assign(){
       fprintf(tout,"<%s>\n",__func__);
       Node *node=equality();
       if(consume("=")){
               node=new_node(ND_ASSIGN,node,assign());
               fprintf(tout,"</%s>\n",__func__);
               return node;
       }
       fprintf(tout,"</%s>\n",__func__);
       return node;
}
Node *expr(){
       fprintf(tout,"<%s>\n",__func__);
       Node*node=assign();
       fprintf(tout,"</%s>\n",__func__);
       return node;
}
Node *stmt(){
       Node *node=expr();
       expect(";");
       return node;
}
Node *code[100]={0};
void program(){
       int i=0;
       while(!at_eof()){
               fprintf(tout,"c:%d\n",i);
               //fprintf(tout,"c:%d:%d\n",i,code[i]->kind);
               code[i++]=stmt();
       }
       code[i]=NULL;
}
