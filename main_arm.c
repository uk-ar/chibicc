#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdarg.h>

typedef enum
{
  TK_IF,
  TK_ELSE,
  TK_WHILE,
  TK_FOR,
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
  ND_FUNC,
  ND_FUNCALL,
  ND_BLOCK,
  ND_IF,
  ND_ELSE,
  ND_WHILE,
  ND_FOR,
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
  Node *lhs;    // left hand side;
  Node *rhs;    // right hand side;
  Node *cond;   // if cond
  Node *then;   // if then
  Node *els;    // if else
  Node *init;   // for init
  Node **stmts; // for next
  Node **params;// for funcall
  Node *next;   // for next
  int val;      // enable iff kind == ND_NUM
  int offset;   // enable iff kind == ND_LVAR
  char *name;   // enable iff kind == ND_FUNCALL
};

typedef struct LVar LVar;

struct LVar
{
  LVar *next;
  char *name; // start pos
  int len;
  int offset; // offset from RBP
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
    if (!strncmp(p, "if", 2) && !isIdent(p[2]))
    {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }
    if (!strncmp(p, "else", 4) && !isIdent(p[4]))
    {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }
    if (!strncmp(p, "while", 5) && !isIdent(p[5]))
    {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }
    if (!strncmp(p, "for", 3) && !isIdent(p[3]))
    {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }
    if (!strncmp(p, "<=", 2) || !strncmp(p, ">=", 2) ||
        !strncmp(p, "==", 2) || !strncmp(p, "!=", 2))
    {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }
    if (*p == '<' || *p == '>' || *p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' 
    || *p == '=' || *p == ';' || *p == '{' || *p =='}' || *p==',')
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
LVar *locals = NULL;

LVar *find_lvar(Token *tok)
{
  for (LVar *var = locals; var; var = var->next)
  {
    if (tok->len == var->len && !memcmp(tok->str, var->name, tok->len))
    {
      return var;
    }
  }
  return NULL;
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
    if (consume("("))
    { // call
      Node *ans = new_node(ND_FUNCALL, NULL, NULL);
      //ans->name = strndup(tok->str, tok->len);
      ans->name = malloc(sizeof(char) * tok->len + 1);
      strncpy(ans->name, tok->str, tok->len);
      ans->params = NULL;
      if (consume(")"))
      {
        fprintf(tout, "</%s>\n", __func__);
        return ans;
      }
      ans->params = calloc(6, sizeof(Node *));
      int i = 0;
      ans->params[i++] = expr();
      for (; i < 6 && !consume(")"); i++)
      {
        consume(",");
        ans->params[i] = expr();
      }
      fprintf(tout, "</%s>\n", __func__);
      return ans;
    }
    Node *ans = new_node(ND_LVAR, NULL, NULL);
    LVar *var = find_lvar(tok);
    if (var)
    {
      ans->offset = var->offset;
    }
    else
    {
      /* LVar *next; */
      /* char *name;//start pos */
      /* int len; */
      /* int offset;//offset from RBP */
      var = calloc(1, sizeof(LVar));
      var->next = locals;
      var->name = tok->str;
      var->len = tok->len;
      var->offset = locals->offset + 8; // last offset+1;
      ans->offset = var->offset;
      locals = var;
    }
    fprintf(tout, "# </%s>\n", __func__);
    // ans->offset=(tok->str[0]-'a'+1)*8;
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
  if (consume_Token(TK_IF))
  {
    expect("(");
    node = new_node(ND_IF, NULL, NULL);
    node->cond = expr();
    expect(")");
    node->then = stmt();
    if (consume_Token(TK_ELSE))
    {
      node->els = stmt();
    }
    fprintf(tout, "</%s>\n", __func__);
    return node;
  }
  if (consume_Token(TK_WHILE))
  {
    expect("(");
    node = new_node(ND_WHILE, NULL, NULL);
    node->cond = expr();
    expect(")");
    node->then = stmt();
    fprintf(tout, "</%s>\n", __func__);
    return node;
  }
  if (consume_Token(TK_FOR))
  {
    /* Node *init;//for init */
    /* Node *cond;//if,while,for cond */
    /* Node *next;//for next */
    /* Node *then;//if,while,for then */
    fprintf(tout, "<for>\n", __func__);
    expect("(");
    node = new_node(ND_FOR, NULL, NULL);
    fprintf(tout, "<init>\n", __func__);
    if (!consume(";"))
    {
      node->init = expr();
      expect(";");
    }
    fprintf(tout, "</init>\n", __func__);
    fprintf(tout, "<cond>\n", __func__);
    if (!consume(";"))
    {
      node->cond = expr();
      expect(";");
    }
    fprintf(tout, "</cond>\n", __func__);
    fprintf(tout, "<next>\n", __func__);
    if (!consume(")"))
    {
      node->next = expr();
      expect(")");
    }
    fprintf(tout, "</next>\n", __func__);
    node->then = stmt();
    fprintf(tout, "</for>\n", __func__);
    return node;
  }
  if (consume("{"))
  {
    node = new_node(ND_BLOCK, NULL, NULL);
    Node **stmts = calloc(100, sizeof(Node *));
    int i;
    for (i = 0; i < 100 && !consume("}"); i++)
    {
      stmts[i] = stmt();
    }
    //assert(i != 100);
    node->stmts = stmts;
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

  printf("  mov x0, x29\n");                  // base pointer
  printf("  sub x0, x0, %d\n", node->offset); // calc local variable address
  printf("  str x0,[SP, #-16]!\n");           // push local variable address
}
int count()
{
  static int cnt = 0;
  return cnt++;
}
static char *argreg[] = {"x0", "x1", "x2", "x3", "x4", "x5"};
//
// Code generator
//
void gen(Node *node)
{
  if (node->kind == ND_NUM)
  {
    // printf("  push %d\n", node->val);
    printf("  mov x0,%d\n", node->val);
    printf("  str x0,[SP, #-16]!\n"); // push
    return;
  }
  else if (node->kind == ND_LVAR)
  {
    gen_lval(node);
    printf("  ldr x0,[SP],#16\n");    // pop
    printf("  ldr x0,[x0]\n");        // get data from address
    printf("  str x0,[SP, #-16]!\n"); // push local variable value
    // printf("  pop rax\n");        // get address
    // printf("  mov rax, [rax]\n"); // get data from address
    // printf("  push rax\n");       // save local variable value
    // fprintf(tout, "g</%s>\n", nodeKind[node->kind]);
    return;
  }
  else if (node->kind == ND_ASSIGN)
  {
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  ldr x1,[SP],#16\n");    // pop rhs
    printf("  ldr x0,[SP],#16\n");    // pop lhs
    printf("  str x1,[x0]\n");        // assign
    printf("  str x1,[SP, #-16]!\n"); // push expression result
    // printf("  pop rdi\n"); // rhs
    // printf("  pop rax\n"); // lhs
    // printf("  mov [rax],rdi\n");
    // printf("  push rdi\n"); // expression result
    // fprintf(tout, "g</%s>\n", nodeKind[node->kind]);
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

    // printf("  pop rax\n");     // move result to rax
    // printf("  mov rsp,rbp\n"); // restore stack pointer
    // printf("  pop rbp\n");     // restore base pointer
    // printf("  ret\n");
    // fprintf(tout, "g</%s>\n", nodeKind[node->kind]);
    return;
  }
  else if (node->kind == ND_IF)
  {
    fprintf(tout, "<cond>\n");
    gen(node->cond);
    fprintf(tout, "</cond>\n");
    printf("  ldr x0,[SP],#16\n"); // pop return value
    // printf("  pop rax\n"); // move result to rax
    printf("  cmp x0, 0\n");
    int num = count();
    printf("  beq .Lelse%d\n", num);
    // fprintf(tout, "<then>\n");
    gen(node->then);
    // fprintf(tout, "</then>\n");
    printf("  b .Lend%d\n", num);
    printf(".Lelse%d:\n", num);
    if (node->els)
    {
      gen(node->els);
    }
    printf(".Lend%d:\n", num);
    return;
  }
  else if (node->kind == ND_WHILE)
  {
    int num = count();
    printf(".Lbegin%d:\n", num);
    fprintf(tout, "<cond>\n");
    gen(node->cond);
    fprintf(tout, "</cond>\n");
    printf("  ldr x0,[SP],#16\n"); // pop return value
    // printf("  pop rax\n"); // move result to rax
    // printf("  cmp rax, 0\n");
    printf("  cmp x0, 0\n");
    printf("  beq .Lend%d\n", num);
    fprintf(tout, "<then>\n");
    gen(node->then);
    fprintf(tout, "</then>\n");
    printf("  b .Lbegin%d\n", num);
    printf(".Lend%d:\n", num);
    return;
  }
  else if (node->kind == ND_FOR)
  {
    int num = count();
    if (node->init)
      gen(node->init);
    printf(".Lbegin%d:\n", num);
    fprintf(tout, "<cond>\n");
    if (node->cond)
      gen(node->cond);
    fprintf(tout, "</cond>\n");
    printf("  ldr x0,[SP],#16\n"); // pop return value
    // printf("  pop rax\n"); // move result to rax
    // printf("  cmp rax, 0\n");
    printf("  cmp x0, 0\n");
    // printf("  je .Lend%d\n", num);
    printf("  beq .Lend%d\n", num);
    fprintf(tout, "<then>\n");
    gen(node->then);
    if (node->next)
      gen(node->next);
    fprintf(tout, "</then>\n");
    // printf("  jmp .Lbegin%d\n", num);
    printf("  b .Lbegin%d\n", num);
    printf(".Lend%d:\n", num);
    return;
  }
  else if (node->kind == ND_BLOCK)
  {
    for (int i = 0; i < 100 && node->stmts[i]; i++)
    {
      gen(node->stmts[i]);
      printf("  ldr x0,[SP],#16\n"); // pop return value
    }
    return;
  }
  else if (node->kind == ND_FUNCALL)
  {
    for (int i = 0; i < 6 && node->params && node->params[i]; i++)
    {
      gen(node->params[i]);
    }
    for (int i = 0; i < 6 && node->params && node->params[i]; i++)
    {
      printf("  ldr %s,[SP],#16\n",argreg[i]); // pop return value
      //printf("  pop %s\n", argreg[i]);
    }
    printf("  bl %s\n", node->name);
    printf("  str x0,[SP, #-16]!\n");
    //printf("  push rax\n"); // save result to sp
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  ldr x1,[SP],#16\n");
  printf("  ldr x0,[SP],#16\n");
  // printf("  pop rdi\n");
  // printf("  pop rax\n");

  switch (node->kind)
  {
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
  case ND_EQ: //
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
  // printf("  stp x0,x1,[SP, #16]!\n");
  printf("  str x0,[SP, #-16]!\n");
  // printf("  push rax\n");
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
  printf("  sub SP,SP,208\n"); // 26*8byte
  // printf("  sub sp, sp, 208\n"); // 26*8byte
  // printf("  push rbp\n");     // save base pointer
  // printf("  mov rbp, rsp\n"); // save stack pointer
  // printf("  sub rsp, 208\n"); // 26*8byte

  char *p = argv[1];
  locals = calloc(1, sizeof(LVar));
  user_input = argv[1];
  token = tokenize(p);
  program();

  for (int i = 0; code[i]; i++)
  {
    // rfprintf(stderr,"c0:%d\n",i);
    // fprintf(stderr,"c0:%d:%d\n",i,code[i]->kind);
    gen(code[i]);

    // pop each result in order not to over flow
    // printf("  pop rax\n");
    printf("  ldr x0,[SP],#16\n");
  }

  // printf("  mov rsp,rbp\n"); // restore stack pointer
  // printf("  pop rbp\n");     // restore base pointer
  printf("  mov SP,x29\n");
  printf("  ldp x29,x30,[SP],#16\n");
  printf("  ret\n");
  return 0;
}