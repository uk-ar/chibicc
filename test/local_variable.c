#include "test.h"

int main(int argc, char **argv)
{   
    ASSERT(3,({int a;a=3;a;}));//always pass;
    ASSERT(2,({int b;b=2;b;}));    
    ASSERT(14,({int a;int b;a=3;b=5*6-8;a+b/2;}));    
    return 0;
}