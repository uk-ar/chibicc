#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    echo "$input" > tmp.cx
    ./9cc tmp.cx > tmp.s
    if [ "$?" -ne 0 ]; then
        echo "compile error"
        exit 1
    fi
    gcc -static -g -o tmp tmp.s test/common.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
       echo "$input => $actual"
    else
       echo "$input => $expected expected, but got $actual"
       exit 1
    fi
}
#sample
assert 1 "main(){char a[2];printA(a);printA(a+1);return distance(a,a+1);}"

echo OK
