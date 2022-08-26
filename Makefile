CFLAGS=-std=c99 -g -static -Wall
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)
TESTSRCS=$(filter-out test/common.c test/self.c,$(wildcard test/*.c))
TESTS=$(TESTSRCS:.c=.exe)
CC=gcc

.PRECIOUS: test/common.s test/self.s

test/%.exe: 9cc test/%.c test/common.s test/self.s
# codegen.s
#プリプロセス結果をcompile(9ccが標準入力に対応しないため一時ファイルに保存)
	$(CC) -o test/$*.e -E -P -C test/$*.c
#コンパイル時エラー解析のため名前を統一
	cp test/$*.e tmp.cx
	./9cc test/$*.e > test/$*.s	
#テストバイナリ作成
	cp test/$*.s tmp.s
	$(CC) -static -g -o $@ test/$*.s test/common.s test/self.s

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS):9cc.h
 
%.s: %.c
	$(CC) -o $*.e -E -P -C $*.c
	cp $*.e tmp.cx
	./9cc $*.e > $*.s	

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
	rm -f 9cc *.o *~ tmp* test/*.s test/*.e test/*.exe

.PHONY: test clean
