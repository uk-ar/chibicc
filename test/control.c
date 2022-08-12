#include "test.h"

int main(int argc, char **argv)
{   

    ASSERT(3, ({ int x; if (0) x=2; else x=3; x; }));//works!
    int y;y=0;
    ASSERT(2, ({if(1==1){y=2;} y;}));
    //ASSERT(5, ({if(1==1){y=2;y=3;} y; }));
    //ASSERT(0,({if(1>2){1;}else{2;}})); 
    return 0;
}