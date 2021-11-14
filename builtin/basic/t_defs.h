#ifndef _T_DEFS_H
#define _T_DEFS_H

#include <stdint.h>

#define NUM_TOKS 4

#define TOK_TERMINATOR 0x00
#define TOK_STRING 0x01
#define TOK_KEYWORD 0x02
#define TOK_NUMERIC 0x03

typedef const uint8_t * (*t_list_t)(const uint8_t *);
typedef uint8_t (*t_size_t)(const uint8_t *);

uint8_t * t_defs_list(const uint8_t * toks);
uint8_t t_defs_size(const uint8_t * toks);

#endif
