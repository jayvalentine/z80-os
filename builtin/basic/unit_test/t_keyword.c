#include "test.h"

#include "t_keyword.h"
#include "t_defs.h"

int test_keyword_parse_for()
{
    const char * input = "FOR ";
    const char * orig_input = input;

    tok_t dst_buf[2];
    
    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    int success = t_keyword_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_UINT(TOK_KEYWORD, dst_buf[0]);
    ASSERT_EQUAL_UINT(KEYWORD_FOR, dst_buf[1]);

    ASSERT(dst == orig_dst+2);
    ASSERT(input == orig_input+3);

    return 0;
}

int test_keyword_parse_to()
{
    const char * input = "TO";
    const char * orig_input = input;

    tok_t dst_buf[2];
    
    tok_t * dst = dst_buf;
    tok_t * orig_dst = dst;

    int success = t_keyword_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_UINT(TOK_KEYWORD, dst_buf[0]);
    ASSERT_EQUAL_UINT(KEYWORD_TO, dst_buf[1]);

    ASSERT(dst == orig_dst+2);
    ASSERT(input == orig_input+2);

    return 0;
}
