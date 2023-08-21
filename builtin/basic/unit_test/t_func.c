#include "test.h"

#include "t_func.h"

int test_fnc_parse_rnd()
{
    const char * input_orig = "RND";
    const char * input = input_orig;

    tok_t dst_buf[2];
    tok_t * dst = dst_buf;

    int success = t_func_parse(&dst, &input);
    ASSERT_EQUAL_INT(1, success);
    ASSERT_EQUAL_INT(TOK_FUNC, dst_buf[0]);
    ASSERT_EQUAL_INT(FUNC_RND, dst_buf[1]);

    ASSERT_EQUAL_PTR(dst_buf+2, dst);
    ASSERT_EQUAL_PTR(input_orig+3, input);

    return 0;
}
