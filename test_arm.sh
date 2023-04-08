#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

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