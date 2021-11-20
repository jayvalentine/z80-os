#ifndef _EVAL_H
#define _EVAL_H

#include "errors.h"

#include "t_defs.h"
#include "t_numeric.h"

/* eval_numeric
 *
 * Purpose:
 *     Attempts to evaluate the expression given
 *     by the tokens at src and provides the result.
 * 
 * Parameters:
 *     result: Destination for result value
 *     src:    Expression tokens to evaluate.
 * 
 * Returns:
 *     Error, if any.
 */
error_t eval_numeric(numeric_t * result, const tok_t * src);

#endif
