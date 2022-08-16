#include "test.h"

int main(int argc, char **argv)
{

    ASSERT(3, ({ int x; if (0) x=2; else x=3; x; })); // works!
    int y;
    ASSERT(2, ({if(1==1){y=2;} y; }));
    ASSERT(3, ({if(1==1){y=2;y=3;} y; }));
    ASSERT(2, ({if(1>2){y=1;}else{y=2;}y; }));
    ASSERT(3, ({if(1==1){2;y=3;}y; }));
    ASSERT(4, ({if(1==1){2;y=3;}y=4;y; }));
    ASSERT(2, ({int a;for(a=1;a;a=a-1) y=2;y; }));
    ASSERT(0, ({int a;a=1;for(;a;)a=a-1; a; }));
    ASSERT(1, ({if(1==1) y=1;y; }));
    ASSERT(2, ({if(1!=1) y=1;else  y=2;y; }));
    ASSERT(2, ({int a;a=1;while(a)a=a-1; y=2;y; }));
    ASSERT(0, ({int a;a=1;while(a)a=a-1; y=0;a;y; }));
    return 0;
}