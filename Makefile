CFLAGS=-std=c99 -g -static -Wall
SELF_SRCS=main.c hashmap.c #parse.c #codegen tokenizer
HOST_SRCS=$(filter-out $(SELF_SRCS) ,$(wildcard *.c))
SELF_OBJS=$(SELF_SRCS:.c=.o)
HOST_OBJS=$(HOST_SRCS:.c=.o)
ASMS1=$(SELF_SRCS:.c=.1.s)
ASMS2=$(SELF_SRCS:.c=.2.s)
ASMS3=$(SELF_SRCS:.c=.3.s)
TEST_SRCS=$(filter-out test/common.c ,$(wildcard test/*.c))
TESTS1=$(TEST_SRCS:.c=.exe1)
TESTS2=$(TEST_SRCS:.c=.exe2)
TESTS3=$(TEST_SRCS:.c=.exe3)

.PRECIOUS: $(TEST_SRCS:.c=.e) $(TEST_SRCS:.c=.1.s) $(TEST_SRCS:.c=.2.s) test/common.e test/common.1.s $(TEST_SRCS:.c=.3.s) main.e test/common.2.s hashmap.e

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

%.3.s: %.c stage3 %.e %.2.s
	cp $*.e tmp.cx
	./stage3 $*.e > $*.3.s
	diff $*.3.s $*.2.s

test/%.exe1: test/%.c test/%.1.s test/common.o hashmap.o
#テストバイナリ作成
	cp test/$*.1.s tmp.s
	$(CC) -static -g -o $@ test/$*.1.s test/common.o hashmap.o

test/%.exe2: test/%.c test/%.2.s test/common.1.s hashmap.1.s
#テストバイナリ作成
	cp test/$*.2.s tmp.s
	$(CC) -static -g -o $@ test/$*.2.s test/common.1.s hashmap.1.s

test/%.exe3: test/%.c test/%.3.s test/common.2.s hashmap.2.s
#テストバイナリ作成
	cp test/$*.2.s tmp.s
	$(CC) -static -g -o $@ test/$*.2.s test/common.2.s hashmap.2.s

stage1: $(SELF_OBJS) $(HOST_OBJS)
	$(CC) -o $@ $(SELF_OBJS) $(HOST_OBJS) $(CFLAGS)

stage2: test1 stage1 $(ASMS1) $(HOST_OBJS)
	$(CC) -o $@ $(ASMS1) $(HOST_OBJS) $(CFLAGS)

stage3: test2 stage2 $(ASMS2) $(HOST_OBJS)
	$(CC) -o $@ $(ASMS2) $(HOST_OBJS) $(CFLAGS)

$(OBJS):9cc.h

test1: $(TESTS1)
#実行時エラー解析のため名前を統一
	for i in $^; do \
	cp $${i%.*}.e tmp.cx; \
	cp $${i%.*}.1.s tmp.s; \
	cp $${i%.*}.exe1 tmp; \
	echo ./$$i ; \
	if ! ./$$i ; then gcc -static -g -o tmp tmp.1.s test/common.1.s ; exit 1; fi; echo; \
	done
# 失敗したらデバッグ情報付で再コンパイル
# test/common.s1をリンクするとデバッガでmainが追えなくなる？

#if [ ! ./$$i ]; then echo "fail"; else echo "succ"; fi \

test2: $(TESTS2)
	for i in $^; do \
	cp $${i%.*}.e tmp.cx; \
	cp $${i%.*}.2.s tmp.s; \
	cp $${i%.*}.exe2 tmp; \
	echo ./$$i ; \
	if ! ./$$i ; then gcc -static -g -o tmp tmp.s test/common.s ; exit 1; fi; echo; \
	done

diff: $(ASMS3)

test3: $(TESTS3)
	for i in $^; do \
	cp $${i%.*}.e tmp.cx; \
	cp $${i%.*}.3.s tmp.s; \
	cp $${i%.*}.exe3 tmp; \
	echo ./$$i ; \
	if ! ./$$i ; then gcc -static -g -o tmp tmp.s test/common.s ; exit 1; fi; echo; \
	done

all: test3 diff

test_o: stage1 test/common.o
	sh ./test.sh

clean:
	rm -f *.o *~ tmp* 9cc stage1 stage2 stage3 *.e *.s test/*.s test/*.e test/*.exe test/*.exe1 test/*.exe2 test/*.exe3 test/*.o

.PHONY: test1 test2 test3 clean
