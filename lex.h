#ifndef LEX_H
#define LEX_H

#include <limits.h>

enum {
        TOK_EOS = '\0',
        TOK_CARET = '^',
        TOK_DOLLAR = '$',
        TOK_LBRACK = '[',
        TOK_RBRACK = ']',
        TOK_LPAREN = '(',
        TOK_RPAREN = ')',
        TOK_DASH = '-',
        TOK_DOT = '.',
        TOK_STAR = '*',
        TOK_PLUS = '+',
        TOK_QMARK = '?',
        TOK_PIPE = '|',
        TOK_CHAR = UCHAR_MAX,
};

extern int lex_init(const char *regex);
extern void lex_free(void);
extern int lex_next(void);
extern int lex_char(void);
extern int lex_type(void);
extern const char *lex_name(void);

#endif
