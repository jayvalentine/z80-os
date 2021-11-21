#ifndef _T_DEFS_H
#define _T_DEFS_H

#include <stdint.h>

#define NUM_TOKS 7

#define TOK_TERMINATOR 0x00
#define TOK_STRING 0x01
#define TOK_KEYWORD 0x02
#define TOK_NUMERIC 0x03
#define TOK_OPERATOR 0x04
#define TOK_VARIABLE 0x05
#define TOK_SEPARATOR 0x06

/* Type representing a token in a program. */
typedef uint8_t tok_t;

/* Type representing the size of a sequence of tokens in a type. */
typedef uint8_t tok_size_t;

typedef const tok_t * (*t_list_t)(const tok_t *);
typedef tok_t (*t_size_t)(const tok_t *);

const tok_t * t_defs_list(const tok_t * toks);
tok_size_t t_defs_size(const tok_t * toks);

/* Parse a separator from the input stream. */
int t_sep_parse(tok_t ** dst_ptr, const char ** input_ptr);

#endif
