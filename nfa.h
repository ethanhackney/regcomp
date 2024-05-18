#ifndef NFA_H
#define NFA_H

#include "bitset.h"
#include <stdbool.h>

enum {
        EMPTY = -3,
        CCL,
        EPSILON,
};

enum {
        NONE,
        START,
        END,
        BOTH = (START | END),
};

struct nfa {
        int edge;
        struct bitset set;
        struct nfa *next;
        struct nfa *next2;
        bool accept;
        int anchor;
};

#endif
