CFLAGS  = -Wall -Werror -pedantic -fsanitize=address,undefined
SRC     = main.c bitset.c lex.c slab.c parse.c
CC      = gcc

all: $(SRC)
	$(CC) $(CFLAGS) $^
