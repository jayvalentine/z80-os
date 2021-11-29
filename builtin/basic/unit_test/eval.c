#include "test.h"

#include "statement.h"
#include "program.h"

#include "t_numeric.h"
#include "eval.h"
#include "errors.h"

int test_eval_numeric()
{
    const char * input = "123";
    tok_t dst_buf[256];
    tok_t * dst = dst_buf;

    error_t success = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, success);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(123, num);

    return 0;
}

int test_eval_numeric_negative()
{
    const char * input = "-456";
    tok_t dst_buf[256];
    tok_t * dst = dst_buf;

    error_t success = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, success);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(-456, num);

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

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(127, num);

    return 0;
}

int test_eval_subtraction_positive_result()
{
    const char * input = "9-5";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(4, num);

    return 0;
}

int test_eval_subtraction_negative_result()
{
    const char * input = "10-14";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(-4, num);

    return 0;
}

int test_eval_addition_three_terms()
{
    const char * input = "10+3+4";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);
    
    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(17, num);

    return 0;
}

int test_eval_addition_subtraction()
{
    const char * input = "10+3-4";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(9, num);

    return 0;
}

int test_eval_subtraction_addition()
{
    const char * input = "10-3+4";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(11, num);

    return 0;
}

int test_eval_variable()
{
    program_new();

    /* Set up context. */
    program_set_numeric("VAR", 42);

    const char * input = "VAR";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(42, num);

    return 0;
}

int test_eval_variable_complex()
{
    program_new();

    /* Set up context. */
    program_set_numeric("VAR", 42);
    program_set_numeric("FRED", 9);

    const char * input = "VAR + 22 - FRED";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);
    
    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(55, num);

    return 0;
}

int test_eval_variable_negation()
{
    program_new();

    /* Set up context. */
    program_set_numeric("VAR", 8);

    const char * input = "-VAR";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(-8, num);

    return 0;
}

int test_eval_array_access_first()
{
    program_new();

    /* Set up context. */
    program_create_array("ARR", 10);
    tok_t * arr;
    program_get_array("ARR", &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[0] = 42;

    const char * input = "ARR(1)";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(42, num);

    return 0;
}

int test_eval_array_access_last()
{
    program_new();

    /* Set up context. */
    program_create_array("BOB", 24);
    tok_t * arr;
    program_get_array("BOB", &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[11] = 9;

    const char * input = "BOB(12)";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(9, num);

    return 0;
}

int test_eval_array_access_mid()
{
    program_new();

    /* Set up context. */
    program_create_array("FOO", 12);
    tok_t * arr;
    program_get_array("FOO", &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[2] = 22;

    const char * input = "FOO(3)";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(22, num);

    return 0;
}

int test_eval_array_access_outofrange_end()
{
    program_new();

    /* Set up context. */
    program_create_array("BOB", 24);
    tok_t * arr;
    program_get_array("BOB", &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[11] = 9;

    const char * input = "BOB(13)";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_RANGE, err);

    return 0;
}

int test_eval_array_access_outofrange_begin()
{
    program_new();

    /* Set up context. */
    program_create_array("BOB", 6);
    tok_t * arr;
    program_get_array("BOB", &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[11] = 9;

    const char * input = "BOB(0)";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    error_t err = eval_numeric(&num, &dst_buf[0]);

    ASSERT_EQUAL_UINT(ERROR_RANGE, err);

    return 0;
}
