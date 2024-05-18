#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <stdlib.h>

#define STACK_DEFINE(LINKAGE, TYPE, NAME)       \
                                                \
struct NAME {                                   \
        TYPE *arr;                              \
        TYPE *tos;                              \
        int sz;                                 \
};                                              \
                                                \
LINKAGE int                                     \
NAME ## _init(struct NAME *s, int size)         \
{                                               \
        s->arr = malloc(sizeof(TYPE) * size);   \
        if (!s->arr)                            \
                return -1;                      \
        s->tos = s->arr + size;                 \
        s->sz = size;                           \
        return 0;                               \
}                                               \
                                                \
LINKAGE void                                    \
NAME ## _free(struct NAME *s)                   \
{                                               \
        free(s->arr);                           \
        s->arr = s->tos = NULL;                 \
        s->sz = 0;                              \
}                                               \
                                                \
LINKAGE bool                                    \
NAME ## _full(const struct NAME *s)             \
{                                               \
        return s->tos == s->arr;                \
}                                               \
                                                \
LINKAGE bool                                    \
NAME ## _empty(const struct NAME *s)            \
{                                               \
        return s->tos == s->arr + s->sz;        \
}                                               \
                                                \
LINKAGE void                                    \
NAME ## _put(struct NAME *s, TYPE elem)         \
{                                               \
        *--s->tos = elem;                       \
}                                               \
                                                \
LINKAGE TYPE                                    \
NAME ## _get(struct NAME *s)                    \
{                                               \
        return *s->tos++;                       \
}

#endif
