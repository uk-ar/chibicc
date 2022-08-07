#!/bin/bash
assert(){
    expected="$1"
    input="$2"

    ./9cc "$input" > tmp.s
    cc -g -c test/test.c
    cc -g -o tmp tmp.s test/test.o
    ./tmp
    actual="$?"

    if [ "$actual" = "$expected" ]; then
       echo "$input => $actual"
    else
       echo "$input => $expected expected, but got $actual"
       exit 1
    fi
}
assert 3 "int main(){int x;int *y;y=&x;return 3;}"
assert 3 "int main(){int x;int *y;y=&x;*y=3;return x;}"
assert 4 "int main(){return 4;}"
assert 3 "int qux(int x){return 3;}int main(){return qux(3);}"
assert 3 "int main(){int a;a=3;return a;}"
assert 3 "main(){int x;int y;x=3;y=&x;return *y;}"
assert 3 "main(){int x;int y;int z;x=3;y=5;z=&y+8;return *z;}"
assert 3 "quxx(){return 3;}qux(){return quxx();}main(){return qux();}"
assert 1 "fib(x){if(x<=1)return x;return fib(x-2)+fib(x-1);}main(){return fib(1);}"
assert 1 "fib(x){if(x<=1)return x;return fib(x-2)+fib(x-1);}main(){return fib(2);}"
assert 2 "fib(x){if(x<=1)return x;return fib(x-2)+fib(x-1);}main(){return fib(3);}"
assert 3 "fib(x){if(x<=1)return x;return fib(x-2)+fib(x-1);}main(){return fib(4);}"
assert 5 "fib(x){if(x<=1)return x;return fib(x-2)+fib(x-1);}main(){return fib(5);}"
assert 8 "fib(x){if(x<=1)return x;return fib(x-2)+fib(x-1);}main(){return fib(6);}"
assert 0 "main(){return 0;}"
assert 3 "main(){return 1+2;}"
assert 2 "main(){return foo();}"
assert 3 "main(){return bar(1,2);}"
assert 3 "qux(){return 3;}main(){return qux();}"
assert 1 "main(){int f;f=1;return f;}"
assert 3 "qux(x){return 3;}main(){return qux(3);}"
assert 3 "qux(x){return x;}main(){return qux(3);}"
assert 6 "qux(x,y){return x+y;}main(){return qux(2+3,1);}"
assert 2 "main(){return foo();}"
assert 3 "main(){return bar(1,2);}"
assert 5 "main(){return bar(1+2,2);}"
assert 3 "main(){return baz(3);}"
assert 3 "main(){if(1==1){2;return 3;}}"
assert 4 "main(){if(1==1){2;3;}return 4;}"
assert 2 "main(){int a;for(a=1;a;a=a-1)return 2;}"
assert 0 "main(){int a;a=1;for(;a;)a=a-1;return a;}"
assert 1 "main(){if(1==1)return 1;}"
assert 2 "main(){if(1!=1)return 1;else return 2;}"
assert 2 "main(){int a;a=1;while(a)a=a-1;return 2;}"
assert 0 "main(){int a;a=1;while(a)a=a-1;return a;}"
assert 3 "main(){return 3;}"
assert 3 "main(){int foo;foo=3;return foo;}"
assert 5 "main(){int foo;foo=3;return 2+3;}"
assert 3 "main(){int foo;foo=3;2;return foo;}"
assert 3 "main(){int foo;foo=3;2+3;return foo;}"
assert 4 "main(){int foo;foo=3;2+3;return foo+1;}"
assert 6 "main(){int foo;int bar;foo=1;bar=2+3;return foo+bar;}"
assert 3 "main(){int a;return a=3;}"
assert 14 "main(){int a;int b;a=3;b=5*6-8;return a+b/2;}"
assert 0 "main(){return 1>2;}"
assert 3 "main(){return 1+2;}"
assert 0 "main(){return 0;}"
assert 4 "main(){return 4;}"
assert 42 "main(){return 42;}"
assert 6 "main(){return 1+2+3;}"
assert 2 "main(){return 5 - 3;}"
assert 7 "main(){return 1+2*3;}"
assert 9 "main(){return 1*2+(3+4);}"
assert 21 "main(){return 5+20-4;}"
assert 10 "main(){return -10+20;}"
assert 1 "main(){return 1<=2;}"
assert 1 "main(){return 2==2;}"
# assert 0 "main(){return ;}"

echo OK
