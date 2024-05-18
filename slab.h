#ifndef SLAB_H
#define SLAB_H

#include "nfa.h"

extern int nfa_slab_init(int nr_nfas);
extern void nfa_slab_free(void);
extern struct nfa *nfa_slab_get(void);
extern int nfa_slab_put(struct nfa *nfa);
extern int get_max_state(void);
extern struct nfa *get_states(void);

#endif
