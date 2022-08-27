#include "test.h"
enum
{
    EA,
    EB
};
int main(int argc, char **argv)
{
    int y;
    ASSERT(3, ({ int x; if (0) x=2; else x=3; x; })); // works!
    ASSERT(2, ({if(1==1){y=2;} y; }));
    ASSERT(3, ({if(1==1){y=2;y=3;} y; }));
    ASSERT(2, ({if(1>2){y=1;}else{y=2;}y; }));
    ASSERT(3, ({if(1==1){2;y=3;}y; }));
    ASSERT(4, ({if(1==1){2;y=3;}y=4;y; }));
    ASSERT(2, ({int a;for(a=1;a;a=a-1) y=2;y; }));
    ASSERT(2, ({for(int a=1;a;a=a-1) y=2;y; }));
    ASSERT(2, ({int a=0;for(int a=1;a;a=a-1) y=2;y; }));
    ASSERT(0, ({int a;a=1;for(;a;)a=a-1; a; }));
    ASSERT(1, ({if(1==1) y=1;y; }));
    ASSERT(2, ({if(1!=1) y=1;else  y=2;y; }));
    ASSERT(2, ({int a;a=1;while(a)a=a-1; y=2;y; }));
    ASSERT(0, ({int a;a=1;while(a)a=a-1; y=0;a;y; }));
    ASSERT(2, (0 || 2));
    ASSERT(2, (0 || 2 || 3));
    ASSERT(1, (1 || 0));
    ASSERT(2, (2 || 1));
    ASSERT(0, (0 || 0));
    ASSERT(1, (0 || 0 || 1));
    ASSERT(0, (0 && 2));
    ASSERT(0, (1 && 0));
    ASSERT(2, (1 && 2));
    ASSERT(3, (1 && 2 && 3));
    ASSERT(0, (1 && 2 && 0));
    ASSERT(0, (0 && 0));

    ASSERT(21, ({int a=1,y=0;switch(a){case 1:y=21;break;case 2:y=22;break;default:y=23;break;}y; }));
    ASSERT(2, ({int a=2,y=0;switch(a){case 1:y=1;break;case 2:y=2;break;default:y=3;break;}y; }));
    ASSERT(3, ({int a=10,y=0;switch(a){case 1:y=1;break;case 2:y=2;break;default:y=3;break;}y; }));
    ASSERT(2, ({int a=1,y=0;switch(a){case 1:y=1;case 2:y=2;break;default:y=3;break;}y; }));
    ASSERT(22, ({int a=EB,y=0;switch(a){case EA:y=21;break;case EB:y=22;break;default:y=23;break;}y; }));

    return 0;
}