#include <mock.h>

#define ASSERT(cond) do         \
{                               \
    failed_line = __LINE__;     \
    if (!(cond)) return 1;      \
} while (0)
