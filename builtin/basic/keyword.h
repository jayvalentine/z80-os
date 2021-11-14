#ifndef _KEYWORD_H
#define _KEYWORD_H

#include <stdint.h>

#define NUM_KEYWORDS 1

#define KEYWORD_UNDEFINED 0
#define KEYWORD_PRINT 128

typedef uint8_t kw_code; 

typedef struct _Keyword_T
{
    const char * str;
    kw_code code;
} Keyword_T;

kw_code keyword_parse(const char * s);

#endif
