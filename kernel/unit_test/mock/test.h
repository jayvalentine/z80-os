#include <mock.h>

extern int failed_line;

#include <stdio.h>

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

#define ASSERT_EQUAL_STRING(expected, actual) do           \
{                                                       \
    failed_line = __LINE__;                             \
    if (strcmp(expected, actual) != 0)                  \
    {                                                   \
        printf("\nexpected: %s actual: %s",             \
            expected,                                   \
            actual);                                    \
        return 1;                                       \
    }                                                   \
} while (0)
