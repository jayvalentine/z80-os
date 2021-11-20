#include "test.h"

#include "t_variable.h"
#include "t_defs.h"

int test_variable_parse_c()
{
    const char * input = "C";
    tok_t dst_buf[3];
    tok_t * dst = dst_buf;

    int success = t_variable_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_VARIABLE, dst_buf[0]);
    ASSERT_EQUAL_INT(1, dst_buf[1]);
    ASSERT_EQUAL_INT('C', dst_buf[2]);

    return 0;
}

int test_variable_parse_var()
{
    const char * input = "VAR";
    tok_t dst_buf[5];
    tok_t * dst = dst_buf;

    int success = t_variable_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_VARIABLE, dst_buf[0]);
    ASSERT_EQUAL_INT(3, dst_buf[1]);
    ASSERT_EQUAL_INT('V', dst_buf[2]);
    ASSERT_EQUAL_INT('A', dst_buf[3]);
    ASSERT_EQUAL_INT('R', dst_buf[4]);

    return 0;
}

int test_variable_parse_something()
{
    const char * input = "SOMETHING";
    tok_t dst_buf[11];
    tok_t * dst = dst_buf;

    int success = t_variable_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_VARIABLE, dst_buf[0]);
    ASSERT_EQUAL_INT(9, dst_buf[1]);
    ASSERT_EQUAL_INT('S', dst_buf[2]);
    ASSERT_EQUAL_INT('O', dst_buf[3]);
    ASSERT_EQUAL_INT('M', dst_buf[4]);
    ASSERT_EQUAL_INT('E', dst_buf[5]);
    ASSERT_EQUAL_INT('T', dst_buf[6]);
    ASSERT_EQUAL_INT('H', dst_buf[7]);
    ASSERT_EQUAL_INT('I', dst_buf[8]);
    ASSERT_EQUAL_INT('N', dst_buf[9]);
    ASSERT_EQUAL_INT('G', dst_buf[10]);

    return 0;
}
