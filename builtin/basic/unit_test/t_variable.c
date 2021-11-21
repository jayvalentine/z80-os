#include "test.h"

#include "t_variable.h"
#include "t_defs.h"

#include <string.h>

int test_variable_parse_c()
{
    const char * input = "C";
    const char * orig_input = input;

    tok_t dst_buf[3];

    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    int success = t_variable_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_VARIABLE, dst_buf[0]);
    ASSERT_EQUAL_INT(1, dst_buf[1]);
    ASSERT_EQUAL_INT('C', dst_buf[2]);

    ASSERT(dst == orig_dst+3);
    ASSERT(input == orig_input+1);

    return 0;
}

int test_variable_parse_var()
{
    const char * input = "VAR";
    const char * orig_input = input;

    tok_t dst_buf[5];

    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    int success = t_variable_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_VARIABLE, dst_buf[0]);
    ASSERT_EQUAL_INT(3, dst_buf[1]);
    ASSERT_EQUAL_INT('V', dst_buf[2]);
    ASSERT_EQUAL_INT('A', dst_buf[3]);
    ASSERT_EQUAL_INT('R', dst_buf[4]);

    ASSERT(dst == orig_dst+5);
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
    ASSERT_EQUAL_INT(4, dst_buf[1]);
    ASSERT_EQUAL_INT('S', dst_buf[2]);
    ASSERT_EQUAL_INT('O', dst_buf[3]);
    ASSERT_EQUAL_INT('M', dst_buf[4]);
    ASSERT_EQUAL_INT('E', dst_buf[5]);

    ASSERT(dst == orig_dst+6);
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

    char varname[VARNAME_BUF_SIZE];
    t_variable_get(varname, orig_dst+1);
    ASSERT(strcmp("SOME", varname) == 0);

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

    char varname[VARNAME_BUF_SIZE];
    t_variable_get(varname, orig_dst+1);
    ASSERT(strcmp("I", varname) == 0);

    return 0;
}
