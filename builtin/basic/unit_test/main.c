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

int test_numeric_parse_positive();

int test_numeric_parse_positive_boundary();

int test_numeric_parse_negative();

int test_numeric_parse_negative_boundary();

int test_numeric_size();

int test_eval_numeric();

int test_eval_addition();


int main(void)
{

    RUN_TEST(test_numeric_parse_zero);

    RUN_TEST(test_numeric_parse_positive);

    RUN_TEST(test_numeric_parse_positive_boundary);

    RUN_TEST(test_numeric_parse_negative);

    RUN_TEST(test_numeric_parse_negative_boundary);

    RUN_TEST(test_numeric_size);

    RUN_TEST(test_eval_numeric);

    RUN_TEST(test_eval_addition);

    printf("\n");
    return 0;
}
