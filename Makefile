CFLAGS=-std=c99 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
#ASMS=$(filter-out no_self.1.s ,$(SRCS:.c=.1.s))
ASMS2=main.1.s
OBJS2=$($(filter-out $(ASMS2) ,$(SRCS:.c=.1.s)):.1.s=.o)
TESTSRCS=$(filter-out test/common.c ,$(wildcard test/*.c))
TESTS=$(TESTSRCS:.c=.exe1)
TESTS2=$(TESTSRCS:.c=.exe2)
CC=gcc

.PRECIOUS: $(TESTSRCS:.c=.e) $(TESTSRCS:.c=.1.s)

%.e: %.c
#プリプロセス結果をcompile(9ccが標準入力に対応しないため一時ファイルに保存)
	$(CC) -o $*.e -E -P -C $*.c
	
%.1.s: %.c stage1 %.e
#コンパイル時エラー解析のため名前を統一
	cp $*.e tmp.cx
	./stage1 $*.e > $*.1.s	

%.2.s: %.c stage2 %.e
	cp $*.e tmp.cx
	./stage2 $*.e > $*.2.s

test/%.exe1: stage1 test/%.c test/%.1.s test/common.o hashmap.o
#テストバイナリ作成
	cp test/$*.1.s tmp.s
	$(CC) -static -g -o $@ test/$*.1.s test/common.o hashmap.o

test/%.exe2: stage2 test/%.c test/%.2.s test/common.1.s hashmap.1.s
#テストバイナリ作成
	cp test/$*.2.s tmp.s
	$(CC) -static -g -o $@ test/$*.2.s test/common.1.s hashmap.1.s

stage1: $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS)

stage2: test stage1 $(ASMS2) $(OBJS2)
	$(CC) -o $@ $(ASMS2) $(OBJS2) $(CFLAGS)

all: stage2

$(OBJS):9cc.h

test: $(TESTS)
#実行時エラー解析のため名前を統一
	for i in $^; do \
	cp $${i%.*}.e tmp.cx; \
	cp $${i%.*}.s tmp.s; \
	cp $${i%.*}.exe tmp; \
	echo ./$$i ; \
	if ! ./$$i ; then gcc -static -g -o tmp tmp.1.s test/common.1.s ; exit 1; fi; echo; \
	done
# 失敗したらデバッグ情報付で再コンパイル
# test/common.s1をリンクするとデバッガでmainが追えなくなる？

#if [ ! ./$$i ]; then echo "fail"; else echo "succ"; fi \

test2: $(TESTS2)
#実行時エラー解析のため名前を統一
	for i in $^; do \
	cp $${i%.*}.e tmp.cx; \
	cp $${i%.*}.s tmp.s; \
	cp $${i%.*}.exe tmp; \
	echo ./$$i ; \
	if ! ./$$i ; then gcc -static -g -o tmp tmp.s test/common.s ; exit 1; fi; echo; \
	done
# 失敗したらデバッグ情報付で再コンパイル
# test/common.s1をリンクするとデバッガでmainが追えなくなる？

test_o: 9cc test/common.o
	sh ./test.sh

clean:
	rm -f *.o *~ tmp* 9cc stage1 stage2 *.e *.s test/*.s test/*.e test/*.exe test/*.exe1 test/*.o

.PHONY: test clean
