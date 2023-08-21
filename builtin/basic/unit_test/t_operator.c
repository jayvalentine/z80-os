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

int test_operator_parse_lparen()
{
    const char * input = "(";

    tok_t dst_buf[2];
    tok_t * dst = dst_buf;

    int success = t_operator_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_OPERATOR, dst_buf[0]);
    ASSERT_EQUAL_INT(OP_LPAREN, dst_buf[1]);

    return 0;
}

int test_operator_parse_rparen()
{
    const char * input = ")";

    tok_t dst_buf[2];
    tok_t * dst = dst_buf;

    int success = t_operator_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_OPERATOR, dst_buf[0]);
    ASSERT_EQUAL_INT(OP_RPAREN, dst_buf[1]);

    return 0;
}

int test_operator_parse_lt()
{
    const char * input = "<";

    tok_t dst_buf[2];
    tok_t * dst = dst_buf;

    int success = t_operator_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_OPERATOR, dst_buf[0]);
    ASSERT_EQUAL_INT(OP_LT, dst_buf[1]);

    return 0;
}

int test_operator_parse_gt()
{
    const char * input = ">";

    tok_t dst_buf[2];
    tok_t * dst = dst_buf;

    int success = t_operator_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_OPERATOR, dst_buf[0]);
    ASSERT_EQUAL_INT(OP_GT, dst_buf[1]);

    return 0;
}

int test_operator_parse_lteq()
{
    const char * input = "<=";

    tok_t dst_buf[2];
    tok_t * dst = dst_buf;

    int success = t_operator_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_OPERATOR, dst_buf[0]);
    ASSERT_EQUAL_INT(OP_LTEQ, dst_buf[1]);

    return 0;
}

int test_operator_parse_gteq()
{
    const char * input = ">=";

    tok_t dst_buf[2];
    tok_t * dst = dst_buf;

    int success = t_operator_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_OPERATOR, dst_buf[0]);
    ASSERT_EQUAL_INT(OP_GTEQ, dst_buf[1]);

    return 0;
}
