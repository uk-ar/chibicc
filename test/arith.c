#include "test.h"

int assert(int expected,int actual,char *code){
    if(expected==actual){
        printf("%s => %d\n",code,actual);
    }else{
        printf("%s => %d expected but got %d\n",code,expected,actual);
        exit(1);
    }
    return 0;
}

int main(int argc, char **argv)
{   
    ASSERT(0,1>2); 
    ASSERT(3,1+2);
    ASSERT(0,0);
    ASSERT(4,4);
    ASSERT(42,42);
    ASSERT(6,1+2+3);    
    ASSERT(3,5 - 2);
    ASSERT(7,1+2*3);
    ASSERT(9,1*2+(3+4));
    ASSERT(21,5+20-4);
    ASSERT(10,-10+20);
    ASSERT(1,1<=2);
    ASSERT(1,2==2);
    return 0;
}