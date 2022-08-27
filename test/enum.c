#include "test.h"
//#include "../9cc.h"

enum hue
{
    chartreuse,
    burgundy,
    // claret = 20,
    winedark
};
enum hue col, *cp;

typedef enum
{
    TY_CHAR,
    TY_INT,
    TY_LONG,
    TY_PTR,
    TY_ARRAY,
    TY_STRUCT
} TypeKind;

enum
{
    EA,
    EB
};
int main(int argc, char **argv)
{
    TypeKind t = TY_INT;
    ASSERT(TY_INT, t);
    ASSERT(1, t);
    col = winedark;
    ASSERT(winedark, col);
    cp = &col;
    ASSERT(winedark, *cp);
    ASSERT(2, *cp);
    return 0;
}