#ifndef _PROGRAM_H
#define _PROGRAM_H

#include <stdint.h>

#include "errors.h"
#include "t_defs.h"
#include "t_numeric.h"
#include "t_variable.h"

#ifdef Z80
#define TGT_FASTCALL __z88dk_fastcall
#else
#define TGT_FASTCALL
#endif

/* Type of an entry in the program return stack.
 * lineno:   Line number to return to.
 * vartoken: Token for associated variable (NULL if N/A).
 *           Must have program lifetime.
 */
typedef struct _PROGRAM_RETURN_T
{
    numeric_t lineno;
    const tok_t * vartoken;
} program_return_t;

/* program_end
 *
 * Purpose:
 *     End the currently-running program.
 * 
 * Parameters:
 *     Error, if program has been ended with error.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_end(error_t error);

uint16_t program_free(void);

/* program_new
 *
 * Purpose:
 *     Create a new program.
 * 
 * Parameters:
 *     None.
 * 
 * Returns:
 *     None.
 */
void program_new(void);

error_t program_insert(const tok_t * toks);

void program_list(void);

error_t program_run(void);

/* program_current_lineno
 *
 * Purpose:
 *     Get the current line number of the executing program.
 *
 * Parameters:
 *     Nothing.
 *
 * Returns:
 *     Integer line number.
 */
numeric_t program_current_lineno(void);

/* program_next_lineno
 *
 * Purpose:
 *     Get the next line number to be executed.
 *     Mainly used for error reporting.
 * 
 * Parameters:
 *     Nothing.
 * 
 * Returns:
 *     Integer line number.
 */
numeric_t program_next_lineno(void);

/* program_set_next_lineno
 *
 * Purpose:
 *     Set the next line number to be executed.
 * 
 * Parameters:
 *     Integer line number.
 * 
 * Returns:
 *     Nothing.
 */
void program_set_next_lineno(numeric_t lineno);

/* program_set_numeric
 *
 * Purpose:
 *     Set a numeric variable.
 * 
 * Parameters:
 *     toks: Pointer to variable token.
 *     val:  Value of the variable.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_set_numeric(const tok_t * toks, numeric_t val);

/* program_get_numeric
 *
 * Purpose:
 *     Get the value of a numeric variable.
 * 
 * Parameters:
 *     toks: Pointer to variable token.
 *     val:  Reference value of the variable.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_get_numeric(const tok_t * toks, numeric_t * val);

/* program_get_numeric_ref
 *
 * Purpose:
 *     Get a reference to a numeric variable.
 * 
 * Parameters:
 *     toks: Pointer to variable token.
 * 
 * Returns:
 *     Pointer to numeric variable, or NULL if undefined.
 */
numeric_t * program_get_numeric_ref(const tok_t * toks) TGT_FASTCALL;

/* program_create_array
 *
 * Purpose:
 *     Create a new array for the given variable.
 * 
 * Parameters:
 *     toks: Pointer to variable token.
 *     size: Size of array.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_create_array(const tok_t * toks, tok_size_t size);

/* program_get_array
 *
 * Purpose:
 *     Get the value (pointer to) an array variable.
 * 
 * Parameters:
 *     toks: Pointer to variable token.
 *     val: Reference value of the array.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_get_array(const tok_t * toks, tok_t ** array);

/* program_push_return
 *
 * Purpose:
 *     Push a value onto the program's return stack.
 * 
 * Parameters:
 *     ret: Reference to program_return_t object.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_push_return(const program_return_t * ret);

/* program_pop_return
 *
 * Purpose:
 *     Pop a value from the program's return stack.
 * 
 * Parameters:
 *     ret: Reference to program_return_t object to populate.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_pop_return(program_return_t * ret);

/* program_alloc
 *
 * Purpose:
 *     Allocates space in the program region for some
 *     data.
 * 
 * Parameters:
 *     size: Size of data region to allocate.
 * 
 * Returns:
 *     Pointer to allocation object:
 *     <TOK_ALLOC> <size> <allocated space>
 */
tok_t * program_alloc(tok_size_t size);

#endif
