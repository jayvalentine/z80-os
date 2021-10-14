#include <stdio.h>

#define RUN_TEST(test_name)             \
    if (test_name())                    \
    {                                   \
        printf(#test_name " X\n");   \
        return 1;                       \
    }                                   \
    else printf(#test_name " O\n")

int test_read_less_than_512_bytes();
int test_read_512_bytes();

int main(void)
{
    RUN_TEST(test_read_less_than_512_bytes);
    RUN_TEST(test_read_512_bytes);
    return 0;
}
