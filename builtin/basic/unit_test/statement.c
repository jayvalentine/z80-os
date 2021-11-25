#include "test.h"

#include "statement.h"
#include "program.h"

#include "t_defs.h"
#include "t_keyword.h"

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
}

int test_tokenize_variable()
{
    const char * input = "C";
    tok_t dst[10];

    error_t e = statement_tokenize(dst, input);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    ASSERT_EQUAL_UINT(TOK_VARIABLE, dst[0]);
    ASSERT_EQUAL_UINT(1, dst[1]);
    ASSERT_EQUAL_UINT('C', dst[2]);
    ASSERT_EQUAL_UINT(TOK_TERMINATOR, dst[3]);
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
    error_t e3 = program_get_numeric("C", &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(3, val);
}

int test_interpret_assignment_from_self()
{
    program_new();

    const char * input = "AM = AM - 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    /* Set up the context. */
    program_set_numeric("AM", 9);

    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    error_t e3 = program_get_numeric("AM", &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(5, val);
}

int test_interpret_assignment_from_self_equal()
{
    program_new();

    const char * input = "AM = AM - AM + AM";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    /* Set up the context. */
    program_set_numeric("AM", 9);

    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    error_t e3 = program_get_numeric("AM", &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(9, val);
}

extern numeric_t current_lineno;

int test_interpret_for_entry()
{
    program_new();

    program_return_t ret_start;
    ret_start.lineno = 123;
    strcpy(ret_start.varname, "VAR");

    program_push_return(&ret_start);

    const char * input = "FOR I=1 TO 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    current_lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    error_t e3 = program_get_numeric("I", &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(1, val);

    /* Program stack should have the for added. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e4);

    ASSERT_EQUAL_INT(456, ret1.lineno);
    ASSERT(strcmp("I", ret1.varname) == 0);

    /* Program stack should have the for added. */
    program_return_t ret2;
    error_t e5 = program_pop_return(&ret2);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e5);

    ASSERT_EQUAL_INT(123, ret2.lineno);
    ASSERT(strcmp("VAR", ret2.varname) == 0);

    return 0;
}

int test_interpret_for_continue()
{
    program_new();
    program_set_numeric("I", 1);

    program_return_t ret_start;
    ret_start.lineno = 210;
    strcpy(ret_start.varname, "I");

    program_push_return(&ret_start);

    const char * input = "FOR I=1 TO 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    current_lineno = 130;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    error_t e3 = program_get_numeric("I", &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(2, val);

    /* Program stack should have the for added. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e4);

    ASSERT_EQUAL_INT(130, ret1.lineno);
    ASSERT(strcmp("I", ret1.varname) == 0);

    return 0;
}

int test_interpret_for_hit_limit()
{
    program_new();
    program_set_numeric("I", 3);

    program_return_t ret_start;
    ret_start.lineno = 210;
    strcpy(ret_start.varname, "I");

    program_push_return(&ret_start);

    const char * input = "FOR I=1 TO 4";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    current_lineno = 130;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    error_t e3 = program_get_numeric("I", &val);
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

    current_lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    numeric_t val;
    error_t e3 = program_get_numeric("I", &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(1, val);

    /* Program stack should have the for added. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e4);

    ASSERT_EQUAL_INT(456, ret1.lineno);
    ASSERT(strcmp("I", ret1.varname) == 0);

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

    current_lineno = 456;
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

    current_lineno = 456;
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

    current_lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_SYNTAX, e2);

    return 0;
}

int test_interpret_next()
{
    program_new();

    program_set_numeric("I", 3);

    program_return_t ret_start;
    ret_start.lineno = 123;
    strcpy(ret_start.varname, "I");
    program_push_return(&ret_start);

    const char * input = "NEXT I";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    current_lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    ASSERT_EQUAL_INT(123, program_next_lineno());

    numeric_t val;
    error_t e3 = program_get_numeric("I", &val);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e3);

    ASSERT_EQUAL_INT(3, val);

    /* Program stack should have the for added. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e4);

    ASSERT_EQUAL_INT(456, ret1.lineno);
    ASSERT(strcmp("I", ret1.varname) == 0);

    /* Only one entry in stack. */
    program_return_t ret2;
    error_t e5 = program_pop_return(&ret2);
    ASSERT_EQUAL_UINT(ERROR_RETSTACK_EMPTY, e5);

    return 0;
}

int test_interpret_next_wrongvar()
{
    program_new();

    program_set_numeric("I", 3);

    program_return_t ret_start;
    ret_start.lineno = 123;
    strcpy(ret_start.varname, "I");
    program_push_return(&ret_start);

    const char * input = "NEXT J";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    current_lineno = 456;
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

    current_lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_SYNTAX, e2);

    return 0;
}

int test_tokenize_with_sep()
{
    const char * input = "\"HELLO\",NAME";
    const char * orig_input = input;

    tok_t dst_buf[40];
    
    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    error_t e = statement_tokenize(dst_buf, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    ASSERT_EQUAL_UINT(TOK_STRING, dst_buf[0]);
    ASSERT_EQUAL_UINT(5, dst_buf[1]);

    ASSERT_EQUAL_UINT('H', dst_buf[2]);
    ASSERT_EQUAL_UINT('E', dst_buf[3]);
    ASSERT_EQUAL_UINT('L', dst_buf[4]);
    ASSERT_EQUAL_UINT('L', dst_buf[5]);
    ASSERT_EQUAL_UINT('O', dst_buf[6]);

    ASSERT_EQUAL_UINT(TOK_SEPARATOR, dst_buf[7]);

    ASSERT_EQUAL_UINT(TOK_VARIABLE, dst_buf[8]);
    ASSERT_EQUAL_UINT(4, dst_buf[9]);

    ASSERT_EQUAL_UINT('N', dst_buf[10]);
    ASSERT_EQUAL_UINT('A', dst_buf[11]);
    ASSERT_EQUAL_UINT('M', dst_buf[12]);
    ASSERT_EQUAL_UINT('E', dst_buf[13]);

    ASSERT_EQUAL_UINT(TOK_TERMINATOR, dst_buf[14]);

    return 0;
}

int test_interpret_gosub()
{
    program_new();

    const char * input = "GOSUB 500";
    tok_t dst[80];

    error_t e = statement_tokenize(dst, input);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e);

    current_lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e2);

    ASSERT_EQUAL_INT(500, program_next_lineno());

    /* Program stack should have the GOSUB return. */
    program_return_t ret1;
    error_t e4 = program_pop_return(&ret1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, e4);

    ASSERT_EQUAL_INT(456, ret1.lineno);
    ASSERT(strcmp("", ret1.varname) == 0);

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

    current_lineno = 456;
    error_t e2 = statement_interpret(dst);
    ASSERT_EQUAL_UINT(ERROR_SYNTAX, e2);

    return 0;
}
