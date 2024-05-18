#ifndef BITSET_H
#define BITSET_H

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t word_t;

struct bitset {
        bool compl;
        word_t nw;
        word_t *set;
};

extern int bitset_init(struct bitset *b, word_t nwords);
extern void bitset_free(struct bitset *b);
extern int bitset_set(struct bitset *b, word_t n);
extern bool bitset_test(struct bitset *b, word_t n);
extern void bitset_compl(struct bitset *b);
extern bool bitset_is_compl(const struct bitset *b);
extern struct bitset *dupset(const struct bitset *b);

#endif
