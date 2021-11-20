#include "test.h"

#include "t_operator.h"
#include "t_defs.h"

int test_operator_parse_minus()
{
    const char * input = "-";

    tok_t dst_buf[2];
    tok_t * dst = dst_buf;

    int success = t_operator_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_OPERATOR, dst_buf[0]);
    ASSERT_EQUAL_INT(OP_MINUS, dst_buf[1]);

    return 0;
}

int test_operator_parse_plus()
{
    const char * input = "+";

    tok_t dst_buf[2];
    tok_t * dst = dst_buf;

    int success = t_operator_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_OPERATOR, dst_buf[0]);
    ASSERT_EQUAL_INT(OP_PLUS, dst_buf[1]);

    return 0;
}

int test_operator_parse_equal()
{
    const char * input = "=";

    tok_t dst_buf[2];
    tok_t * dst = dst_buf;

    int success = t_operator_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_OPERATOR, dst_buf[0]);
    ASSERT_EQUAL_INT(OP_EQUAL, dst_buf[1]);

    return 0;
}
