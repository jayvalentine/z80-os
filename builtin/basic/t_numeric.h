#ifndef _T_NUMERIC_H
#define _T_NUMERIC_H

#include <stdint.h>

#include "t_defs.h"

typedef int16_t numeric_t;

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
numeric_t t_numeric_get(const tok_t * toks);

/* t_numeric_put
 *
 * Purpose:
 *     Write a numeric (int) to the given token stream.
 * 
 * Parameters:
 *     toks: Token stream to write to.
 *     num:  Numeric value to write.
 * 
 * Returns:
 *     Nothing.
 */
void t_numeric_put(tok_t * toks, numeric_t num);

/* t_numeric_size
 * Purpose:
 *     Return the size of a numeric in memory.
 * 
 * Parameters:
 *     toks: Token stream to read from.
 * 
 * Returns:
 *     unsigned size of numeric tokens (2).
 */

tok_size_t t_numeric_size(const tok_t * toks);

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
int t_numeric_parse(tok_t ** dst_ptr, const char ** input_ptr);

const tok_t * t_numeric_list(const tok_t * toks);

#endif
