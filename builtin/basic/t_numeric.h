#ifndef _T_NUMERIC_H
#define _T_NUMERIC_H

#include <stdint.h>

#include "t_defs.h"

/* t_numeric_get
 *
 * Purpose:
 *     Return a numeric (int) from the given token stream.
 * 
 * Parameters:
 *     toks: Token stream to read from.
 * 
 * Returns:
 *     integer value of numeric.
 */
int t_numeric_get(uint8_t * toks);

/* t_numeric_parse
 *
 * Purpose:
 *     Parse a numeric in the given input into the token stream.
 *
 * Parameters:
 *     stmt:  Token stream to write to.
 *     input: Input from which to parse numeric.
 * 
 * Returns:
 *     true value if a numeric has been parsed, false otherwise.
 */
int t_numeric_parse(uint8_t ** dst_ptr, const char ** input_ptr);

const uint8_t * t_numeric_list(const uint8_t * toks);

#endif
