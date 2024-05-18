#include "bitset.h"
#include "lex.h"
#include "stack.h"
#include "parse.h"
#include "slab.h"
#include <err.h>
#include <stdio.h>
#include <sysexits.h>
#include <limits.h>

enum {
        NFA_MAX = 1024,
};

STACK_DEFINE(static, int, intstk)

static void print_nfa(struct nfa *nfa, struct nfa *start, int len);
static void free_sets(struct nfa *nfa, struct nfa *start, int len);
static int nextchar(void);
static void printbuf(void);
static struct bitset *e_closure(struct bitset *input, bool *accept, int *anchor);
static struct bitset *move(struct bitset *set, int c);

static char buf[1024];
static char *bufp = buf;
static bool debug = true;
static int max_state;
static struct nfa *states;

int main(int argc, char **argv)
{
        struct nfa *p;
        struct bitset *start_set;
        struct bitset *current;
        struct bitset *next;
        bool accept;
        int sstate;
        int anchor;
        int c;

        if (argc != 2)
                errx(EX_USAGE, "regex");

        if (parser_init(argv[1], NFA_MAX) < 0)
                err(EX_SOFTWARE, "main(): parser_init()");

        p = parser_parse();
        max_state = get_max_state();
        states = get_states();
        if (debug)
                print_nfa(states, p, max_state);
        sstate = p - states;

        next = malloc(sizeof(*next));
        if (!next)
                err(EX_SOFTWARE, "main(): malloc()");
        if (bitset_init(next, 1) < 0)
                err(EX_SOFTWARE, "main(): bitset_init()");
        if (bitset_set(next, sstate) < 0)
                err(EX_SOFTWARE, "main(): bitset_set()");

        start_set = e_closure(next, &accept, &anchor);
        if (!start_set)
                err(EX_SOFTWARE, "main(): empty state machine");

        current = dupset(start_set);
        while ((c = nextchar()) != EOF) {
                struct bitset *m = move(current, c);
                next = e_closure(m, &accept, &anchor);
                if (next) {
                        if (accept)
                                printbuf();
                        else {
                                if (current)
                                        bitset_free(current);
                                current = next;
                        }
                } else {
                        if (next)
                                bitset_free(next);
                        if (current)
                                bitset_free(current);
                        current = dupset(start_set);
                }
                if (m)
                        bitset_free(m);
        }

        free(start_set);
        free_sets(states, p, max_state);
        parser_free();
}

static char *plab(struct nfa *nfa, struct nfa *state);
static void print_ccl(struct bitset *set);

static void
print_nfa(struct nfa *nfa, struct nfa *start, int len)
{
        struct nfa *s = nfa;

        printf("======================= nfa =================\n");

        for (; --len >= 0; nfa++) {
                printf("NFA state %s: ", plab(s, nfa));
                if (!nfa->next) {
                        printf("(TERMINAL)");
                } else {
                        printf("--> %s ", plab(s, nfa->next));
                        printf("(%s) on ", plab(s, nfa->next2));
                        switch (nfa->edge) {
                        case CCL:
                                print_ccl(&nfa->set);
                                break;
                        case EPSILON:
                                printf("EPSILON ");
                                break;
                        default:
                                putchar(nfa->edge);
                                break;
                        }
                }
                if (nfa == start)
                        printf(" (START STATE)");
                if (nfa->accept)
                        printf(" accepting state\n");
                printf("\n");
        }

        printf("======================= nfa =================\n");
}

static char *
plab(struct nfa *nfa, struct nfa *state)
{
        static char buf[32];
        if (!nfa || !state)
                return "--";
        sprintf(buf, "%2ld", state - nfa);
        return buf;
}

static void
print_ccl(struct bitset *set)
{
        int i;

        putchar('[');
        for (i = 0; i <= 0x7f; i++) {
                if (bitset_test(set, i)) {
                        if (i < ' ')
                                printf("^%c", i + '@');
                        else
                                printf("%c", i);
                }
        }
        putchar(']');
}

static void
free_sets(struct nfa *nfa, struct nfa *start, int len)
{
        for (; --len >= 0; nfa++) {
                if (!nfa->next)
                        continue;
                if (nfa->edge != CCL)
                        continue;
                bitset_free(&nfa->set);
        }
}

static int
nextchar(void)
{
        if (!*bufp) {
                if (!fgets(buf, sizeof(buf), stdin))
                        return EOF;
                bufp = buf;
        }
        return *bufp++;
}

static void
printbuf(void)
{
        printf("%s", buf);
        *bufp = 0;
}

static struct bitset *
e_closure(struct bitset *input, bool *accept, int *anchor)
{
        struct intstk stack;
        struct nfa *p;
        int i;
        int acceptnum = INT_MAX;
        word_t word;
        word_t bit;

        *accept = false;

        if (!input)
                return NULL;

        if (intstk_init(&stack, NFA_MAX) < 0)
                err(EX_SOFTWARE, "e_closure(): intstk_init()");

        for (word = bit = 0; word < input->nw; word++) {
                word_t curbit;
                for (curbit = 0; curbit < 64; curbit++) {
                        if (input->set[word] & ((word_t)1 << curbit)) {
                                if (intstk_full(&stack))
                                        err(EX_SOFTWARE, "e_closure(): intstk_put()");
                                intstk_put(&stack, bit);
                        }
                        bit++;
                }
        }

        while (!intstk_empty(&stack)) {
                i = intstk_get(&stack);
                p = &states[i];
                if (p->accept && (i < acceptnum)) {
                        acceptnum = i;
                        *accept = true;
                        *anchor = p->anchor;
                }
                if (p->edge == EPSILON) {
                        if (p->next) {
                                i = p->next - states;
                                if (!bitset_test(input, i)) {
                                        if (bitset_set(input, i) < 0)
                                                err(EX_SOFTWARE, "e_closure(): bitset_set()");
                                        if (intstk_full(&stack))
                                                err(EX_SOFTWARE, "e_closure(): intstk_put()");
                                        intstk_put(&stack, i);
                                }
                        }
                        if (p->next2) {
                                i = p->next2 - states;
                                if (!bitset_test(input, i)) {
                                        if (bitset_set(input, i) < 0)
                                                err(EX_SOFTWARE, "e_closure(): bitset_set()");
                                        if (intstk_full(&stack))
                                                err(EX_SOFTWARE, "e_closure(): intstk_put()");
                                        intstk_put(&stack, i);
                                }
                        }
                }
        }

        intstk_free(&stack);
        return input;
}

static struct bitset *
move(struct bitset *set, int c)
{
        struct nfa *p;
        struct bitset *out = NULL;
        int i;

        for (i = max_state; --i >= 0; ) {
                if (!bitset_test(set, i))
                        continue;
                p = &states[i];
                if (p->edge == c || (p->edge == CCL && bitset_test(&p->set, c) && !p->set.compl)) {
                        if (!out) {
                                out = malloc(sizeof(*out));
                                if (!out)
                                        err(EX_SOFTWARE, "move: malloc()");
                                if (bitset_init(out, 1) < 0)
                                        err(EX_SOFTWARE, "move: bitset_init()");
                        }
                        if (bitset_set(out, p->next - states) < 0)
                                err(EX_SOFTWARE, "move: bitset_set()");
                }
        }

        return out;
}
