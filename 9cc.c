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
       int len;//token length
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

typedef enum {
       ND_ADD,
       ND_SUB,
       ND_MUL,
       ND_DIV,
       ND_NUM,
       ND_LT,
       ND_GT,
       ND_EQ,
       ND_NE,
       ND_LE,
       ND_GE
} NodeKind;

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
static FILE *tout;

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
