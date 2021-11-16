#include "eval.h"

#include "t_numeric.h"

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
error_t eval_numeric(tok_t * dst, const tok_t * src)
{
    *dst = TOK_NUMERIC;
    dst++;
    numeric_t * num_ptr = (numeric_t *)dst;
    *num_ptr = 123;

    return ERROR_NOERROR;
}