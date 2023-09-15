#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// prototypeee

typedef enum
{
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;
typedef enum
{
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_NUM,
    ND_EQ,
    ND_NE,
    ND_LT,
    ND_LE,

} NodeKind;
typedef struct Token Token;
typedef struct Node Node;
int eq_cnt = 0;
int le_cnt = 0;
int lt_cnt = 0;
int ne_cnt = 0;
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *primary();
Node *mul();
Node *expr();
Node *unary();
Node *add();
Node *relational();
Node *equality();
Token *token;
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
char *user_input;
bool consume(char *op);
void expect(char *op);
int expect_number();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
bool at_eof();
Token *tokenize();
void gen(Node *node);

struct Token
{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
    int len;
};

struct Node
{
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};
Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }
    return new_node_num(expect_number());
}
Node *mul()
{
    Node *node = unary();
    for (;;)
    {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *expr()
{
    return equality();
}

Node *relational()
{
    Node *node = add();
    for (;;)
    {
        if (consume("<="))
            node = new_node(ND_LE, node, add());
        else if (consume("<"))
            node = new_node(ND_LT, node, add());
        else if (consume(">="))
            node = new_node(ND_LE, add(), node);
        else if (consume(">"))
            node = new_node(ND_LT, add(), node);
        else
            return node;
    }
}
Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}
Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}
Node *unary()
{
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
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

bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}
void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
    {
        error_at(token->str, "not '%c", op);
    }
    token = token->next;
}

int expect_number()
{
    if (token->kind != TK_NUM)
        error_at(token->str, "not number");
    int val = token->val;
    token = token->next;
    return val;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

bool start_with(char *input, char *c)
{

    while (*c)
    {
        if (*input != *c)
            return false;
        input++;
        c++;
    }
    return true;
}
Token *tokenize()
{
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }
        // printf("%s\n", p);
        if (start_with(p, "<=") || start_with(p, ">=") || start_with(p, "!=") || start_with(p, "=="))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p++;
            p++;
            continue;
        }

        if (strchr("+-*/()<>", *p))
        {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p))
        {
            char *q = p;
            int val = strtol(p, &p, 10);
            cur = new_token(TK_NUM, cur, p, p - q);
            cur->val = val;
            continue;
        }

        error_at(p, "expected number!");
    }

    new_token(TK_EOF, cur, p, 1);
    return head.next;
}

void gen(Node *node)
{
    if (node->kind == ND_NUM)
    {
        printf("  mov x0, #%d\n", node->val);
        printf("  str x0, [sp, #-16]!\n");
        return;
    }
    gen(node->lhs);
    gen(node->rhs);

    printf("  ldr x1, [sp], #16\n");
    printf("  ldr x0, [sp], #16\n");

    switch (node->kind)
    {
    case ND_NUM:
        break;
    case ND_ADD:
        printf("  add x0,x0, x1\n");
        break;
    case ND_SUB:
        printf("  sub x0, x0, x1\n");
        break;
    case ND_MUL:
        printf("  mul x0, x0,x1\n");
        break;
    case ND_DIV:
        printf("  sdiv x0,x0,x1\n");
        break;
    case ND_LE:
        le_cnt++;
        printf("  cmp x0, x1\n");
        printf("  ble LE%d\n", le_cnt);
        printf("  mov x0,#0\n");
        printf("  B LEEND%d\n", le_cnt);
        printf("LE%d:\n", le_cnt);
        printf("  mov x0,#1\n");
        printf("LEEND%d:\n", le_cnt);
        break;
    case ND_EQ:
        eq_cnt++;
        printf("  cmp x0, x1\n");
        printf("  beq EQ%d\n", eq_cnt);
        printf("  mov x0,#0\n");
        printf("  B EQEND%d\n", eq_cnt);
        printf("EQ%d:\n", eq_cnt);
        printf("  mov x0,#1\n");
        printf("EQEND%d:\n", eq_cnt);
        break;
    case ND_LT:
        lt_cnt++;
        printf("  cmp x0, x1\n");
        printf("  blt LT%d\n", lt_cnt);
        printf("  mov x0,#0\n");
        printf("  B LTEND%d\n", lt_cnt);
        printf("LT%d:\n", lt_cnt);
        printf("  mov x0,#1\n");
        printf("LTEND%d:\n", lt_cnt);
        break;
    case ND_NE:
        ne_cnt++;
        printf("  cmp x0, x1\n");
        printf("  bne NE%d\n", ne_cnt);
        printf("  mov x0,#0\n");
        printf("  B NEEND%d\n", ne_cnt);
        printf("NE%d:\n", ne_cnt);
        printf("  mov x0,#1\n");
        printf("NEEND%d:\n", ne_cnt);
        break;
    }
    printf("  str x0, [sp, #-16]!\n");
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "argcnt != 2!");
        return 1;
    }
    user_input = argv[1];
    token = tokenize();
    Node *node = expr();
    printf("    .section __TEXT,__text\n");
    printf("    .globl _main\n");
    printf("    .p2align 2\n");
    printf("_main:\n");

    gen(node);
    printf("  ldr x0, [sp], #16\n");
    printf("  ret \n");
    return 0;
}
