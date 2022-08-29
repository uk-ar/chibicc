CFLAGS=-std=c99 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
ASMS=$(SRCS:.c=.s)
TESTSRCS=$(filter-out test/common.c test/self.c,$(wildcard test/*.c))
TESTS=$(TESTSRCS:.c=.exe)
CC=gcc

.PRECIOUS: test/common.s test/self.s

test/%.exe: stage1 test/%.c test/common.o test/self.o #codegen.s main.s hashmap.s
# parse.s
# 
#プリプロセス結果をcompile(9ccが標準入力に対応しないため一時ファイルに保存)
	$(CC) -o test/$*.e -E -P -C test/$*.c
#コンパイル時エラー解析のため名前を統一
	cp test/$*.e tmp.cx
	./stage1 test/$*.e > test/$*.s	
#テストバイナリ作成
	cp test/$*.s tmp.s
	$(CC) -static -g -o $@ test/$*.s test/common.o test/self.o

stage1: $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS)

stage2: test stage1 $(ASMS)
	$(CC) -o $@ $(ASMS) $(CFLAGS)

all: stage2

$(OBJS):9cc.h
 
%.s: %.c stage1
	$(CC) -o $*.e -E -P -C $*.c
	cp $*.e tmp.cx
	./stage1 $*.e > $*.s	

test: $(TESTS)
#実行時エラー解析のため名前を統一
	for i in $^; do \
	cp $${i%.*}.e tmp.cx; \
	cp $${i%.*}.s tmp.s; \
	cp $${i%.*}.exe tmp; \
	echo ./$$i ; \
	if ! ./$$i ; then gcc -static -g -o tmp tmp.s test/common.s test/self.s; exit 1; fi; echo; \
	done
# 失敗したらデバッグ情報付で再コンパイル
# test/common.s1をリンクするとデバッガでmainが追えなくなる？

#if [ ! ./$$i ]; then echo "fail"; else echo "succ"; fi \

test_o: 9cc test/common.o
	sh ./test.sh

clean:
	rm -f *.o *~ tmp* 9cc stage1 stage2 *.e *.s test/*.s test/*.e test/*.exe

.PHONY: test clean
