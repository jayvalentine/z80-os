#include <stdio.h>

#define RUN_TEST(test_name)             \
    if (test_name())                    \
    {                                   \
        printf("\n" #test_name " X (line %d)\n", failed_line);   \
        return 1;                       \
    }                                   \
    else printf(".")

int failed_line;


int test_numeric_parse_zero();


int main(void)
{

    RUN_TEST(test_numeric_parse_zero);

    printf("\n");
    return 0;
}
