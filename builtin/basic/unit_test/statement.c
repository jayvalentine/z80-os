#include "test.h"

#include "statement.h"
#include "program.h"

#include "t_defs.h"
#include "t_keyword.h"
#include "t_numeric.h"
#include "t_operator.h"

#include "array.h"

#include <string.h>

int test_tokenize_print()
{
    const char * input = "PRINT";
    tok_t dst[10];

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    ASSERT_EQUAL_UINT(TOK_KEYWORD, dst[0]);
    ASSERT_EQUAL_UINT(KEYWORD_PRINT, dst[1]);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, dst[2]);

    return 0;
}

int test_tokenize_list()
{
    const char * input = "LIST";
    tok_t dst[10];

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    ASSERT_EQUAL_UINT(TOK_KEYWORD, dst[0]);
    ASSERT_EQUAL_UINT(KEYWORD_LIST, dst[1]);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, dst[2]);

    return 0;
}

int test_tokenize_variable()
{
    const char * input = "C";
    tok_t dst[10];

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    ASSERT_EQUAL_UINT(TOK_REGISTER, dst[0]);
    ASSERT_EQUAL_UINT(2, dst[1]);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, dst[2]);

    return 0;
}

int test_interpret_assignment()
{
    program_new();

    const char * input = "C=1+2";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    tok_t c[10];
    MAKE_VAR_TOK(c, "C");
    error_t e3 = program_get_numeric(c, &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(3, val);

    return 0;
}

int test_interpret_assignment_from_self()
{
    program_new();

    const char * input = "AM = AM - 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    /* Set up the context. */
    SET_TEST_VAR(AM, 9);

    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    error_t e3 = program_get_numeric(TEST_VAR(AM), &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(5, val);

    return 0;
}

int test_interpret_assignment_from_self_equal()
{
    program_new();

    const char * input = "AM = AM - AM + AM";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    /* Set up the context. */
    SET_TEST_VAR(AM, 9);

    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    error_t e3 = program_get_numeric(TEST_VAR(AM), &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(9, val);

    return 0;
}

/* Program listing.
 *
 * Matches line numbers to program statements.
 */
typedef struct _LISTING_ENTRY_T
{
    numeric_t lineno;
    const tok_t * stmt;
} listing_entry_t;

typedef struct _LISTING_T
{
    uint16_t count;
    int16_t  index; /* -1 if invalid. */
    listing_entry_t entries[100];
} listing_t;

extern listing_t program_listing;

int test_interpret_for_entry()
{
    program_new();

    program_return_t ret_start;
    ret_start.lineno = 123;
    tok_t var[VAR_TOK_BUF_SIZE];
    MAKE_VAR_TOK(var, "VAR");
    ret_start.vartoken = var;

    program_push_return(&ret_start);

    const char * input = "FOR I=1 TO 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    tok_t i[10];
    MAKE_VAR_TOK(i, "I");
    error_t e3 = program_get_numeric(i, &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(1, val);

    /* Program stack should have the for added. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e4);

    ASSERT_EQUAL_INT(456, ret1.lineno);
    ASSERT(varcmp(i, ret1.vartoken) == 0);

    /* Program stack should have the for added. */
    program_return_t ret2;
    error_t e5 = program_pop_return(&ret2);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e5);

    ASSERT_EQUAL_INT(123, ret2.lineno);
    ASSERT(varcmp(var, ret2.vartoken) == 0);

    return 0;
}

int test_interpret_for_continue()
{
    program_new();
    SET_TEST_VAR(I, 1);

    program_return_t ret_start;
    ret_start.lineno = 210;
    ret_start.vartoken = TEST_VAR(I);

    program_push_return(&ret_start);

    const char * input = "FOR I=1 TO 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 130;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    error_t e3 = program_get_numeric(TEST_VAR(I), &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(2, val);

    /* Program stack should have the for added. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e4);

    ASSERT_EQUAL_INT(130, ret1.lineno);
    ASSERT(varcmp(TEST_VAR(I), ret1.vartoken) == 0);

    return 0;
}

int test_interpret_for_hit_limit()
{
    program_new();
    SET_TEST_VAR(I, 3);

    program_return_t ret_start;
    ret_start.lineno = 210;
    ret_start.vartoken = TEST_VAR(I);

    program_push_return(&ret_start);

    const char * input = "FOR I=1 TO 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 130;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    error_t e3 = program_get_numeric(TEST_VAR(I), &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(4, val);

    /* Program stack should be empty. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_RETSTACK_EMPTY, e4);
    
    /* Next line number to execute should be 211. */
    numeric_t next_lineno = program_next_lineno();
    ASSERT_EQUAL_INT(211, next_lineno);

    return 0;
}

int test_interpret_entry_empty_stack()
{
    program_new();

    const char * input = "FOR I=1 TO 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    tok_t i[10];
    MAKE_VAR_TOK(i, "I");
    error_t e3 = program_get_numeric(i, &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(1, val);

    /* Program stack should have the for added. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e4);

    ASSERT_EQUAL_INT(456, ret1.lineno);
    ASSERT(varcmp(i, ret1.vartoken) == 0);

    /* Only one entry in stack. */
    program_return_t ret2;
    error_t e5 = program_pop_return(&ret2);
    ASSERT_EQUAL_UINT(ERROR_RETSTACK_EMPTY, e5);

    return 0;
}

int test_interpret_for_missing_var()
{
    program_new();

    const char * input = "FOR PRINT=1 TO 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_SYNTAX, e2);

    return 0;
}

int test_interpret_for_missing_equals()
{
    program_new();

    const char * input = "FOR I+1 TO 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_SYNTAX, e2);

    return 0;
}

int test_interpret_for_missing_to()
{
    program_new();

    const char * input = "FOR I=1 FOO 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_SYNTAX, e2);

    return 0;
}

int test_interpret_next()
{
    program_new();

    SET_TEST_VAR(I, 3);

    program_return_t ret_start;
    ret_start.lineno = 123;
    ret_start.vartoken = TEST_VAR(I);
    program_push_return(&ret_start);

    const char * input = "NEXT I";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    ASSERT_EQUAL_INT(123, program_next_lineno());

    numeric_t val;
    error_t e3 = program_get_numeric(TEST_VAR(I), &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(3, val);

    /* Program stack should have the for added. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e4);

    ASSERT_EQUAL_INT(456, ret1.lineno);
    ASSERT(varcmp(TEST_VAR(I), ret1.vartoken) == 0);

    /* Only one entry in stack. */
    program_return_t ret2;
    error_t e5 = program_pop_return(&ret2);
    ASSERT_EQUAL_UINT(ERROR_RETSTACK_EMPTY, e5);

    return 0;
}

int test_interpret_next_wrongvar()
{
    program_new();

    SET_TEST_VAR(I, 3);

    program_return_t ret_start;
    ret_start.lineno = 123;
    ret_start.vartoken = TEST_VAR(I);
    program_push_return(&ret_start);

    const char * input = "NEXT J";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_SYNTAX, e2);

    return 0;
}

int test_interpret_next_notvar()
{
    program_new();

    const char * input = "NEXT 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_SYNTAX, e2);

    return 0;
}

int test_tokenize_with_sep()
{
    const char * input = "\"HELLO\",NAME";
    tok_t dst_buf[40];

    error_t e = statement_tokenize(dst_buf, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    ASSERT_EQUAL_UINT(TOK_STRING, dst_buf[0]);
    ASSERT_EQUAL_UINT(6, dst_buf[1]);

    ASSERT_EQUAL_UINT('H', dst_buf[2]);
    ASSERT_EQUAL_UINT('E', dst_buf[3]);
    ASSERT_EQUAL_UINT('L', dst_buf[4]);
    ASSERT_EQUAL_UINT('L', dst_buf[5]);
    ASSERT_EQUAL_UINT('O', dst_buf[6]);
    ASSERT_EQUAL_UINT('\0', dst_buf[7]);

    ASSERT_EQUAL_UINT(TOK_SEPARATOR, dst_buf[8]);

    ASSERT_EQUAL_UINT(TOK_VARIABLE, dst_buf[9]);
    ASSERT_EQUAL_UINT(5, dst_buf[10]);

    ASSERT_EQUAL_UINT('N', dst_buf[11]);
    ASSERT_EQUAL_UINT('A', dst_buf[12]);
    ASSERT_EQUAL_UINT('M', dst_buf[13]);
    ASSERT_EQUAL_UINT('E', dst_buf[14]);
    ASSERT_EQUAL_UINT('\0', dst_buf[15]);

    ASSERT_EQUAL_UINT(TOK_TERMINATOR, dst_buf[16]);

    return 0;
}

int test_interpret_gosub()
{
    program_new();

    const char * input = "GOSUB 500";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    ASSERT_EQUAL_INT(500, program_next_lineno());

    /* Program stack should have the GOSUB return. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e4);

    ASSERT_EQUAL_INT(456, ret1.lineno);
    ASSERT_EQUAL_UINT(NULL, ret1.vartoken); /* No variable associated. */

    /* Only one entry in stack. */
    program_return_t ret2;
    error_t e5 = program_pop_return(&ret2);
    ASSERT_EQUAL_UINT(ERROR_RETSTACK_EMPTY, e5);

    return 0;
}

int test_interpret_gosub_invalid_lineno()
{
    program_new();

    const char * input = "GOSUB FOO";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_SYNTAX, e2);

    return 0;
}

int test_interpret_return()
{
    program_new();

    program_return_t ret_start;
    ret_start.lineno = 123;
    ret_start.vartoken = NULL;
    program_push_return(&ret_start);

    const char * input = "RETURN";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    ASSERT_EQUAL_INT(124, program_next_lineno());

    /* Program stack should be empty. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_RETSTACK_EMPTY, e4);

    return 0;
}

int test_interpret_return_empty_stack()
{
    program_new();

    const char * input = "RETURN";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    program_listing.entries[program_listing.index].lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_RETSTACK_EMPTY, e2);

    return 0;
}

int test_interpret_dim()
{
    program_new();

    const char * input = "DIM X(4)";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    /* Get value of variable. */
    tok_t * a;
    tok_t var[10]; MAKE_VAR_TOK(var, "X");
    error_t e3 = program_get_array(var, &a);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_UINT(TOK_ALLOC, a[0]);
    ASSERT_EQUAL_UINT((unsigned int)(4 * sizeof(numeric_t)), a[1]); /* Double size because it's storing numerics. */

    return 0;
}

int test_interpret_dim14()
{
    program_new();

    const char * input = "DIM ABC(14)";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    /* Get value of variable. */
    tok_t * a;
    tok_t abc[10];
    MAKE_VAR_TOK(abc, "ABC");
    error_t e3 = program_get_array(abc, &a);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_UINT(TOK_ALLOC, a[0]);
    ASSERT_EQUAL_UINT((unsigned int)(14 * sizeof(numeric_t)), a[1]); /* Double size because it's storing numerics. */

    return 0;
}

int test_interpret_dim_toolarge_boundary_ok()
{
    program_new();

    const char * input = "DIM Z(127)";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    /* Get value of variable. */
    tok_t * a;
    tok_t z[10];
    MAKE_VAR_TOK(z, "Z");
    error_t e3 = program_get_array(z, &a);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_UINT(TOK_ALLOC, a[0]);
    ASSERT_EQUAL_UINT((unsigned int)(127 * sizeof(numeric_t)), a[1]); /* Double size because it's storing numerics. */

    return 0;
}

int test_interpret_dim_toolarge_boundary_toolarge()
{
    program_new();

    const char * input = "DIM FOO(128)";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_RANGE, e2);

    /* Variable should be undefined. */
    tok_t * a;
    tok_t foo[10];
    MAKE_VAR_TOK(foo, "FOO");
    error_t e3 = program_get_array(foo, &a);
    ASSERT_EQUAL_UINT(ERROR_UNDEFINED_ARRAY, e3);

    return 0;
}

int test_interpret_dim_toolarge()
{
    program_new();

    const char * input = "DIM Z(1994)";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_RANGE, e2);

    /* Variable should be undefined. */
    tok_t * a;
    tok_t z[10];
    MAKE_VAR_TOK(z, "Z");
    error_t e3 = program_get_array(z, &a);
    ASSERT_EQUAL_UINT(ERROR_UNDEFINED_ARRAY, e3);

    return 0;
}

#define HELPER_TOKENIZE(dst, input) \
    ASSERT_EQUAL_UINT(ERROR_NOERROR, statement_tokenize(dst, input))

#define HELPER_INTERPRET(toks) \
    ASSERT_EQUAL_UINT(ERROR_NOERROR, statement_interpret(toks))

int test_interpret_assign_array()
{
    program_new();
    tok_t arr[10];
    MAKE_VAR_TOK(arr, "ARR");
    program_create_array(arr, 15);

    const char * input = "ARR(5) = 42";
    tok_t dst[80];

    HELPER_TOKENIZE(dst, input);
    HELPER_INTERPRET(dst);

    /* Array index 5 should be set. */
    tok_t * a;
    ASSERT_NO_ERROR(program_get_array(arr, &a));
    ASSERT_EQUAL_INT(42, ARRAY_ACCESS(a, 5));

    return 0;
}

int test_interpret_assign_array_variable_index()
{
    program_new();
    tok_t arr[10];
    MAKE_VAR_TOK(arr, "ARR");
    program_create_array(arr, 15);
    SET_TEST_VAR(IDX, 3);

    const char * input = "ARR(IDX) = 42";
    tok_t dst[80];

    HELPER_TOKENIZE(dst, input);
    HELPER_INTERPRET(dst);

    /* Array index 5 should be set. */
    tok_t * a;
    ASSERT_NO_ERROR(program_get_array(arr, &a));
    ASSERT_EQUAL_INT(42, ARRAY_ACCESS(a, 3));

    return 0;
}

int test_interpret_assign_array_undefined()
{
    program_new();
    SET_TEST_VAR(IDX, 3);

    const char * input = "ARR(IDX) = 42";
    tok_t dst[80];

    HELPER_TOKENIZE(dst, input);
    error_t e = statement_interpret(dst);

    ASSERT_EQUAL_UINT(ERROR_UNDEFINED_ARRAY, e);

    return 0;
}

int test_interpret_if_true()
{
    program_new();
    SET_TEST_VAR(TEST, 42);
    SET_TEST_VAR(FOO, 99);

    const char * input = "IF TEST = 42 THEN FOO = 9";
    tok_t dst[80];

    HELPER_TOKENIZE(dst, input);
    HELPER_INTERPRET(dst);

    numeric_t val;
    ASSERT_EQUAL_UINT(ERROR_NOERROR, program_get_numeric(TEST_VAR(FOO), &val));
    ASSERT_EQUAL_INT(9, val);

    return 0;
}

int test_interpret_if_false()
{
    program_new();
    SET_TEST_VAR(TEST, 100);
    SET_TEST_VAR(FOO, 99);

    const char * input = "IF TEST = 42 THEN FOO = 9";
    tok_t dst[80];

    HELPER_TOKENIZE(dst, input);
    HELPER_INTERPRET(dst);

    numeric_t val;
    ASSERT_EQUAL_UINT(ERROR_NOERROR, program_get_numeric(TEST_VAR(FOO), &val));
    ASSERT_EQUAL_INT(99, val);

    return 0;
}

int test_interpret_if_lteq_true()
{
    program_new();
    SET_TEST_VAR(TEST, 100);
    SET_TEST_VAR(FOO, 99);

    const char * input = "IF TEST <= 104 THEN FOO = 9";
    tok_t dst[80];

    HELPER_TOKENIZE(dst, input);
    HELPER_INTERPRET(dst);

    numeric_t val;
    ASSERT_EQUAL_UINT(ERROR_NOERROR, program_get_numeric(TEST_VAR(FOO), &val));
    ASSERT_EQUAL_INT(9, val);

    return 0;
}

int test_interpret_if_gt_false()
{
    program_new();
    SET_TEST_VAR(TEST, 100);
    SET_TEST_VAR(FOO, 99);

    const char * input = "IF TEST > 100 THEN FOO = 9";
    tok_t dst[80];

    HELPER_TOKENIZE(dst, input);
    HELPER_INTERPRET(dst);

    numeric_t val;
    ASSERT_EQUAL_UINT(ERROR_NOERROR, program_get_numeric(TEST_VAR(FOO), &val));
    ASSERT_EQUAL_INT(99, val);

    return 0;
}

int test_tokenize_and_interpret_rem()
{
    program_new();
    
    const char * input = "REM This is a comment";
    tok_t dst[80];

    HELPER_TOKENIZE(dst, input);
    
    ASSERT_EQUAL_UINT(TOK_REMARK, dst[0]);

    HELPER_INTERPRET(dst);

    return 0;
}

int test_tokenize_long_print()
{
    program_new();

    const char * input = "PRINT \"A = { \", A(1), \" \", A(2), \" \", A(3), \" \", A(4), \" \", A(5), \" \", A(6), \" \", A(7), \" \", A(8), \" \", A(9), \" \", A(10), \" }\"";
    tok_t dst[256];

    HELPER_TOKENIZE(dst, input);
    const tok_t * toks = dst;

    /* Check first token is PRINT. */
    ASSERT(KW_CHECK(toks, KEYWORD_PRINT));
    toks += KW_SIZE;

    /* Then "A = {" */
    ASSERT_EQUAL_UINT(TOK_STRING, *toks);
    ASSERT_EQUAL_UINT(7, *(toks+1));
    ASSERT(strcmp("A = { ", (const char *)(toks+2)) == 0);
    toks = t_varlen_skip(toks);

    /* Then 1-9 times: SEP, A, (, NUMERIC, ), SEP, " " */
    for (numeric_t i = 1; i <= 9; i++)
    {
        ASSERT_EQUAL_UINT(TOK_SEPARATOR, *toks);
        toks++;

        ASSERT_EQUAL_UINT(TOK_REGISTER, *toks);
        ASSERT_EQUAL_UINT(0, *(toks+1));
        toks += 2;

        ASSERT(OP_CHECK(toks, OP_LPAREN));
        toks += OP_SIZE;

        ASSERT_EQUAL_INT(i, NUMERIC_GET(toks));
        toks += NUMERIC_SIZE;

        ASSERT(OP_CHECK(toks, OP_RPAREN));
        toks += OP_SIZE;

        ASSERT_EQUAL_UINT(TOK_SEPARATOR, *toks);
        toks++;

        ASSERT_EQUAL_UINT(TOK_STRING, *toks);
        ASSERT_EQUAL_UINT(2, *(toks+1));
        ASSERT(strcmp(" ", (const char *)(toks+2)) == 0);
        toks = t_varlen_skip(toks);
    }

    /* Then: SEP, A, (, NUMERIC, ), SEP, " }" */
    ASSERT_EQUAL_UINT(TOK_SEPARATOR, *toks);
    toks++;
    
    ASSERT_EQUAL_UINT(TOK_REGISTER, *toks);
    ASSERT_EQUAL_UINT(0, *(toks+1));
    toks += 2;

    ASSERT(OP_CHECK(toks, OP_LPAREN));
    toks += OP_SIZE;

    ASSERT_EQUAL_INT(10, NUMERIC_GET(toks));
    toks += NUMERIC_SIZE;

    ASSERT(OP_CHECK(toks, OP_RPAREN));
    toks += OP_SIZE;

    ASSERT_EQUAL_UINT(TOK_SEPARATOR, *toks);
    toks++;

    ASSERT_EQUAL_UINT(TOK_STRING, *toks);
    ASSERT_EQUAL_UINT(3, *(toks+1));
    ASSERT(strcmp(" }", (const char *)(toks+2)) == 0);
    toks = t_varlen_skip(toks);

    /* Now expect terminator token. */
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, *toks);
    toks++;

    tok_size_t actual_size = (tok_size_t)(toks - dst);
    ASSERT_EQUAL_UINT(actual_size, statement_size(dst));

    return 0;
}