typedef enum {
       TK_RESERVED,//symbol
       TK_IDENT,//identifier
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

typedef enum {
       ND_ADD,
       ND_SUB,
       ND_MUL,
       ND_DIV,
       ND_NUM,//integer
       ND_LVAR,//variable
       ND_ASSIGN,//=
       ND_LT,//??
       ND_GT,
       ND_EQ,
       ND_NE,
       ND_LE,
       ND_GE
} NodeKind;

typedef struct Node Node;

struct Node{//binary tree node
       NodeKind kind;
       Node *lhs;//left hand side;
       Node *rhs;//left hand side;
       int val; // enable iff kind == ND_NUM
       int offset; // enable iff kind == ND_LVAR
};

typedef struct LVar LVar;

struct LVar{
       LVar *next;
       char *name;//start pos
       int len;
       int offset;//offset from RBP
};

Token *tokenize(char *p);
Node *expr();
void gen(Node *root);
extern FILE *tout;
void program();
