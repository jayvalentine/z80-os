#include "test.h"

#include "statement.h"
#include "t_defs.h"
#include "t_keyword.h"

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
