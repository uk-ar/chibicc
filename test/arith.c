#include "test.h"

int main(int argc, char **argv)
{
    ASSERT(0, 1 > 2);
    ASSERT(3, 1 + 2);
    ASSERT(7, 10-1-2);
    ASSERT(0, 0);
    ASSERT(0, 1 > 2);
    ASSERT(4, 4);
    ASSERT(42, 42);
    ASSERT(6, 1 + 2 + 3);
    ASSERT(3, 5 - 2);
    ASSERT(7, 1 + 2 * 3);
    ASSERT(36, 1 + 2 * 3 * 5 + 2 + 3);
    ASSERT(9, 1 * 2 + (3 + 4));
    ASSERT(21, 5 + 20 - 4);
    ASSERT(10, -10 + 20);
    ASSERT(1, 1 <= 2);
    ASSERT(1, 2 == 2);
    ASSERT(0, 2 % 2);
    ASSERT(1, 1 % 2);
    return 0;
}