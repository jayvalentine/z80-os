#include "test.h"

#include "t_defs.h"

#include <string.h>

int test_parse_sep()
{
    const char * input = ", ";
    const char * orig_input = input;

    tok_t dst_buf[40];
    
    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    int success = t_sep_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_UINT(TOK_SEPARATOR, dst_buf[0]);

    ASSERT_EQUAL_UINT(1, t_defs_size(orig_dst));

    ASSERT(dst == orig_dst+1);
    ASSERT(input == orig_input+1);

    return 0;
}

int test_parse_rem()
{
    const char * input = "REM This is a remark";
    
    tok_t dst_buf[40];
    tok_t * dst = dst_buf;

    int success = t_rem_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_UINT(TOK_REMARK, dst_buf[0]);
    ASSERT_EQUAL_UINT(17, dst_buf[1]);

    char s[40];
    memcpy(s, &dst_buf[2], 17);
    s[17] = '\0';
    int cmp = strcmp(" This is a remark", s);
    ASSERT_EQUAL_INT(0, cmp);
}
