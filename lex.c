#include "lex.h"
#include <string.h>
#include <stdlib.h>

static char *expr;
static int c;
static int type;

int
lex_init(const char *regex)
{
        expr = strdup(regex);
        if (!expr)
                return -1;
        c = -1;
        return 0;
}

void
lex_free(void)
{
        free(expr);
        expr = NULL;
}

int
lex_next(void)
{
        c++;
        if (c >= strlen(expr))
                return type = TOK_EOS;
        switch (expr[c]) {
        case '\0':
        case '^':
        case '$':
        case '[':
        case ']':
        case '(':
        case ')':
        case '-':
        case '.':
        case '*':
        case '+':
        case '?':
        case '|':
                return type = expr[c];
        }
        if (expr[c] == '\\')
                c++;
        return type = TOK_CHAR;
}

int
lex_char(void)
{
        return expr[c];
}

int
lex_type(void)
{
        return type;
}

const char *
lex_name(void)
{
        switch (type) {
        case TOK_EOS: return "TOK_EOS";
        case TOK_CARET: return "TOK_CARET";
        case TOK_DOLLAR: return "TOK_DOLLAR";
        case TOK_LBRACK: return "TOK_LBRACK";
        case TOK_RBRACK: return "TOK_RBRACK";
        case TOK_LPAREN: return "TOK_LPAREN";
        case TOK_RPAREN: return "TOK_RPAREN";
        case TOK_DASH: return "TOK_DASH";
        case TOK_DOT: return "TOK_DOT";
        case TOK_STAR: return "TOK_STAR";
        case TOK_PLUS: return "TOK_PLUS";
        case TOK_QMARK: return "TOK_QMARK";
        case TOK_PIPE: return "TOK_PIPE";
        }

        return "TOK_CHAR";
}
