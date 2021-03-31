#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
	TK_RESERVED,
    TK_IDENT,
	TK_NUM,
	TK_EOF,
    TK_RETURN
} TokenKind;

typedef struct Token Token;

struct Token {
	TokenKind kind;
	Token *next;
	int val;
	char *str;
	int len;
};

extern Token *token;


typedef enum {
	ND_EQ,
	ND_NEQ,
	ND_LT,
	ND_LTEQ,
	ND_ADD,
	ND_SUB,
	ND_MUL,
	ND_DIV,
    ND_ASSIGN,
    ND_LVAR,
    ND_ID,
	ND_NUM,
    ND_RETURN,
} NodeKind;

typedef struct Node Node;

struct Node {
	NodeKind kind;
	Node *lhs;
	Node *rhs;
	int val;
    int offset;
};

extern Node *code[100];

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};

LVar *locals;

extern LVar *find_lvar(Token *tok);

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);

Node *new_node_num(int val);

extern char *user_input;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

bool consume(char *op);

void expect(char *op);

int expect_number();

bool at_eof();

Token *new_token(TokenKind kind, Token *cur, char *str);

void tokenize();

Node *primary();

Node *unary();

Node *mul();

Node *add();

Node *relational();

Node *equality();

Node *assign();

Node *expr();

Node *stmt();

void program();

void gen(Node *node);
