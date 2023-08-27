#ifndef _T_STRING_H
#define _T_STRING_H

#include <stddef.h>
#include <stdint.h>

#include "t_defs.h"

#define STRING_GET(_toks) ((const char *)(_toks+2))

/* t_string_parse
 *
 * Purpose:
 *     Parse a string in the given input into the token stream.
 *
 * Parameters:
 *     stmt:  Token stream to write to.
 *     input: Input from which to parse string.
 * 
 * Returns:
 *     Pointer to the next byte after the tokenized string
 *     in the token stream.
 */
int t_string_parse(tok_t ** stmt, const char ** input);

const tok_t * t_string_list(const tok_t * toks);

#endif
