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
#todo string escape
assert 98 "int main(){char *x;x=\"abc\";return x[1];}"
assert 97 "int main(){char *x;x=\"abc\";return ({x[0];});}"
assert 97 "int main(){char *x;x=\"abc\";printC(x[0]);return 97;}"
assert 97 "int main(){printC(({char *x;x=\"abc\";x[0];}));return 97;}"
#assert 97 "int main(){char *x;assert(97,({x=\"abc\";x[0];}),\"foo\");return 97;}"
#assert 0 "int main(){char *x;x=\"abc\";return x[0];}"

assert 97 "int main(){char *x;x=\"abc\";return *x;}"
assert 3  "int main(){char *x;x=\"abc\";puts(x);return 3;}"
assert 3 "int main(){char x[3];x[0]=-1;x[1]=2;int y;y=4;return x[0]+y;}"

#arrays
assert 1 "main(){char a[2];printA(a);printA(a+1);return distance(a,a+1);}"
assert 1 "char a[2];main(){printA(a);printA(a+1);return distance(a,a+1);}"
assert 1 "main(){char a[2];*a=1;*(a+1)=2;printVC(a,2);return *a;}"
assert 2 "main(){char a[2];*a=1;*(a+1)=2;return *(a+1);}"
assert 1 "main(){char a[2];*a=1;*(a+1)=2;return a[0];}"
assert 2 "main(){char a[2];*a=1;*(a+1)=2;return a[1];}"
assert 1 "main(){char a;char b;a=1;b=2;return a;}"
assert 2 "main(){char a;char b;a=1;b=2;return b;}"
# assert 8 "main(){int a;int b;a=1;b=2;printA(&a);printA(&b);return distance(&b,&a);}"
assert 1 "main(){char a;char b;a=1;b=2;printA(&a);printA(&b);return distance(&b,&a);}"

assert 1 "main(){int a[2];a[0]=1;return *a;}"
assert 2 "main(){int a[2];a[0]=2;return a[0];}"
assert 3 "main(){int a[2];*a=3;return a[0];}"
assert 4 "main(){int a[2];printA(a);printA(a+1);return distance(a,a+1);}"
assert 4 "int a[2];main(){printA(a);printA(a+1);return distance(a,a+1);}"

assert 3 "main(){int a[3];*a=1;*(a+1)=2;printI(*(a+1));printVI(a,3);int *p;p=a;return *p+*(p+1);}"
assert 3 "main(){int a[2];*a=1;*(a+1)=2;printI(*(a+1));printVI(a,2);int *p;p=a;return *p+*(p+1);}"
assert 0 "main(){int a[2];*a=1;return 0;}"
assert 1 "main(){int a[2];*a=1;return *a;}"
assert 1 "main(){int a[2];*a=1;*(a+1)=1;printI(*a);return *a;}"

assert 3 "int main(){int a[10];return 3;}"
assert 40 "main(){int a[10];return sizeof(a);}"
assert 1 "main(){int a[2];*a=1;return *a;}"
assert 2 "main(){int a[2];*(a+1)=2;return *(a+1);}"
assert 1 "main(){int a[2];*a=1;return a[0];}"
assert 2 "main(){int a[2];*(a+1)=2;return a[1];}"
assert 1 "main(){int a[2];*a=1;*(a+1)=2;return *a;}"
assert 2 "main(){int a[2];*a=1;*(a+1)=2;return *(a+1);}"
assert 1 "main(){int a[2];*a=1;*(a+1)=2;int *p;p=a;return *p;}"
assert 2 "main(){int a[2];*a=1;*(a+1)=2;int *p;p=a;return *(p+1);}"
assert 2 "main(){int a[2];*a=1;*(a+1)=2;int *p;p=a;return *(a+1);}"
assert 2 "main(){int a[2];*a=1;*(a+1)=2;int *p;p=a;printP(p+1);printP(a+1); return 2;}"
assert 3 "main(){int a[2];*a=1;*(a+1)=2;int *p;p=a;return *p+*(p+1);}"
assert 1 "main(){int a[2];*a=1;*(a+1)=2;return a[0];}"
assert 2 "main(){int a[2];*a=1;*(a+1)=2;return a[1];}"
#assert 1 "main(){int a[2];*a=1;*(a+1)=2;return 0[a];}"
#assert 2 "main(){int a[2];*a=1;*(a+1)=2;return 1[a];}"
#assert 4 "main(){int a[10];return sizeof(a[0]);}"


echo OK
