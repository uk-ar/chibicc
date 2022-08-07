#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>
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

Token *consume_Token(TokenKind kind){
       if(!token || token->kind!=kind)
               return NULL;
       Token*ans=token;
       token=token->next;
       return ans;
}

Token *consume(char *op){//if next == op, advance & return true;
       if(strncmp(op,token->str,strlen(op))!=0)
               return NULL;
       //printf("t:%s:%d\n",token->str,token->len);
       Token*ans=token;
       token=token->next;
       return ans;
}

Token *consume_ident(){//if next == op, advance & return true;
       return consume_Token(TK_IDENT);
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
bool isIdent(char c){
       return isdigit(c) || isalpha(c);
}
Token *tokenize(char *p){
       Token head,*cur=&head;
       while(*p){
               fprintf(tout,"# t:%c\n",*p);
               if(isspace(*p)){
                       p++;
                       continue;
               }
               if(!strncmp(p,"int",3) && !isIdent(p[3])){
                       cur = new_token(TK_TYPE,cur,p,3);
                       p+=3;
                       continue;
               }
               if(!strncmp(p,"return",6) && !isIdent(p[6])){
                       cur = new_token(TK_RETURN,cur,p,6);
                       p+=6;
                       continue;
               }
               if(!strncmp(p,"if",2) && !isIdent(p[2])){
                       cur = new_token(TK_IF,cur,p,2);
                       p+=2;
                       continue;
               }
               if(!strncmp(p,"else",4) && !isIdent(p[4])){
                       cur = new_token(TK_ELSE,cur,p,4);
                       p+=4;
                       continue;
               }
               if(!strncmp(p,"while",5) && !isIdent(p[5])){
                       cur = new_token(TK_WHILE,cur,p,5);
                       p+=5;
                       continue;
               }
               if(!strncmp(p,"for",3) && !isIdent(p[3])){
                       cur = new_token(TK_FOR,cur,p,3);
                       p+=3;
                       continue;
               }
               if(!strncmp(p,"<=",2) || !strncmp(p,">=",2) ||
                  !strncmp(p,"==",2) || !strncmp(p,"!=",2)){
                       cur = new_token(TK_RESERVED,cur,p,2);
                       p+=2;
                       continue;
               }
               if(*p=='<' || *p=='>' || *p=='+' || *p=='-' || *p=='*' || *p=='/' || *p=='(' || *p==')'
                  || *p=='=' || *p==';' || *p=='{' || *p=='}' || *p==',' || *p=='&'){
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
                       char *pre=p;
                       while(isalpha(*p)||isdigit(*p)){
                               p++;
                       }
                       cur = new_token(TK_IDENT,cur,pre,p-pre);
                       continue;
               }
               //printf("eee");
               error_at(p," can not tokenize '%c'",*p);
       }
       cur = new_token(TK_EOF,cur,p,0);
       return head.next;
}

Node *new_node(NodeKind kind,Node *lhs,Node *rhs,Token *token){
       Node *ans=calloc(1,sizeof(Node));
       ans->kind=kind;
       ans->lhs=lhs;
       ans->rhs=rhs;
       ans->token=token;
       return ans;
}
Node *new_node_num(int val,Token *token){
       //leaf node
       Node *ans=calloc(1,sizeof(Node));
       ans->kind=ND_NUM;
       ans->val=val;
       ans->token=token;
       return ans;
}
LVar *locals=NULL;

LVar *find_lvar(Token *tok){
       for(LVar *var=locals;var;var=var->next){
               if(tok->len==var->len && !memcmp(tok->str,var->name,tok->len)){
                       return var;
               }
       }
       return NULL;
}
/* program    = stmt* */
/* stmt       = expr ";"
              | "{" stmt* "}"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "while" "(" expr ")" stmt
              | "for" "(" expr? ";" expr? ";" expr? ")" stmt
              | "int" ident ";"
              | "return" expr ";" */
/* exprs      = expr ("," expr)* */
/* expr       = assign */
/* assign     = equality ("=" assign)? */
/* equality   = relational ("==" relational | "!=" relational)* */
/* relational = add ("<" add | "<=" add | ">" add | ">=" add)* */
/* add        = mul ("+" mul | "-" mul)* */
/* mul     = unary ("*" unary | "/" unary)* */
/* unary   = "-"? primary | "+"? primary | "*" unary | "&" unary  */
/* primary = num | ident ("(" exprs? ")")? | "(" expr ")" */
Node *expr();
/* primary = num | ident ("(" exprs? ")")? | "(" expr ")" */
/* exprs      = expr ("," expr)* */
Node *primary(){
       fprintf(tout,"# <%s>\n",__func__);
       if(consume("(")){
               Node*ans = expr();
               expect(")");//important
               fprintf(tout,"# </%s>\n",__func__);
               return ans;
       }
       Token* tok=consume_ident();
       if(tok){
               if(consume("(")){//call
                       Node*ans = new_node(ND_FUNCALL,NULL,NULL,tok);
                       //ans->name=strndup(tok->str,tok->len);
                       ans->name=malloc(sizeof(char)*tok->len+1);
                       strncpy(ans->name,tok->str,tok->len);
                       ans->params=NULL;
                       if(consume(")")){
                               fprintf(tout,"# </%s>\n",__func__);
                               return ans;
                       }
                       ans->params=calloc(6,sizeof(Node*));
                       int i=0;
                       ans->params[i++]=expr();
                       for(;i<6 && !consume(")");i++){
                               consume(",");
                               ans->params[i]=expr();
                       }
                       fprintf(tout,"# </%s>\n",__func__);
                       return ans;
               }else{//var
                       Node *ans = new_node(ND_LVAR,NULL,NULL,tok);
                       LVar *var = find_lvar(tok);
                       if(var){
                               ans->offset=var->offset;
                       }else{
                               error_at(tok->str,"token '%s' is not defined",tok->str);
                       }
                       fprintf(tout,"# </%s>\n",__func__);
                       //ans->offset=(tok->str[0]-'a'+1)*8;
                       return ans;
               }
       }
       fprintf(tout,"# </%s>\n",__func__);
       return  new_node_num(expect_num(),NULL);
}
///* unary   = "-"? primary | "+"? primary | "*" unary | "&" unary  */
Node *unary(){
       Token *tok=NULL;
       if(tok=consume("+")){
               return primary();
       }
       if(tok=consume("-")){
               //important
               return new_node(ND_SUB,new_node_num(0,NULL),primary(),tok);//0-primary()
       }
       if(tok=consume("*")){
               return new_node(ND_DEREF,unary(),NULL,tok);
       }
       if(tok=consume("&")){
               return new_node(ND_ADDR,unary(),NULL,tok);
       }
       return primary();
}
Node *mul(){
       Token *tok=NULL;
       Node *node=unary();
       for(;;){
               if(tok=consume("*")){
                       node=new_node(ND_MUL,node,unary(),tok);
               }
               if(tok=consume("/")){
                       node=new_node(ND_DIV,node,unary(),tok);
               }
               return node;
       }
}
Node *add(){
       Token *tok=NULL;
       Node *node=mul();
       for(;;){
               if(tok=consume("-")){
                       node=new_node(ND_SUB,node,mul(),tok);
                       continue;
               }
               if(tok=consume("+")){
                       node=new_node(ND_ADD,node,mul(),tok);
                       continue;
               }
               return node;
       }
}
Node *relational(){
       fprintf(tout,"# <%s>\n",__func__);
       Token *tok=NULL;
       Node *node=add();
       for(;;){
               if(tok=consume("<=")){
                       node=new_node(ND_LE,node,add(),tok);
                       continue;
               }
               if(tok=consume(">=")){
                       node=new_node(ND_LE,add(),node,tok);//swap!
                       continue;
               }
               if(tok=consume("<")){
                       node=new_node(ND_LT,node,add(),tok);
                       continue;
               }
               if(tok=consume(">")){
                       node=new_node(ND_LT,add(),node,tok);//swap!
                       continue;
               }
               fprintf(tout,"# </%s>\n",__func__);
               return node;
       }
}
Node *equality(){
       fprintf(tout,"# <%s>\n",__func__);
       Token *tok=NULL;
       Node *node=relational();
       for(;;){
               if(tok=consume("==")){
                       node=new_node(ND_EQ,node,relational(),tok);
                       continue;
               }
               if(tok=consume("!=")){
                       node=new_node(ND_NE,node,relational(),tok);
                       continue;
               }
               fprintf(tout,"# </%s>\n",__func__);
               return node;
       }
}

Node *assign(){
       fprintf(tout,"# <%s>\n",__func__);
       Token *tok=NULL;
       Node *node=equality();
       if(tok=consume("=")){
               node=new_node(ND_ASSIGN,node,assign(),tok);
               fprintf(tout,"# </%s>\n",__func__);
               return node;
       }
       fprintf(tout,"# </%s>\n",__func__);
       return node;
}
Node *expr(){
       fprintf(tout,"# <%s>\n",__func__);
       Node*node=assign();
       fprintf(tout,"# </%s>\n",__func__);
       return node;
}
Node *stmt(){
       /* stmt       = expr ";"
          | "if" "(" expr ")" stmt ("else" stmt)?
          | "while" "(" expr ")" stmt
          | "for" "(" expr? ";" expr? ";" expr? ")" stmt
          | "int" ident ";"
          | "return" expr ";" */
       Node *node=NULL;
       Token *tok=NULL;
       fprintf(tout,"# <%s>\n",__func__);
       if(consume_Token(TK_TYPE)){
               while(consume("*"));
               Token*tok = consume_ident();//ident
               LVar *var = find_lvar(tok);//
               if(var){
                       error_at(tok->str,"token '%s' is already defined",tok->str);
               }else{
                       var=calloc(1,sizeof(LVar));
                       var->next=locals;
                       var->name=tok->str;
                       var->len=tok->len;
                       var->offset=locals->offset+8;//last offset+1;
                       //ans->offset=var->offset;
                       locals=var;
               }
               fprintf(tout,"# </%s>\n",__func__);
               expect(";");
               return stmt();
               //skip token
       }
       if(tok=consume_Token(TK_RETURN)){
               node=new_node(ND_RETURN,node,expr(),tok);
               expect(";");
               fprintf(tout,"# </%s>\n",__func__);
               return node;
       }
       if(tok=consume_Token(TK_IF)){
               expect("(");
               node=new_node(ND_IF,NULL,NULL,tok);
               node->cond=expr();
               expect(")");
               node->then=stmt();
               if(consume_Token(TK_ELSE)){
                       node->els=stmt();
               }
               fprintf(tout,"# </%s>\n",__func__);
               return node;
       }
       if(tok=consume_Token(TK_WHILE)){
               expect("(");
               node=new_node(ND_WHILE,NULL,NULL,tok);
               node->cond=expr();
               expect(")");
               node->then=stmt();
               fprintf(tout,"# </%s>\n",__func__);
               return node;
       }
       if(tok=consume_Token(TK_FOR)){
               /* Node *init;//for init */
               /* Node *cond;//if,while,for cond */
               /* Node *next;//for next */
               /* Node *then;//if,while,for then */
               fprintf(tout,"# <for>\n",__func__);
               expect("(");
               node=new_node(ND_FOR,NULL,NULL,tok);
               fprintf(tout,"# <init>\n",__func__);
               if(!consume(";")){
                       node->init=expr();
                       expect(";");
               }
               fprintf(tout,"# </init>\n",__func__);
               fprintf(tout,"# <cond>\n",__func__);
               if(!consume(";")){
                       node->cond=expr();
                       expect(";");
               }
               fprintf(tout,"# </cond>\n",__func__);
               fprintf(tout,"# <next>\n",__func__);
               if(!consume(")")){
                       node->next=expr();
                       expect(")");
               }
               fprintf(tout,"# </next>\n",__func__);
               node->then=stmt();
               fprintf(tout,"# </for>\n",__func__);
               fprintf(tout,"# </%s>\n",__func__);
               return node;
       }
       // "{" stmt* "}"
       if(tok=consume("{")){
               fprintf(tout,"# <{>\n",__func__);
               node=new_node(ND_BLOCK,NULL,NULL,tok);
               Node **stmts=calloc(100,sizeof(Node*));
               int i;
               for(i=0;i<100 && !consume("}");i++){
                       stmts[i]=stmt();
               }
               assert(i!=100);
               node->stmts=stmts;
               fprintf(tout,"# <}>\n",__func__);
               fprintf(tout,"# </%s>\n",__func__);
               return node;
       }
       node=expr();
       expect(";");
       fprintf(tout,"# </%s>\n",__func__);
       return node;
}
Node *arg(){
       fprintf(tout,"# <%s>\n",__func__);
       consume_Token(TK_TYPE);
       Token*tok = consume_ident();
       Node *ans = new_node(ND_LVAR,NULL,NULL,tok);
       LVar *var = find_lvar(tok);
       if(var){
               ans->offset=var->offset;//TODO: shadow
       }else{
               var=calloc(1,sizeof(LVar));
               var->next=locals;
               var->name=tok->str;
               var->len=tok->len;
               var->offset=locals->offset+8;//last offset+1;
               ans->offset=var->offset;
               locals=var;
       }
       fprintf(tout,"# </%s>\n",__func__);
       return ans;
}

Node *func(){
       fprintf(tout,"# <%s>\n",__func__);
       consume_Token(TK_TYPE);
       Token* tok=consume_ident();
       if(tok){
               expect("(");//dec
               Node*ans = new_node(ND_FUNC,NULL,NULL,tok);
               //ans->name=strndup(tok->str,tok->len);
               ans->name=malloc(sizeof(char)*tok->len+1);
               strncpy(ans->name,tok->str,tok->len);
               printf("# ans->name:%s",ans->name);
               ans->params=NULL;
               ans->params=calloc(6,sizeof(Node*));
               int i=0;
               if(!consume(")")){
                       ans->params[i++]=arg();
                       for(;i<6 && !consume(")");i++){
                               consume(",");
                               ans->params[i]=arg();
                       }
               }
               int off=locals->offset;
               ans->then=stmt();//block
               ans->offset=locals->offset-off;
               fprintf(tout,"# </%s>\n",__func__);
               return ans;
       }
}
Node *code[100]={0};
void program(){
       int i=0;
       while(!at_eof()){
               fprintf(tout,"# c:%d:%s\n",i,token->str);
               //fprintf(tout,"# c:%d:%d\n",i,code[i]->kind);
               code[i++]=func();
       }
       code[i]=NULL;
}
