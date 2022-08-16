#include <stdlib.h>
#include <stdio.h>

typedef enum
{
       TY_CHAR,
       TY_INT,
       TY_PTR,
       TY_ARRAY
} TypeKind;

typedef struct Type Type;
struct Type
{
       TypeKind kind;
       struct Type *ptr_to;
       size_t array_size;
};

typedef enum
{
       TK_STR,
       TK_SIZEOF,
       TK_TYPE,
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
       char *pos;      // token position
       int len;        // token length
};

typedef enum
{
       ND_STR,
       ND_GVAR,
       ND_DEREF,
       ND_ADDR,
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
       Node *cond;   // if,while,for cond
       Node *then;   // if,while,for then
       Node *els;    // if else
       Node *init;   // for init
       Node *next;   // for next
       Node **stmts; // block
       Node *tail;
       Node *head;
       Node *next2;
       Node **params; // funcall
       Type *type;
       int val;    // enable iff kind == ND_NUM
       int offset; // enable iff kind == ND_LVAR
       char *name; // enable iff kind == ND_FUNCALL
       Token *token;
};

typedef struct LVar LVar;

struct LVar
{
       LVar *next;
       char *name; // null terminated string
       Type *type;
       int len;
       int offset; // offset from RBP
       char *init;
};

Token *tokenize(char *p);
Node *expr();
Type *gen(Node *root);
extern FILE *tout;
void program();
void error_at(char *loc, char *fmt, ...);

typedef struct HashNode HashNode;

struct HashNode
{
       HashNode *next_bucket;
       HashNode *next; // for iteration;
       char *key;
       void *value;
};

typedef struct HashMap HashMap;

struct HashMap
{
       HashNode **nodes;
       HashNode *begin; // for iteration;
       int size;
};

HashMap *new_hash(int size);
void add_hash(HashMap *h, char *key, void *value);
void *get_hash(HashMap *h, char *key);

#define MAX(a, b) ((a) > (b) ? (a) : (b))