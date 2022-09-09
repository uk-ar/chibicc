#include "test.h"

int main(int argc, char **argv)
{
    ASSERT(8, sizeof(long int));
    ASSERT(1, sizeof(char));
    ASSERT(4, sizeof(int));
    ASSERT(8, sizeof(long));
    ASSERT(8, sizeof(char *));
    ASSERT(8, sizeof(int *));
    ASSERT(8, sizeof(long *));
    ASSERT(8, sizeof(long int *));
    // ASSERT(1, 2);
    return 0;
}