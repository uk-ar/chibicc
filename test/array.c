
#include "test.h"
char a[2];
int a2[2];
int main(int argc, char **argv)
{
   /*
   ASSERT(0, ({ char x[16]; (unsigned long)&x % 16; }));
   ASSERT(0, ({ char x[17]; (unsigned long)&x % 16; }));
   ASSERT(0, ({ char x[100]; (unsigned long)&x % 16; }));
   ASSERT(0, ({ char x[101]; (unsigned long)&x % 16; }));
   */
   ASSERT(1, ({char a[2];printA(a);printA(a+1);distance(a,a+1); }));
   ASSERT(1, ({printA(a);printA(a+1);distance(a,a+1); }));
   ASSERT(1, ({int a[2];a[0]=1;*a; }));
   ASSERT(2, ({int a[2];a[0]=2;a[0]; }));
   ASSERT(3, ({int a[2];*a=3;a[0]; }));
   ASSERT(4, ({int a[2];printA(a);printA(a+1);distance(a,a+1); }));
   ASSERT(4, ({printA(a2);printA(a2+1);distance(a2,a2+1); }));
   ASSERT(3, ({int a[3];*a=1;*(a+1)=2;printI(*(a+1));printVI(a,3);int *p;p=a;*p+*(p+1); }));
   ASSERT(3, ({int a[2];*a=1;*(a+1)=2;printI(*(a+1));printVI(a,2);int *p;p=a;*p+*(p+1); }));
   ASSERT(0, ({int a[2];*a=1;0; }));
   ASSERT(1, ({int a[2];*a=1;*a; }));
   ASSERT(1, ({int a[2];*a=1;*(a+1)=1;printI(*a);*a; }));

   ASSERT(3, ({int a[10];3; }));
   ASSERT(40, ({int a[10];sizeof(a); }));
   ASSERT(1, ({int a[2];*a=1;*a; }));
   ASSERT(2, ({int a[2];*(a+1)=2;*(a+1); }));
   ASSERT(1, ({int a[2];*a=1;a[0]; }));
   ASSERT(2, ({int a[2];*(a+1)=2;a[1]; }));
   ASSERT(1, ({int a[2];*a=1;*(a+1)=2;*a; }));
   ASSERT(2, ({int a[2];*a=1;*(a+1)=2;*(a+1); }));
   ASSERT(3, ({int a[2];*a=1;*(a+1)=2;*a+*(a+1); }));
   ASSERT(1, ({int a[2];*a=1;*(a+1)=2;int *p;p=a;*p; }));
   ASSERT(2, ({int a[2];*a=1;*(a+1)=2;int *p;p=a;*(p+1); }));
   ASSERT(2, ({int a[2];*a=1;*(a+1)=2;int *p;p=a;*(a+1); }));
   ASSERT(2, ({int a[2];*a=1;*(a+1)=2;int *p;p=a;printP(p+1);printP(a+1); 2; }));
   ASSERT(3, ({int a[2];*a=1;*(a+1)=2;int *p;p=a;*p+*(p+1); }));
   ASSERT(1, ({int a[2];*a=1;*(a+1)=2;a[0]; }));
   ASSERT(2, ({int a[2];*a=1;*(a+1)=2;a[1]; }));
   ASSERT(4, ({int a[2];*a=1;*(a+1)=2;distance(a,a+1); }));
   // TODO: fix local value offset for int
   ASSERT(4, ({int a;int b;a=1;b=2;printA(&a);printA(&b);distance(&b,&a); }));

   ASSERT(1, ({char a[2];*a=1;*a; }));
   ASSERT(2, ({char a[2];*a=1;*(a+1)=2;*(a+1); }));
   ASSERT(1, ({char a[2];*a=1;*(a+1)=2;*a; }));
   ASSERT(1, ({char a[2];*a=1;*(a+1)=2;printVC(a,2);*a; }));

   ASSERT(1, ({char a[2];*a=1;*(a+1)=2;char *p=a;*p; }));
   ASSERT(2, ({char a[2];*a=1;*(a+1)=2;char *p=a;*(p+1); }));
   ASSERT(3, ({char a[2];*a=1;*(a+1)=2;char *p=a;*p+*(p+1); }));
   ASSERT(1, ({char a[2];*a=1;*(a+1)=2;a[0]; }));
   ASSERT(2, ({char a[2];*a=1;*(a+1)=2;a[1]; }));
   ASSERT(1, ({char a[2];*a=1;*(a+1)=2;distance(a,a+1); }));
   ASSERT(1, ({char a;char b;a=1;b=2;a; }));
   ASSERT(2, ({char a;char b;a=1;b=2;b; }));
   ASSERT(1, ({char a;char b;a=1;b=2;printA(&a);printA(&b);distance(&b,&a); }));

   ASSERT(1, ({long a[2];*a=1;*a; }));
   ASSERT(2, ({long a[2];*a=1;*(a+1)=2;*(a+1); }));
   ASSERT(1, ({long a[2];*a=1;*(a+1)=2;*a; }));
   ASSERT(1, ({long a[2];*a=1;*(a+1)=2;printVC(a,2);*a; }));

   ASSERT(1, ({long a[2];*a=1;*(a+1)=2;long *p=a;*p; }));
   ASSERT(2, ({long a[2];*a=1;*(a+1)=2;long *p=a;*(p+1); }));
   ASSERT(3, ({long a[2];*a=1;*(a+1)=2;long *p=a;*p+*(p+1); }));
   ASSERT(1, ({long a[2];*a=1;*(a+1)=2;a[0]; }));
   ASSERT(2, ({long a[2];*a=1;*(a+1)=2;a[1]; }));
   ASSERT(8, ({long a[2];*a=1;*(a+1)=2;distance(a,a+1); }));
   ASSERT(8, ({long a[2];distance(&(a[0]),&(a[1])); }));

   ASSERT(1, ({long a;long b;a=1;b=2;a; }));
   ASSERT(2, ({long a;long b;a=1;b=2;b; }));
   ASSERT(8, ({long a;long b;a=1;b=2;printA(&a);printA(&b);distance(&b,&a); }));

   ASSERT(8, ({void* a[2];distance(&(a[0]),&(a[1])); }));
   ASSERT(8, ({void* a[2];*a=1;*(a+1)=2;distance(a,a+1); }));
   // Not supported yet
   // ASSERT(1 ,({int a[2];*a=1;*(a+1)=2;0[a];}));
   // ASSERT(2 ,({int a[2];*a=1;*(a+1)=2;1[a];}));
   // ASSERT(4 ,({int a[10];sizeof(a[0]);}));*/
   return 0;
}