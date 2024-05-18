#ifndef PARSE_H
#define PARSE_H

#include "nfa.h"

int parser_init(const char *regex, int slab_size);
void parser_free(void);
struct nfa *parser_parse(void);

#endif
