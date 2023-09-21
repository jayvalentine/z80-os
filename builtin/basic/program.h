#ifndef _PROGRAM_H
#define _PROGRAM_H

#include <stdint.h>

#include "errors.h"
#include "t_defs.h"
#include "t_numeric.h"
#include "t_variable.h"

#ifdef Z80
#include <os.h>

extern tok_t end;

#define program_start (&end)
#define program_max ((tok_t *)USER_PROG_END)

#else
#define TGT_FASTCALL

extern tok_t program_region[4096];

#define program_start (&program_region[0])
#define program_max (program_start + 4096)
#endif

/* If pointers are two bytes wide on the target,
 * then a line pointer can be the pointer directly.
 *
 * Otherwise, it must be an offset from some program start region.
 * Either way, 64KB of program space is addressable.
 */
#if UINTPTR_MAX == 0xffff
typedef const tok_t * lineptr_t;

#define GET_LINE(_lineptr) _lineptr
#define GET_LINEPTR(_line) _line

#else
extern tok_t * program;
typedef uint16_t lineptr_t;

#define GET_LINE(_lineptr) (_lineptr == 0xffff ? NULL: (program_start + _lineptr))
#define GET_LINEPTR(_line) (_line == NULL ? 0xffff : ((uint16_t)(_line - program_start)))
#endif

/* Lines in the program are split into the following components: 
 *
 * 2 bytes:  lineno
 * 2 bytes:  previous line pointer
 * 2 bytes:  next line pointer
 * variable: statement
 */

/* Macros for accessing program line contents. */
#define PROG_HDR_SIZE (sizeof(numeric_t) + (sizeof(tok_t *) * 2))

#define PROG_LINENO(_progline) (*(numeric_t *)_progline)
#define PROG_PREV(_progline) (*(tok_t **)(_progline + sizeof(numeric_t)))
#define PROG_NEXT(_progline) (*(tok_t **)(_progline + sizeof(numeric_t) + sizeof(tok_t *)))
#define PROG_STMT(_progline) ((tok_t *)(_progline + PROG_HDR_SIZE))

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

lineptr_t program_get_current_line(void);

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

/* program_transfer_control
 *
 * Purpose:
 *     Set the next line to be executed.
 * 
 * Parameters:
 *     Integer line number.
 * 
 * Returns:
 *     Line pointer.
 */
lineptr_t program_transfer_control(numeric_t lineno);

/* program_transfer_control
 *
 * Purpose:
 *     Set the next line to be executed.
 * 
 * Parameters:
 *     Pointer to line.
 * 
 * Returns:
 *     Nothing.
 */
void program_transfer_control_direct(lineptr_t line);

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
numeric_t * program_get_numeric_ref(const tok_t * toks);

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
 *     Push a line onto the program's return stack.
 * 
 * Parameters:
 *     line:   Line pointer
 *     vartok: Variable token
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_push_return(lineptr_t line, const tok_t * vartok);

/* program_pop_return
 *
 * Purpose:
 *     Pop a value from the program's return stack.
 * 
 * Parameters:
 *     line:   Line pointer
 *     vartok: Variable token
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_pop_return(lineptr_t * line, const tok_t ** vartok);

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
