//#include <stdlib.h>
//#include <stdio.h>

typedef enum
{
       TY_CHAR,
       TY_INT,
       TY_LONG,
       TY_PTR,
       TY_ARRAY,
       TY_STRUCT
} TypeKind;

typedef struct Type Type;
struct Type
{
       TypeKind kind;
       struct Type *ptr_to;
       long size;
       char *str;
};

typedef enum
{
       TK_NOT_EXIST, // default
       TK_ENUM,
       TK_IDENT, // identifier
       TK_NOT_SUPPORT,
       TK_TYPE_QUAL,
       TK_STORAGE,
       TK_STR,
       TK_SIZEOF,
       TK_TYPE_SPEC,
       TK_IF,
       TK_ELSE,
       TK_WHILE,
       TK_FOR,
       TK_RESERVED, // symbol
       TK_RETURN,   // return
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
       char *str;      // token string
       int loc;
};

typedef enum
{
       ND_CONTINUE,
       ND_COND,
       ND_EXPR,
       ND_CASE,
       ND_BREAK,
       ND_SWITCH,
       ND_OR,
       ND_AND,
       ND_MOD,
       ND_CAST,
       ND_STR,
       ND_GVAR,
       ND_DEREF,
       ND_ADDR,
       ND_FUNC,
       ND_FUNCALL,
       ND_EBLOCK,
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

typedef struct HashNode HashNode;

struct HashNode
{
       HashNode *next_bucket; // 8
       HashNode *next;        // for iteration;//8
       char *key;             // 1+(7)
       void *value;           // 8
};

typedef struct HashMap HashMap;

struct HashMap
{
       HashNode **nodes;
       HashNode *begin; // for iteration;
       int size;
};

typedef struct Node Node;

struct Node
{ // binary tree node
       NodeKind kind;
       Node *lhs;    // left hand side;
       Node *rhs;    // right hand side;
       Node *cond;   // if,while,for cond
       Node *then;   // if,while,for then
       Node *els;    // if else,switch default
       Node *init;   // for init
       Node *inc;    // for next
       Node **stmts; // block
       Node *tail;
       Node *head;
       Node *next; // treat as list
       Type *type;
       HashMap *cases;
       long val;   // enable iff kind == ND_NUM
       int offset; // enable iff kind == ND_LVAR
       Token *token;
};

typedef struct listnode listnode;

struct listnode
{
       listnode *next;
       void *value;
};

typedef struct list list;

struct list
{
       listnode *head;
       listnode *tail;
       int size;
};
extern list *new_list();
extern void add_list(list *l, void *value);

#define bool char
typedef long unsigned int size_t;
typedef struct Obj Obj;

struct Obj
{
       Obj *next;
       Type *type;
       bool is_function;
       Token *token;
       // deprecated
       char *name; // null terminated string
       int len;    // string length of name

       // for locals
       long offset; // offset from RBP
       // for globals
       list *init;
       // for functions
       int stacksize;
       Obj *params;
       Obj *locals;
       Node *body;
};
extern Obj *locals, *globals;

Token *tokenize(char *p);
Node *expr();
Type *gen_stmt(Node *root);
// extern FILE *tout;
Obj *program();
void error_tok(Token *tok, char *fmt, ...);
void error_at(char *s, char *fmt, ...);
HashMap *new_hash(int size);
HashNode *add_hash(HashMap *h, char *key, void *value);
void *get_hash(HashMap *h, char *key);
void *get_node_value(HashNode *n);
char *format(char *fmt, ...);
void codegen(Obj *code, char *filename);
long get_string_offset(char *s);
extern int count();
extern Type *new_type(TypeKind ty, Type *ptr_to, size_t size, char *str, int align);

extern HashMap *keyword2token;
// extern HashMap *structs, *types, *keyword2token, *type_alias, *enums;
Type *ty_int;
Type *ty_char;

#define MAX(a, b) ((a) > (b) ? (a) : (b))