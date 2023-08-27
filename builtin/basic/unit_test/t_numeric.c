#include "test.h"

#include "t_numeric.h"
#include "t_defs.h"

int test_numeric_parse_zero()
{
    const char * input = "0";
    tok_t dst_buf[3];
    tok_t * dst = dst_buf;

    int success = t_numeric_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_NUMERIC, dst_buf[0]);
    ASSERT_EQUAL_INT(0, dst_buf[1]);
    ASSERT_EQUAL_INT(0, dst_buf[2]);

    return 0;
}

int test_numeric_parse_positive()
{
    const char * input = "123";
    tok_t dst_buf[3];
    tok_t * dst = dst_buf;

    int success = t_numeric_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_NUMERIC, dst_buf[0]);
    
    numeric_t num = NUMERIC_GET(&dst_buf[0]);
    ASSERT_EQUAL_INT(123, num);

    return 0;
}

int test_numeric_parse_positive_boundary()
{
    const char * input = "32767";
    tok_t dst_buf[3];
    tok_t * dst = dst_buf;

    int success = t_numeric_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_NUMERIC, dst_buf[0]);
    
    numeric_t num = NUMERIC_GET(&dst_buf[0]);
    ASSERT_EQUAL_INT(32767, num);

    return 0;
}

int test_numeric_size()
{
    const char * input = "32768";
    tok_t dst_buf[3];
    tok_t * dst = dst_buf;

    int success = t_numeric_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_NUMERIC, dst_buf[0]);

    return 0;
}
