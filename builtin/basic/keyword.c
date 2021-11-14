#include <string.h>

#include "keyword.h"

const Keyword_T keywords[NUM_KEYWORDS] =
{
    {
        "PRINT",
        KEYWORD_PRINT
    }
};

kw_code keyword_parse(const char * s)
{
    for (int i = 0; i < NUM_KEYWORDS; i++)
    {
        if (strcmp(s, keywords[i].str) == 0)
        {
            return keywords[i].code;
        }
    }

    /* Keyword not defined. */
    return KEYWORD_UNDEFINED;
}
