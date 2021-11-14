#ifndef _KEYWORD_H
#define _KEYWORD_H

#include <stdint.h>

#include "errors.h"

#define NUM_KEYWORDS 2

#define KEYWORD_UNDEFINED 0

#define KEYWORD_BASE 1
#define KEYWORD_PRINT 1

typedef uint8_t kw_code;

typedef error_t (*f_interpreter_t)(const uint8_t *);

typedef struct _Keyword_T
{
    const char * str;
    kw_code code;
} Keyword_T;

int t_keyword_parse(uint8_t ** dst, const char ** s);

error_t t_keyword_interpret(kw_code kw, const uint8_t * toks);

#endif
