#include "lex.h"
#include "parse.h"
#include "slab.h"
#include "bitset.h"
#include <stdio.h>
#include <stdbool.h>
#include <err.h>
#include <sysexits.h>
#include <string.h>

int
parser_init(const char *regex, int slab_size)
{
        if (lex_init(regex))
                return -1;
        if (nfa_slab_init(slab_size) < 0) {
                lex_free();
                return -1;
        }
        lex_next();
        return 0;
}

void
parser_free(void)
{
        nfa_slab_free();
        lex_free();
}

static void expr(struct nfa **startp, struct nfa **endp);

struct nfa *
parser_parse(void)
{
        struct nfa *start = NULL;
        struct nfa *end = NULL;
        int anchor = NONE;

        if (lex_type() == TOK_CARET) {
                start = nfa_slab_get();
                if (!start)
                        err(EX_SOFTWARE, "parser_parse(): nfa_slab_get()");
                start->edge = '\n';
                anchor |= START;
                lex_next();
                expr(&start->next, &end);
        } else {
                expr(&start, &end);
        }

        if (lex_type() == TOK_DOLLAR) {
                lex_next();
                end->next = nfa_slab_get();
                if (!end->next)
                        err(EX_SOFTWARE, "parser_parse(): nfa_slab_get()");
                end->edge = CCL;
                if (bitset_init(&end->set, 1) < 0)
                        err(EX_SOFTWARE, "parser_parse(): bitset_init()");
                bitset_set(&end->set, '\n');
                bitset_compl(&end->set);
                end = end->next;
                anchor |= END;
        }

        end->accept = true;
        end->anchor = anchor;
        lex_next();
        return start;
}

static void cat_expr(struct nfa **startp, struct nfa **endp);

static void
expr(struct nfa **startp, struct nfa **endp)
{
        struct nfa *e2_start = NULL;
        struct nfa *e2_end = NULL;
        struct nfa *p;

        cat_expr(startp, endp);
        while (lex_type() == TOK_PIPE) {
                lex_next();
                cat_expr(&e2_start, &e2_end);
                p = nfa_slab_get();
                if (!p)
                        err(EX_SOFTWARE, "expr(): nfa_slab_get()");
                p->next2 = e2_start;
                p->next = *startp;
                *startp = p;
                p = nfa_slab_get();
                if (!p)
                        err(EX_SOFTWARE, "expr(): nfa_slab_get()");
                (*endp)->next = p;
                e2_end->next = p;
                *endp = p;
        }
}

static bool first_in_cat(void);
static void factor(struct nfa **startp, struct nfa **endp);

static void
cat_expr(struct nfa **startp, struct nfa **endp)
{
        struct nfa *e2_start;
        struct nfa *e2_end;

        if (first_in_cat())
                factor(startp, endp);

        while (first_in_cat()) {
                factor(&e2_start, &e2_end);
                memcpy(*endp, e2_start, sizeof(struct nfa));
                if (nfa_slab_put(e2_start) < 0)
                        err(EX_SOFTWARE, "cat_expr(): nfa_slab_put()");
                *endp = e2_end;
        }
}

static bool
first_in_cat(void)
{
        switch (lex_type()) {
        case TOK_RPAREN:
        case TOK_DOLLAR:
        case TOK_PIPE:
        case TOK_EOS:
                return false;
        case TOK_STAR:
        case TOK_PLUS:
        case TOK_QMARK:
                return false;
        case TOK_RBRACK:
                return false;
        case TOK_CARET:
                return false;
        }
        return true;
}

static void term(struct nfa **startp, struct nfa **endp);

static void
factor(struct nfa **startp, struct nfa **endp)
{
        struct nfa *start;
        struct nfa *end;
        int t;

        term(startp, endp);
        t = lex_type();
        if (t == TOK_STAR || t == TOK_PLUS || t == TOK_QMARK) {
                start = nfa_slab_get();
                if (!start)
                        err(EX_SOFTWARE, "factor(): nfa_slab_get()");
                end = nfa_slab_get();
                if (!end)
                        err(EX_SOFTWARE, "factor(): nfa_slab_get()");
                start->next = *startp;
                (*endp)->next = end;
                if (t == TOK_STAR || t == TOK_QMARK)
                        start->next2 = end;
                if (t == TOK_STAR || t == TOK_PLUS)
                        (*endp)->next2 = *startp;
                *startp = start;
                *endp = end;
                lex_next();
        }
}

static void do_dash(struct bitset *set);

static void
term(struct nfa **startp, struct nfa **endp)
{
        struct nfa *start;
        int c;

        if (lex_type() == TOK_LPAREN) {
                lex_next();
                expr(startp, endp);
                if (lex_type() != TOK_RPAREN)
                        err(EX_SOFTWARE, "term(): expected )");
                return;
        }

        *startp = start = nfa_slab_get();
        if (!*startp)
                err(EX_SOFTWARE, "term(): nfa_slab_get()");
        *endp = start->next = nfa_slab_get();
        if (!*endp)
                err(EX_SOFTWARE, "term(): nfa_slab_get()");

        if (lex_type() != TOK_DOT && lex_type() != TOK_LBRACK) {
                start->edge = lex_char();
                lex_next();
                return;
        }
        start->edge = CCL;

        if (bitset_init(&start->set, 1) < 0)
                err(EX_SOFTWARE, "term(): bitset_init()");

        if (lex_type() == TOK_DOT) {
                if (bitset_set(&start->set, '\n') < 0)
                        err(EX_SOFTWARE, "term(): bitset_set()");
                bitset_compl(&start->set);
                lex_next();
                return;
        }

        lex_next();
        if (lex_type() == TOK_CARET) {
                lex_next();
                if (bitset_set(&start->set, '\n') < 0)
                        err(EX_SOFTWARE, "term(): bitset_set()");
                bitset_compl(&start->set);
        }

        if (lex_type() != TOK_RBRACK) {
                do_dash(&start->set);
        } else {
                for (c = 0; c < ' '; c++) {
                        if (bitset_set(&start->set, c) < 0)
                                err(EX_SOFTWARE, "term(): bitset_set()");
                }
        }
        lex_next();
}

static void
do_dash(struct bitset *set)
{
        int first;

        while (lex_type() != TOK_EOS && lex_type() != TOK_RBRACK) {
                if (lex_type() != TOK_DASH) {
                        first = lex_char();
                        if (bitset_set(set, first) < 0)
                                err(EX_SOFTWARE, "do_dash(): bitset_set()");
                } else {
                        lex_next();
                        for (; first <= lex_char(); first++) {
                                if (bitset_set(set, first) < 0) {
                                        err(EX_SOFTWARE,
                                            "do_dash(): bitset_set()");
                                }
                        }
                }
                lex_next();
        }
}
