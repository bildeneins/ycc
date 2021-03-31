#include "ycc.h"

char *user_input;
Token *token;
Node *code[100];
LVar *locals;

LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = kind;
	node->lhs = lhs;
	node->rhs = rhs;
	return node;
}

Node *new_node_num(int val) {
	Node *node = calloc(1, sizeof(Node));
	node->kind = ND_NUM;
	node->val = val;
	return node;
}

char *user_input;

void error(char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

void error_at(char *loc, char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);

	int pos = loc - user_input;
	fprintf(stderr, "%s\n", user_input);
	fprintf(stderr, "%*s", pos, " ");
	fprintf(stderr, "^ ");
	vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(1);
}

bool consume(char *op) {
	if (token->kind != TK_RESERVED || 
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		return false;
	token = token->next;
	return true;
}

bool consume_token(TokenKind kind) {
	if (token->kind != kind)
		return false;
	token = token->next;
	return true;
}

Token *consume_ident() {
    Token *tok = NULL;
    if (token->kind == TK_IDENT) {
        tok = token;
        token = token->next;
    }
    return tok;
}

void expect(char *op) {
	if (token->kind != TK_RESERVED ||
		strlen(op) != token->len ||
		memcmp(token->str, op, token->len))
		error_at(token->str, "expected '%s'", op);
	token = token->next;
}

int expect_number() {
	if (token->kind != TK_NUM)
		error_at(token->str, "expected a number");
	int val = token->val;
	token = token->next;
	return val;
}

bool at_eof() {
	return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str) {
	Token *tok = calloc(1, sizeof(Token));
	tok->kind = kind;
	tok->str = str;
	cur->next = tok;
	return tok;
}

int is_alnum(char c) {
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           (c == '_');
}

void tokenize() {
    char *p = user_input;
	Token head;
	head.next = NULL;
	Token *cur = &head;

	while (*p) {
		if (isspace(*p)) {
			p++;
			continue;
		}

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            cur = new_token(TK_RETURN, cur, p);
            p += 6;
            continue;
        }

		if (strncmp(p, "<=", 2) == 0 || strncmp(p, ">=", 2) == 0 || strncmp(p, "==", 2) == 0 || strncmp(p, "!=", 2) == 0) {
			cur = new_token(TK_RESERVED, cur, p);
			p += 2;
			cur->len = 2;
			continue;
		}

		if (*p == '<' || *p == '>' ||  *p == '=') {
			cur = new_token(TK_RESERVED, cur, p++);
			cur->len = 1;
			continue;
		}

		if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';') {
			cur = new_token(TK_RESERVED, cur, p++);
			cur->len = 1;
			continue;
		}

		if (isdigit(*p)) {
			cur = new_token(TK_NUM, cur, p);
			cur->val = strtol(p, &p, 10);
			continue;
		}

        if ('a' <= *p && *p <= 'z') {
            char *st = p;
            int count = 1;
            p++;
            while (is_alnum(*p)) {
                count++;
                p++;
            }
            cur = new_token(TK_IDENT, cur, st);
            cur->len = count;
            continue;
        }

		error_at(p, "cannot tokenize");
	}

	new_token(TK_EOF, cur, p);
	token = head.next;
}

Node *primary() {
	if (consume("(")) {
		Node *node = expr();
		expect(")");
		return node;
	}
    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Token));
        node->kind = ND_LVAR;
        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            LVar *lv = calloc(1, sizeof(LVar));
            if (locals) {
                lv->next = locals;
                lv->name = tok->str;
                lv->len = tok->len;
                lv->offset = locals->offset + 8;
                node->offset = lv->offset;
                locals = lv;
            } else {
                lv->next = NULL;
                lv->name = tok->str;
                lv->len = tok->len;
                lv->offset = 8;
                node->offset = lv->offset;
                locals = lv;
            }
        }
        return node;
    }

	return new_node_num(expect_number());
}

Node *unary() {
	if (consume("+"))
		return primary();
	if (consume("-"))
		return new_node(ND_SUB, new_node_num(0), primary());
	return primary();
}

Node *mul() {
	Node *node = unary();

	for (;;) {
		if (consume("*"))
			node = new_node(ND_MUL, node, unary());
		else if (consume("/"))
			node = new_node(ND_DIV, node, unary());
		else return node;
	}
}

Node *add() {
	Node *node = mul();

	for (;;) {
		if (consume("+"))
			node = new_node(ND_ADD, node, mul());
		else if (consume("-"))
			node = new_node(ND_SUB, node, mul());
		else
			return node;
	}
}

Node *relational() {
	Node *node = add();
	
	for (;;) {
		if (consume("<"))
			node = new_node(ND_LT, node, add());
		else if (consume(">"))
			node = new_node(ND_LT, add(), node);
		else if (consume("<="))
			node = new_node(ND_LTEQ, node, add());
		else if (consume(">="))
			node = new_node(ND_LTEQ, add(), node);
		else
			return node;
	}
}

Node *equality() {
	Node *node = relational();

	for (;;) {
		if (consume("=="))
			node = new_node(ND_EQ, node, relational());
		else if (consume("!="))
			node = new_node(ND_NEQ, node, relational());
		else
			return node;
	}
}

Node *assign() {
    Node *node = equality();

    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }

    return node;
};

Node *expr() {
	return assign();
}

Node *stmt() {
    Node *node;
    if (consume_token(TK_RETURN)) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    } else {
        node = expr();
    }
    
    expect(";");
    return node;
};

void program() {
    int i = 0;
    while (!at_eof()) {
        code[i++] = stmt();
    }
        
    code[i] = NULL;
};