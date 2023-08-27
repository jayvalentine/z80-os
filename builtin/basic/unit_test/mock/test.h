extern int failed_line;

#include <stdio.h>
#include <string.h>

#define MAKE_VAR_TOK(_buf, _name) \
    if (strlen(_name) == 1) \
    { \
        _buf[0] = TOK_REGISTER; \
        _buf[1] = _name[0] - 'A'; \
    } \
    else \
    { \
        _buf[0] = TOK_VARIABLE; \
        _buf[1] = strlen(_name) + 1; \
        strcpy((char *)&_buf[2], _name); \
    }


#define SET_TEST_VAR(_name, _val) \
    tok_t tmpbuf_ ## _name[20]; \
    MAKE_VAR_TOK(tmpbuf_ ## _name, #_name); \
    program_set_numeric(tmpbuf_ ## _name, _val)

#define TEST_VAR(_name) tmpbuf_ ## _name

#define ASSERT(cond) do         \
{                               \
    failed_line = __LINE__;     \
    if (!(cond)) return 1;      \
} while (0)

#define ASSERT_EQUAL_INT(expected, actual) do           \
{                                                       \
    failed_line = __LINE__;                             \
    if (expected != actual)                             \
    {                                                   \
        printf("\nexpected: %d actual: %d",             \
            expected,                                   \
            actual);                                    \
        return 1;                                       \
    }                                                   \
} while (0)

#define ASSERT_EQUAL_UINT(expected, actual) do           \
{                                                       \
    failed_line = __LINE__;                             \
    if (expected != actual)                             \
    {                                                   \
        printf("\nexpected: %u actual: %u",             \
            expected,                                   \
            actual);                                    \
        return 1;                                       \
    }                                                   \
} while (0)

#define ASSERT_EQUAL_PTR(expected, actual) do           \
{                                                       \
    failed_line = __LINE__;                             \
    if (expected != actual)                             \
    {                                                   \
        printf("\nexpected: %p actual: %p",             \
            expected,                                   \
            actual);                                    \
        return 1;                                       \
    }                                                   \
} while (0)

#define ASSERT_NO_ERROR(e) ASSERT_EQUAL_UINT(ERROR_NOERROR, e)
