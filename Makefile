CFLAGS=-std=c99 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
TESTSRCS=$(filter-out test/common.c,$(wildcard test/*.c))
TESTS=$(TESTSRCS:.c=.exe)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS):9cc.h

test/%.exe: 9cc test/%.c
#プリプロセス結果をcompile
	$(CC) -o test/$*.e -E -P -C test/$*.c
	./9cc test/$*.e > test/$*.s
#cat test/$*.s
#テストバイナリ作成
	$(CC) -static -g -o $@ test/$*.s test/common.c

test: $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done

test_o: 9cc test/common.o
	sh ./test.sh

clean:
	rm -f 9cc *.o *~ tmp* test/*.s test/*.e test/*.exe

.PHONY: test clean
