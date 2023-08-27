#ifndef _KEYWORD_H
#define _KEYWORD_H

#include <stdint.h>

#include "errors.h"

#include "t_defs.h"

#define NUM_KEYWORDS 14

#define KEYWORD_UNDEFINED 0

#define KEYWORD_BASE 1

#define KEYWORD_PRINT 1
#define KEYWORD_LIST 2
#define KEYWORD_NEW 3
#define KEYWORD_RUN 4
#define KEYWORD_END 5
#define KEYWORD_GOTO 6
#define KEYWORD_FOR 7
#define KEYWORD_TO 8
#define KEYWORD_NEXT 9
#define KEYWORD_GOSUB 10
#define KEYWORD_RETURN 11
#define KEYWORD_DIM 12
#define KEYWORD_IF 13
#define KEYWORD_THEN 14

#define KW_CHECK(_toks, _kw_type) ((*_toks == TOK_KEYWORD) && (*(_toks+1) == _kw_type))
#define KW_SIZE 2

typedef uint8_t kw_code;

typedef error_t (*f_interpreter_t)(const tok_t *);

typedef struct _Keyword_T
{
    const char * str;
    kw_code code;
} Keyword_T;

int t_keyword_parse(tok_t ** dst, const char ** s);
const tok_t * t_keyword_list(const tok_t * toks);

error_t t_keyword_interpret(kw_code kw, const tok_t * toks);

#endif
