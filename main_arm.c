#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

typedef enum
{
  TK_RESERVED, // symbol
  TK_RETURN,   // return
  TK_IDENT,    // identifier
  TK_NUM,      // int
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token
{
  TokenKind kind; // type of token
  Token *next;    // next input token
  int val;        // token value if TK_NUM
  char *str;      // token position
  int len;        // token length
};

typedef enum
{
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,    // integer
  ND_LVAR,   // variable
  ND_ASSIGN, //=
  ND_RETURN, // return
  ND_LT,     //??
  ND_GT,
  ND_EQ,
  ND_NE,
  ND_LE,
  ND_GE
} NodeKind;

typedef struct Node Node;

struct Node
{ // binary tree node
  NodeKind kind;
  Node *lhs;  // left hand side;
  Node *rhs;  // left hand side;
  int val;    // enable iff kind == ND_NUM
  int offset; // enable iff kind == ND_LVAR
};

Token *tokenize(char *p);
Node *expr();
void gen(Node *root);
FILE *tout;
void program();

Token *token; // current token

char *user_input;
void error_at(char *loc, char *fmt, ...)
{
  va_list ap;
  fprintf(stderr, "%s\n", user_input);
  int pos = loc - user_input;
  fprintf(stderr, "%*s", pos, " "); // output white space
  fprintf(stderr, "^ ");            // output white space
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool consume_Token(TokenKind kind)
{
  if (!token || token->kind != kind)
    return false;
  token = token->next;
  return true;
}

bool consume(char *op)
{ // if next == op, advance & return true;
  // printf("t:%s:%d\n",token->str,token->len);
  if (!token || token->kind != TK_RESERVED)
    return false;
  if (strncmp(op, token->str, strlen(op)) != 0)
    return false;
  // printf("t:%s:%d\n",token->str,token->len);
  token = token->next;
  return true;
}

Token *consume_ident()
{ // if next == op, advance & return true;
  if (!token || token->kind != TK_IDENT)
    return false;
  // printf("t:%s:%d\n",token->str,token->len);
  Token *ans = token;
  token = token->next;
  return ans;
}
void expect(char *op)
{ // if next == op, advance
  if (!token || token->kind != TK_RESERVED)
    error_at(token->str, "token is not '%s'", op);
  if (strncmp(op, token->str, strlen(op)) != 0)
    error_at(token->str, "token is not '%s'", op);
  // printf("t:%s:%d\n",token->str,token->len);
  token = token->next;
}
int expect_num()
{ //
  if (!token || token->kind != TK_NUM)
  {
    error_at(token->str, "token is not number");
  }
  int ans = token->val;
  // printf("t:%s:%d\n",token->str,token->len);
  token = token->next;
  return ans;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str; // token->next=NULL;
  tok->len = len;
  cur->next = tok;
  return tok;
}
bool at_eof()
{
  return !token || token->kind == TK_EOF;
}
bool isIdent(char c)
{
  return isdigit(c) || isalpha(c);
}
Token *tokenize(char *p)
{
  Token head, *cur = &head;
  while (*p)
  {
    if (isspace(*p))
    {
      p++;
      continue;
    }
    if (!strncmp(p, "return", 6) && !isIdent(p[6]))
    {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }
    if (!strncmp(p, "<=", 2) || !strncmp(p, ">=", 2) ||
        !strncmp(p, "==", 2) || !strncmp(p, "!=", 2))
    {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    if (*p == '<' || *p == '>' || *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '=' || *p == ';')
    {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }
    if (isdigit(*p))
    {
      char *pre = p;
      cur = new_token(TK_NUM, cur, p, 0);
      cur->val = strtol(p, &p, 10);
      cur->len = p - pre;
      // printf("%d",p-pre);
      continue;
    }
    if (isalpha(*p))
    {
      char *pre = p;
      while (isalpha(*p) || isdigit(*p))
      {
        p++;
      }
      cur = new_token(TK_IDENT, cur, pre, p - pre);
      continue;
    }
    // printf("eee");
    error_at(p, " can not tokenize '%c'", *p);
  }
  cur = new_token(TK_EOF, cur, p, 0);
  return head.next;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
  Node *ans = calloc(1, sizeof(Node));
  ans->kind = kind;
  ans->lhs = lhs;
  ans->rhs = rhs;
  return ans;
}
Node *new_node_num(int val)
{
  // leaf node
  Node *ans = calloc(1, sizeof(Node));
  ans->kind = ND_NUM;
  ans->val = val;
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
Node *primary()
{
  fprintf(tout, "<%s>\n", __func__);
  if (consume("("))
  {
    Node *ans = expr();
    expect(")"); // important
    fprintf(tout, "</%s>\n", __func__);
    return ans;
  }
  Token *tok = consume_ident();
  if (tok)
  {
    Node *ans = new_node(ND_LVAR, NULL, NULL);
    ans->offset = (tok->str[0] - 'a' + 1) * 8;
    fprintf(tout, "</%s>\n", __func__);
    return ans;
  }
  fprintf(tout, "</%s>\n", __func__);
  return new_node_num(expect_num());
}
Node *unary()
{
  if (consume("+"))
  {
    return primary();
  }
  if (consume("-"))
  {
    // important
    return new_node(ND_SUB, new_node_num(0), primary()); // 0-primary()
  }
  return primary();
}
Node *mul()
{
  Node *node = unary();
  for (;;)
  {
    if (consume("*"))
    {
      node = new_node(ND_MUL, node, unary());
    }
    if (consume("/"))
    {
      node = new_node(ND_DIV, node, unary());
    }
    return node;
  }
}
Node *add()
{
  Node *node = mul();
  for (;;)
  {
    if (consume("-"))
    {
      node = new_node(ND_SUB, node, mul());
      continue;
    }
    if (consume("+"))
    {
      node = new_node(ND_ADD, node, mul());
      continue;
    }
    return node;
  }
}
Node *relational()
{
  fprintf(tout, "<%s>\n", __func__);
  Node *node = add();
  for (;;)
  {
    if (consume("<="))
    {
      node = new_node(ND_LE, node, add());
      continue;
    }
    if (consume(">="))
    {
      node = new_node(ND_LE, add(), node); // swap!
      continue;
    }
    if (consume("<"))
    {
      node = new_node(ND_LT, node, add());
      continue;
    }
    if (consume(">"))
    {
      node = new_node(ND_LT, add(), node); // swap!
      continue;
    }
    fprintf(tout, "</%s>\n", __func__);
    return node;
  }
}
Node *equality()
{
  fprintf(tout, "<%s>\n", __func__);
  Node *node = relational();
  for (;;)
  {
    if (consume("=="))
    {
      node = new_node(ND_EQ, node, relational());
      continue;
    }
    if (consume("!="))
    {
      node = new_node(ND_NE, node, relational());
      continue;
    }
    fprintf(tout, "</%s>\n", __func__);
    return node;
  }
}

Node *assign()
{
  fprintf(tout, "<%s>\n", __func__);
  Node *node = equality();
  if (consume("="))
  {
    node = new_node(ND_ASSIGN, node, assign());
    fprintf(tout, "</%s>\n", __func__);
    return node;
  }
  fprintf(tout, "</%s>\n", __func__);
  return node;
}
Node *expr()
{
  fprintf(tout, "<%s>\n", __func__);
  Node *node = assign();
  fprintf(tout, "</%s>\n", __func__);
  return node;
}
Node *stmt()
{
  Node *node = NULL;
  fprintf(tout, "<%s>\n", __func__);
  if (consume_Token(TK_RETURN))
  {
    node = new_node(ND_RETURN, node, expr());
    expect(";");
    fprintf(tout, "</%s>\n", __func__);
    return node;
  }
  node = expr();  
  expect(";");
  return node;
}
Node *code[100] = {0};
void program()
{
  int i = 0;
  while (!at_eof())
  {
    fprintf(tout, "c:%d\n", i);
    // fprintf(tout,"c:%d:%d\n",i,code[i]->kind);
    code[i++] = stmt();
  }
  code[i] = NULL;
}
void gen_lval(Node *node)
{
  if (node->kind != ND_LVAR)
  {
    printf("node not lvalue");
    abort();
  }

  printf("  mov x0, x29\n");//base pointer
  printf("  sub x0, x0, %d\n",node->offset);//calc local variable address
  printf("  str x0,[SP, #-16]!\n");//push local variable address
}
//
// Code generator
//
void gen(Node *node) {
  if (node->kind == ND_NUM) {
    //printf("  push %d\n", node->val);
    printf("  mov x0,%d\n",node->val);
    printf("  str x0,[SP, #-16]!\n");//push
    return;
  }
  else if (node->kind == ND_LVAR)
  {
    gen_lval(node);
    printf("  ldr x0,[SP],#16\n");//pop
    printf("  ldr x0,[x0]\n");//get data from address
    printf("  str x0,[SP, #-16]!\n");//push local variable value
    //printf("  pop rax\n");        // get address
    //printf("  mov rax, [rax]\n"); // get data from address
    //printf("  push rax\n");       // save local variable value
    //fprintf(tout, "g</%s>\n", nodeKind[node->kind]);
    return;
  }
  else if (node->kind == ND_ASSIGN)
  {
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  ldr x1,[SP],#16\n"); // pop rhs
    printf("  ldr x0,[SP],#16\n"); // pop lhs
    printf("  str x1,[x0]\n");     // assign
    printf("  str x1,[SP, #-16]!\n"); // push expression result
    //printf("  pop rdi\n"); // rhs
    //printf("  pop rax\n"); // lhs
    //printf("  mov [rax],rdi\n");
    //printf("  push rdi\n"); // expression result
    //fprintf(tout, "g</%s>\n", nodeKind[node->kind]);
    return;
  }
  else if (node->kind == ND_RETURN)
  {
    gen(node->rhs);
    // rbp = base pointer = x29
    // rsp = stack pointer = x30
    printf("  ldr x0,[SP],#16\n"); // pop return value
    printf("  mov SP,x29\n");
    printf("  ldp x29,x30,[SP],#16\n");
    printf("  ret\n");

    //printf("  pop rax\n");     // move result to rax
    //printf("  mov rsp,rbp\n"); // restore stack pointer
    //printf("  pop rbp\n");     // restore base pointer
    //printf("  ret\n");
    //fprintf(tout, "g</%s>\n", nodeKind[node->kind]);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  ldr x1,[SP],#16\n");
  printf("  ldr x0,[SP],#16\n");
  //printf("  pop rdi\n");
  //printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add x0,x0,x1\n");
    break;
  case ND_SUB:
    printf("  sub x0,x0,x1\n");
    break;
  case ND_MUL:
    printf("  mul x0,x0,x1\n");
    break;
  case ND_DIV:
    printf("  sdiv x0,x0,x1\n");
    break;
  case ND_EQ://
    printf("  cmp  x0, x1\n");
    printf("  cset x0, eq\n");
    printf("  and  x0, x0, 255\n");
    break;
  case ND_NE:
    printf("  cmp  x0, x1\n");
    printf("  cset x0, ne\n");
    printf("  and  x0, x0, 255\n");
    break;
  case ND_LT:
    printf("  cmp  x0, x1\n");
    printf("  cset x0, lt\n");
    printf("  and  x0, x0, 255\n");
    break;
  case ND_LE:
    printf("  cmp  x0, x1\n");
    printf("  cset x0, le\n");
    printf("  and  x0, x0, 255\n");
    break;
  }
  //printf("  stp x0,x1,[SP, #16]!\n");
  printf("  str x0,[SP, #-16]!\n");
  //printf("  push rax\n");
}

int main(int argc, char **argv)
{
  // tout=stdout;
  tout = stderr;
  if (argc != 2)
  {
    fprintf(stderr, "wrong number of argument\n.");
    return 1;
  }
  // header
  printf(".global main\n");
  printf("main:\n");
  // prepare variables
  // save base pointer
  printf("  stp x29,x30,[SP, #-16]!\n");
  printf("  mov x29,SP\n");
  printf("  sub SP,SP,208\n");//26*8byte
  //printf("  sub sp, sp, 208\n"); // 26*8byte
  //printf("  push rbp\n");     // save base pointer
  //printf("  mov rbp, rsp\n"); // save stack pointer
  //printf("  sub rsp, 208\n"); // 26*8byte

  char *p = argv[1];
  user_input = argv[1];
  token = tokenize(p);
  program();

  for (int i = 0; code[i]; i++)
  {
    // rfprintf(stderr,"c0:%d\n",i);
    // fprintf(stderr,"c0:%d:%d\n",i,code[i]->kind);
    gen(code[i]);

    // pop each result in order not to over flow
    //printf("  pop rax\n");
    printf("  ldr x0,[SP],#16\n");
  }
  
  //printf("  mov rsp,rbp\n"); // restore stack pointer
  //printf("  pop rbp\n");     // restore base pointer
  printf("  mov SP,x29\n");
  printf("  ldp x29,x30,[SP],#16\n");
  printf("  ret\n");
  return 0;
}