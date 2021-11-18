#include "test.h"

#include "statement.h"

#include "t_numeric.h"
#include "eval.h"
#include "errors.h"

int test_eval_numeric()
{
    const char * input = "123";
    tok_t dst_buf[4];
    tok_t * dst = dst_buf;

    int success = t_numeric_parse(&dst, &input);

    ASSERT(success);

    dst_buf[3] = TOK_TERMINATOR;

    tok_t eval_buf[3];
    tok_t * eval = eval_buf;

    error_t err = eval_numeric(eval, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);

    numeric_t num = t_numeric_get(eval+1);

    ASSERT_EQUAL_INT(123, num);

    return 0;
}

int test_eval_addition()
{
    const char * input = "123+4";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    tok_t eval_buf[3];
    tok_t * eval = eval_buf;

    error_t err = eval_numeric(eval, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);

    numeric_t num = t_numeric_get(eval+1);

    ASSERT_EQUAL_INT(127, num);

    return 0;
}