#include <stdio.h>

#define RUN_TEST(test_name)             \
    if (test_name())                    \
    {                                   \
        printf("\n" #test_name " X (line %d)\n", failed_line);   \
        return 1;                       \
    }                                   \
    else printf(".")

int failed_line;



int main(void)
{

    printf("\n");
    return 0;
}
