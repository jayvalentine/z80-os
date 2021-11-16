#ifndef _EVAL_H
#define _EVAL_H

#include "errors.h"

#include "t_defs.h"

/* eval_numeric
 *
 * Purpose:
 *     Attempts to evaluate the expression given
 *     by the tokens at src and write the result
 *     as a numeric token to dst.
 * 
 * Parameters:
 *     dst: Destination for result token.
 *     src: Expression tokens to evaluate.
 * 
 * Returns:
 *     Error, if any.
 */
error_t eval_numeric(tok_t * dst, const tok_t * src);

#endif
