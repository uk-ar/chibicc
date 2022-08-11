CFLAGS=-std=c99 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
TESTSRCS=$(wildcard test/*.c)
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
	$(CC) -static -o $@ test/$*.s

test: $(TESTS)
	for i in $^; do echo $$i; ./$$i || exit 1; echo; done

clean:
	rm -f 9cc *.o *~ tmp* test/*.s test/*.e test/*.exe

.PHONY: test clean
