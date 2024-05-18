#include "slab.h"
#include "stack.h"
#include <stdlib.h>
#include <string.h>

static struct nfa *states;
static int state_count;
static int inuse;
static int avail;
STACK_DEFINE(static, struct nfa *, nfa_stack)
struct nfa_stack discard;

int
nfa_slab_init(int nr_nfas)
{
        states = calloc(nr_nfas, sizeof(*states));
        if (!states)
                return -1;
        if (nfa_stack_init(&discard, nr_nfas) < 0)
                return -1;
        state_count = nr_nfas;
        inuse = 0;
        avail = 0;
        return 0;
}

void
nfa_slab_free(void)
{
        nfa_stack_free(&discard);
        free(states);
        states = NULL;
        state_count = 0;
        inuse = 0;
        avail = 0;
}

struct nfa *
nfa_slab_get(void)
{
        struct nfa *p;

        inuse++;
        if (inuse == state_count)
                return NULL;

        if (!nfa_stack_empty(&discard)) {
                p = nfa_stack_get(&discard);
        } else {
                p = states + avail;
                avail++;
        }

        p->edge = EPSILON;
        return p;
}

int
nfa_slab_put(struct nfa *nfa)
{
        inuse--;
        memset(nfa, 0, sizeof(*nfa));
        nfa->edge = EMPTY;
        if (nfa_stack_full(&discard))
                return -1;
        nfa_stack_put(&discard, nfa);
        return 0;
}

int
get_max_state(void)
{
        return avail;
}

struct nfa *
get_states(void)
{
        return states;
}
