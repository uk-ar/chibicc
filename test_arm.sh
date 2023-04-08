#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -c test/test.c
  cc -o tmp tmp.s test.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}
assert 2 "foo();"
assert 2 "return foo();"
assert 3 "bar(1,2);"
assert 5 "bar(1+2,2);"
assert 3 "if(1==1){2;return 3;}"
assert 4 "if(1==1){2;3;}return 4;"
assert 2 "for(a=1;a;a=a-1)return 2;"
assert 0 "a=1;for(;a;)a=a-1;return a;"
assert 1 "if(1==1)return 1;"
assert 2 "if(1!=1)return 1;else return 2;"
assert 2 "a=1;while(a)a=a-1;return 2;"
assert 0 "a=1;while(a)a=a-1;return a;"
assert 3 "return 3;"
assert 3 "foo=3;"
assert 3 "foo=3;foo;"
assert 5 "foo=3;2+3;"
assert 3 "foo=3;2;foo;"
assert 3 "foo=3;2+3;foo;"
assert 4 "foo=3;2+3;foo+1;"
assert 6 "foo=1;bar=2+3;foo+bar;"
assert 6 "foo=1;bar=2+3;return foo+bar;"
assert 0 "1>2;"
assert 3 "1+2;"
assert 0 "0;"
assert 4 "4;"
assert 42 "42;"
assert 6 "1+2+3;"
assert 2 "5 - 3;"
assert 7 "1+2*3;"
assert 9 "1*2+(3+4);"
assert 21 "5+20-4;"
assert 10 "-10+20;"
assert 1 "1<=2;"
assert 1 "2==2;"
assert 3 "a=3;"
assert 14 "a=3;b=5*6-8;a+b/2;"

echo OK