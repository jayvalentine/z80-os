#ifndef _PROGRAM_H
#define _PROGRAM_H

#include <stdint.h>

#include "errors.h"
#include "t_defs.h"
#include "t_numeric.h"
#include "t_variable.h"

/* Type of an entry in the program return stack.
 * lineno: Line number to return to.
 * varname: Associated variable (first character '\0' if N/A).
 */
typedef struct _PROGRAM_RETURN_T
{
    numeric_t lineno;
    char varname[VARNAME_BUF_SIZE];
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
int program_current_lineno(void);

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
void program_set_next_lineno(int lineno);

/* program_set_numeric
 *
 * Purpose:
 *     Set a numeric variable.
 * 
 * Parameters:
 *     name: Name of the variable.
 *     val:  Value of the variable.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_set_numeric(const char * name, numeric_t val);

/* program_get_numeric
 *
 * Purpose:
 *     Get the value of a numeric variable.
 * 
 * Parameters:
 *     name: Name of the variable.
 *     val:  Reference value of the variable.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_get_numeric(const char * name, numeric_t * val);

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
error_t program_push_return(program_return_t * ret);

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

#endif
