#include "test.h"

#include "t_variable.h"
#include "t_defs.h"

#include <string.h>

int test_variable_parse_c()
{
    const char * input = "C";
    const char * orig_input = input;

    tok_t dst_buf[4];

    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    int success = t_variable_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_REGISTER, dst_buf[0]);
    ASSERT_EQUAL_INT(2, dst_buf[1]);

    ASSERT(dst == orig_dst+2);
    ASSERT(input == orig_input+1);

    return 0;
}

int test_variable_parse_var()
{
    const char * input = "VAR";
    const char * orig_input = input;

    tok_t dst_buf[6];

    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    int success = t_variable_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_VARIABLE, dst_buf[0]);
    ASSERT_EQUAL_INT(4, dst_buf[1]);
    ASSERT_EQUAL_INT('V', dst_buf[2]);
    ASSERT_EQUAL_INT('A', dst_buf[3]);
    ASSERT_EQUAL_INT('R', dst_buf[4]);
    ASSERT_EQUAL_INT('\0', dst_buf[5]);

    ASSERT(dst == orig_dst+6);
    ASSERT(input == orig_input+3);

    return 0;
}

int test_variable_parse_something()
{
    const char * input = "SOMETHING";
    const char * orig_input = input;

    tok_t dst_buf[11];
    
    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    int success = t_variable_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_VARIABLE, dst_buf[0]);
    ASSERT_EQUAL_INT(5, dst_buf[1]);
    ASSERT_EQUAL_INT('S', dst_buf[2]);
    ASSERT_EQUAL_INT('O', dst_buf[3]);
    ASSERT_EQUAL_INT('M', dst_buf[4]);
    ASSERT_EQUAL_INT('E', dst_buf[5]);
    ASSERT_EQUAL_INT('\0', dst_buf[7]);

    ASSERT(dst == orig_dst+7);
    ASSERT(input == orig_input+9);

    return 0;
}

int test_variable_get_some()
{
    const char * input = "SOMETHING";
    const char * orig_input = input;

    tok_t dst_buf[11];
    
    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    int success = t_variable_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);

    const char * varname = VARIABLE_GET(orig_dst);
    ASSERT(strcmp("SOME", varname) == 0);

    ASSERT_EQUAL_UINT(orig_input+9, input);
    ASSERT_EQUAL_UINT(orig_dst+7, dst);

    return 0;
}

int test_variable_get_i()
{
    const char * input = "I";
    const char * orig_input = input;

    tok_t dst_buf[11];
    
    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    int success = t_variable_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);

    ASSERT_EQUAL_UINT(TOK_REGISTER, dst_buf[0]);
    ASSERT_EQUAL_UINT('I' - 'A', dst_buf[1]);

    ASSERT_EQUAL_UINT(orig_input+1, input);
    ASSERT_EQUAL_UINT(orig_dst+2, dst);

    return 0;
}
