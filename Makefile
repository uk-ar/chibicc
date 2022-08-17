CFLAGS=-std=c99 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
TESTSRCS=$(filter-out test/common.c,$(wildcard test/*.c))
TESTS=$(TESTSRCS:.c=.exe)
CC=gcc

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS):9cc.h

test/%.exe: 9cc test/%.c
#プリプロセス結果をcompile
	$(CC) -o test/$*.e -E -P -C test/$*.c
	cp test/$*.e tmp.cx
	./9cc test/$*.e > test/$*.s	
#テストバイナリ作成
	cp test/$*.s tmp.s
	$(CC) -static -g -o $@ test/$*.s test/common.c

test: $(TESTS)
	for i in $^; do \
	cp $${i%.*}.e tmp.cx; \
	cp $${i%.*}.s tmp.s; \
	cp $${i%.*}.exe tmp; \
	./$$i || exit 1; echo; \
	done

test_o: 9cc test/common.o
	sh ./test.sh

clean:
	rm -f 9cc *.o *~ tmp* test/*.s test/*.e test/*.exe

.PHONY: test clean
