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
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(123, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

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
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(-456, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    return 0;
}

int test_eval_addition()
{
    const char * input = "123+4";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(127, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

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
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(4, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

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
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(-4, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

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
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(17, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    return 0;
}

int test_eval_addition_subtraction()
{
    /* 10 + (3-4) = 9 */
    const char * input = "10+3-4";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(9, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

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
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(11, num);

    return 0;
}

int test_eval_variable()
{
    program_new();

    /* Set up context. */
    SET_TEST_VAR(VAR, 42);

    const char * input = "VAR";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(42, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    return 0;
}

int test_eval_variable_complex()
{
    program_new();

    /* Set up context. */
    SET_TEST_VAR(VAR, 42);
    SET_TEST_VAR(FRED, 9);

    const char * input = "VAR + 22 - FRED";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);
    
    numeric_t num;
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(55, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    return 0;
}

int test_eval_variable_negation()
{
    program_new();

    /* Set up context. */
    SET_TEST_VAR(VAR, 8);

    const char * input = "-VAR";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(-8, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    return 0;
}

int test_eval_array_access_first()
{
    program_new();

    /* Set up context. */
    tok_t var[10];
    MAKE_VAR_TOK(var, "ARR");
    program_create_array(var, 10);
    tok_t * arr;
    program_get_array(var, &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[0] = 42;

    const char * input = "ARR(1)";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(42, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    return 0;
}

int test_eval_array_access_last()
{
    program_new();

    /* Set up context. */
    tok_t bob[10];
    MAKE_VAR_TOK(bob, "BOB");
    program_create_array(bob, 24);
    tok_t * arr;
    program_get_array(bob, &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[11] = 9;

    const char * input = "BOB(12)";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(9, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    return 0;
}

int test_eval_array_access_mid()
{
    program_new();

    /* Set up context. */
    tok_t foo[10];
    MAKE_VAR_TOK(foo, "FOO");
    program_create_array(foo, 12);
    tok_t * arr;
    program_get_array(foo, &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[2] = 22;

    const char * input = "FOO(3)";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    const tok_t * end;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, err);
    ASSERT_EQUAL_INT(22, num);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    return 0;
}

int test_eval_array_access_outofrange_end()
{
    program_new();

    /* Set up context. */
    tok_t bob[10];
    MAKE_VAR_TOK(bob, "BOB");
    program_create_array(bob, 24);
    tok_t * arr;
    program_get_array(bob, &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[11] = 9;

    const char * input = "BOB(13)";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    const tok_t * end = NULL;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);
    ASSERT_EQUAL_UINT(NULL, end);

    ASSERT_EQUAL_UINT(ERROR_RANGE, err);

    return 0;
}

int test_eval_array_access_outofrange_begin()
{
    program_new();

    /* Set up context. */
    tok_t bob[10];
    MAKE_VAR_TOK(bob, "BOB");
    program_create_array(bob, 6);
    tok_t * arr;
    program_get_array(bob, &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[11] = 9;

    const char * input = "BOB(0)";
    tok_t dst_buf[128];
    tok_t * dst = dst_buf;

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    numeric_t num;
    const tok_t * end = NULL;
    error_t err = eval_numeric(&num, &dst_buf[0], &end);
    ASSERT_EQUAL_UINT(NULL, end);

    ASSERT_EQUAL_UINT(ERROR_RANGE, err);

    return 0;
}

int test_eval_var()
{
    tok_t dst_buf[128];
    error_t e;

    program_new();

    SET_TEST_VAR(I, 42);

    e = statement_tokenize(dst_buf, "I");
    ASSERT_NO_ERROR(e);

    numeric_t num;
    const tok_t * end;
    e = eval_numeric(&num, dst_buf, &end);
    ASSERT_NO_ERROR(e);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    ASSERT_EQUAL_INT(42, num);

    return 0;
}

int test_eval_array_access_variable_index()
{
    tok_t dst_buf[128];
    error_t e;

    program_new();

    /* Set a variable to use as index.
     * Remember indexing starts from 1.
     */
    SET_TEST_VAR(I, 3);

    tok_t var[10];
    MAKE_VAR_TOK(var, "ARR");
    program_create_array(var, 6 * sizeof(numeric_t));
    tok_t * arr;
    program_get_array(var, &arr);
    numeric_t * arr_num = (numeric_t *)(arr + 2);
    arr_num[2] = 42;
    arr_num[4] = 99;

    e = statement_tokenize(dst_buf, "ARR(I)");
    ASSERT_NO_ERROR(e);

    const tok_t * end;

    numeric_t num = 0;
    e = eval_numeric(&num, dst_buf, &end);
    ASSERT_NO_ERROR(e);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    e = statement_tokenize(dst_buf, "ARR(I+2)");
    ASSERT_NO_ERROR(e);

    numeric_t num2 = 0;
    e = eval_numeric(&num2, dst_buf, &end);
    ASSERT_NO_ERROR(e);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    ASSERT_EQUAL_INT(42, num);
    ASSERT_EQUAL_INT(99, num2);
    return 0;
}

int test_eval_rnd()
{
    tok_t dst_buf[128];
    error_t e;

    program_new();

    e = statement_tokenize(dst_buf, "RND");
    ASSERT_NO_ERROR(e);

    numeric_t num;
    const tok_t * end;
    e = eval_numeric(&num, dst_buf, &end);
    ASSERT_NO_ERROR(e);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    return 0;
}

int test_eval_rnd_lim()
{
    tok_t dst_buf[128];
    error_t e;

    program_new();

    e = statement_tokenize(dst_buf, "RND(5)");
    ASSERT_NO_ERROR(e);

    numeric_t num;
    const tok_t * end;
    e = eval_numeric(&num, dst_buf, &end);
    ASSERT_NO_ERROR(e);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    ASSERT(num >= 1 && num <= 5);

    return 0;
}

int test_eval_rnd_in_expr()
{
    tok_t dst_buf[128];
    error_t e;

    program_new();

    e = statement_tokenize(dst_buf, "RND(6) + 9");
    ASSERT_NO_ERROR(e);

    numeric_t num;
    const tok_t * end;
    e = eval_numeric(&num, dst_buf, &end);
    ASSERT_NO_ERROR(e);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    ASSERT(num >= 10 && num <= 15);

    return 0;
}

int test_eval_mul_precedence()
{
    tok_t dst_buf[128];
    error_t e;

    program_new();

    e = statement_tokenize(dst_buf, "5 + 1 * 3");
    ASSERT_NO_ERROR(e);

    numeric_t num;
    const tok_t * end;
    e = eval_numeric(&num, dst_buf, &end);
    ASSERT_NO_ERROR(e);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *end);

    /* Should be: 8 (5 + (1*3))
     * and not:   18 ((5+1) * 3)
     */
    ASSERT_EQUAL_INT(8, num);

    return 0;
}
