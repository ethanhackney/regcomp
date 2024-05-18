#include "bitset.h"
#include <stdlib.h>
#include <string.h>

enum {
        BITS_PER_WORD = 64,
};

int
bitset_init(struct bitset *b, word_t nwords)
{
        b->set = calloc(nwords, sizeof(word_t));
        if (!b->set)
                return -1;
        b->compl = false;
        b->nw = nwords;
        return 0;
}

void
bitset_free(struct bitset *b)
{
        free(b->set);
        b->set = NULL;
        b->compl = false;
        b->nw = 0;
}

int
bitset_set(struct bitset *b, word_t n)
{
        word_t word = n / BITS_PER_WORD;
        word_t bit = n % BITS_PER_WORD;

        if (word >= b->nw) {
                word_t *new = calloc(word + 1, sizeof(word_t));
                if (!new)
                        return -1;
                memcpy(new, b->set, sizeof(word_t) * b->nw);
                free(b->set);
                b->set = new;
                b->nw = word + 1;
        }

        b->set[word] |= ((word_t)1 << bit);
        return 0;
}

bool
bitset_test(struct bitset *b, word_t n)
{
        word_t word = n / BITS_PER_WORD;
        word_t bit = n % BITS_PER_WORD;
        if (word >= b->nw)
                return false;
        return b->set[word] & ((word_t)1 << bit);
}

void
bitset_compl(struct bitset *b)
{
        b->compl = !b->compl;
}

bool
bitset_is_compl(const struct bitset *b)
{
        return b->compl;
}

struct bitset *
dupset(const struct bitset *b)
{
        struct bitset *dup;

        dup = malloc(sizeof(*dup));
        if (!dup)
                return NULL;

        if (bitset_init(dup, b->nw) < 0) {
                free(dup);
                return NULL;
        }

        dup->compl = b->compl;
        dup->nw = b->nw;
        memcpy(dup->set, b->set, sizeof(word_t) * b->nw);
        return dup;
}
