#ifndef _STATEMENT_H
#define _STATEMENT_H

#include "errors.h"

/* statement_tokenize
 *
 * Purpose:
 *     Parse a statement into a sequence of tokens.
 * 
 * Parameters:
 *     stmt:  Buffer for generated token stream
 *     input: Buffer containing input to be tokenized
 * 
 * Returns:
 *     error, if any.
 */
error_t statement_tokenize(tok_t * stmt, char * input);

/* statement_interpret
 *
 * Purpose:
 *     Interpret the given token stream.
 * 
 * Parameters:
 *     stmt: Token stream to interpret.
 * 
 * Returns:
 *     error, if any.
 */
error_t statement_interpret(const tok_t * stmt);

tok_size_t statement_size(const tok_t * stmt);

#endif
