CFLAGS=-std=c99 -g -static -Wall -fPIE
SRCS=$(wildcard *.c)
SRCS+=test/test.c
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS):9cc.h

test:9cc
	sh ./test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean
