#include "test.h"

int f1(){
    return 1;
}
int f2(int a){
    return a;
}
int f3(int a,int b){
    return a+b;
}
int f4(){
    return 4+2;
}
int fib(int x){
    if(x<=1){
        return x;
    }        
    return fib(x-2)+fib(x-1);
}
int main(int argc, char **argv)
{   
    ASSERT(1,f1()); 
    ASSERT(2,f2(2)); 
    ASSERT(3,f3(1,2)); 
    ASSERT(7,f3(2+3,2));
    ASSERT(2,foo());
    ASSERT(3,bar(1,2));
    ASSERT(5,bar(1+2,2));
    ASSERT(3,baz(3));
    ASSERT(6,f4());
    
    //ASSERT(1,fib(1));
    //ASSERT(1,fib(2));
    //ASSERT(2,fib(3));
    //ASSERT(3,fib(4));
    //ASSERT(5,fib(5));
    //ASSERT(8,fib(6));
    return 0;
}