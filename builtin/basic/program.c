#include <stdio.h>
#include <string.h>

#include "program.h"

#include "statement.h"

#include "t_defs.h"
#include "t_numeric.h"

#ifdef Z88DK
extern tok_t _tail;

#define program_start (&_tail)
#define program_max ((tok_t *)0xe000)

#else

tok_t program_region[4096];

#define program_start (&program_region[0])
#define program_max (program_start + 4096)
#endif

tok_t * program_end_ptr;
const tok_t * program_stmt_ptr;
numeric_t current_lineno;
numeric_t next_lineno;

#define PROGSTATE_RUNNING 0
#define PROGSTATE_READY 1

uint8_t program_state;

typedef union _VAR_VALUE_T
{
    numeric_t val;
    tok_t * ptr;
} var_value_t;

typedef struct _CONTEXT_VAR_T
{
    char name[VARNAME_BUF_SIZE];
    var_value_t value;
} context_var_t;


#define MAX_NUMERICS 16

typedef struct _CONTEXT_T
{
    uint8_t count;

    context_var_t defines[MAX_NUMERICS];
} context_t;

context_t program_context;

#define RETURN_STACK_LIMIT 8

/* The return stack.
 * Each entry is an object containing:
 * A linenumber
 * A variable name if applicable
 */
typedef struct _PROGRAM_RETURN_STACK_T
{
    uint8_t count;
    program_return_t stack[RETURN_STACK_LIMIT];
} program_return_stack_t;

program_return_stack_t program_return_stack;

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
error_t program_end(error_t error)
{
    if (program_state != PROGSTATE_RUNNING) return ERROR_NOT_RUNNING;
    program_state = PROGSTATE_READY;
    return error;
}

uint16_t program_free(void)
{
    return (uint16_t)(program_max - program_end_ptr);
}

void program_new(void)
{
    current_lineno = 0;
    program_end_ptr = program_start;
    program_state = PROGSTATE_READY;

    /* Free the variable context. */
    program_context.count = 0;
    for (int i = 0; i < MAX_NUMERICS; i++)
    {
        program_context.defines[i].name[0] = '\0';
    }

    /* Clear the return stack. */
    program_return_stack.count = 0;
}

error_t program_insert(const tok_t * toks)
{
    while (*toks != TOK_TERMINATOR)
    {
        tok_size_t size = t_defs_size(toks);
        for (tok_size_t i = 0; i < size; i++)
        {
            *program_end_ptr = *toks;
            program_end_ptr++;
            toks++;
        }
    }

    *program_end_ptr = TOK_TERMINATOR;
    program_end_ptr++;

    return ERROR_NOERROR;
}

void program_list(void)
{
    const tok_t * p = program_start;

    while (p != program_end_ptr)
    {
        p = t_defs_list(p);
    }
}

/* Helper function to search for a statement
 * with a given linenumber.
 * Returns NULL if it can't be found.
 */
const tok_t * program_search_lineno(int lineno, const tok_t * stmt)
{
    const tok_t * ptr = stmt;
    while (ptr < program_end_ptr)
    {
        /* First token should be a line number or an allocation. */
        tok_t tok_type = *ptr;
        
        /* Skip over it if it's an allocation. */
        if (tok_type == TOK_ALLOC)
        {
            ptr += t_defs_size(ptr);
            continue;
        }

        if (tok_type != TOK_NUMERIC) return NULL;

        int this_lineno = t_numeric_get(ptr+1);
        if (this_lineno >= lineno)
        {
            return ptr;
        }

        /* Otherwise not matching, move onto next statement. */
        ptr += statement_size(ptr);
    }

    /* Didn't find matching line number. */
    return NULL;
}

/* Helper function to return a pointer to the statement
 * with a given line number, or the nearest higher statement
 * if none equal exists.
 */
const tok_t * program_getlineno(int lineno)
{
    /* Greater than current lineno?
     * If so search forward from current program pointer.
     */
    if (lineno >= current_lineno)
    {
        return program_search_lineno(lineno, program_stmt_ptr);
    }

    /* Otherwise search forward from beginning
     * (can't search backwards)
     */
    return program_search_lineno(lineno, program_start);
}

error_t program_run(void)
{
    program_state = PROGSTATE_RUNNING;
    program_stmt_ptr = program_start;

    while (program_stmt_ptr < program_end_ptr)
    {
        /* Skip over it if it's an allocation. */
        if (*program_stmt_ptr == TOK_ALLOC)
        {
            program_stmt_ptr += t_defs_size(program_stmt_ptr);
            continue;
        }

        const tok_t * stmt = program_stmt_ptr;

        /* Sanity check, should start with a line number. */
        if (*stmt != TOK_NUMERIC)
        {
            return program_end(ERROR_LINENUM);
        }

        stmt++;

        /* Get numeric line number. */
        current_lineno = t_numeric_get(stmt);
        next_lineno = current_lineno + 1;

        /* Skip initial lineno. */
        stmt += 2;

        /* Interpret the statement. */
        error_t e = statement_interpret(stmt);

        /* We could have ended the program. */
        if (program_state != PROGSTATE_RUNNING) break;

        if (e != ERROR_NOERROR) return program_end(e);

        /* Move onto next statement. */
        program_stmt_ptr = program_getlineno(next_lineno);
        if (program_stmt_ptr == NULL) return program_end(ERROR_GOTO);
    }

    program_state = PROGSTATE_READY;
    return ERROR_NOERROR;
}

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
int program_current_lineno(void)
{
    return current_lineno;
}

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
void program_set_next_lineno(int lineno)
{
    next_lineno = lineno;
}

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
numeric_t program_next_lineno(void)
{
    return next_lineno;
}

/* The functions below relate to defining, setting,
 * and getting variables. */

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
error_t program_set_numeric(const char * name, numeric_t val)
{
    if (strlen(name) > VARNAME_SIZE) return ERROR_VARNAME;

    /* First check if it is already defined. */
    for (uint8_t i = 0; i < program_context.count; i++)
    {
        if (strcmp(program_context.defines[i].name, name) == 0)
        {
            program_context.defines[i].value.val = val;
            return ERROR_NOERROR;
        }
    }
    
    /* Otherwise we're defining a new variable. */
    if (program_context.count == MAX_NUMERICS) return ERROR_TOO_MANY_VARS;

    context_var_t * define = &program_context.defines[program_context.count];
    strcpy(define->name, name);
    define->value.val = val;

    program_context.count++;

    return ERROR_NOERROR;
}

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
error_t program_get_numeric(const char * name, numeric_t * val)
{
    if (strlen(name) > VARNAME_SIZE) return ERROR_VARNAME;
    
    for (uint8_t i = 0; i < program_context.count; i++)
    {
        if (strcmp(program_context.defines[i].name, name) == 0)
        {
            *val = program_context.defines[i].value.val;
            return ERROR_NOERROR;
        }
    }

    return ERROR_UNDEFINED_VAR;
}

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
error_t program_push_return(const program_return_t * ret)
{
    uint8_t i = program_return_stack.count;
    memcpy((char *)&program_return_stack.stack[i], (const char *)ret, sizeof(program_return_t));
    program_return_stack.count++;

    return ERROR_NOERROR;
}

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
error_t program_pop_return(program_return_t * ret)
{
    if (program_return_stack.count == 0) return ERROR_RETSTACK_EMPTY;

    program_return_stack.count--;
    uint8_t i = program_return_stack.count;
    memcpy((char *)ret, (char *)&program_return_stack.stack[i], sizeof(program_return_t));

    return ERROR_NOERROR;
}

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
tok_t * program_alloc(tok_size_t size)
{
    tok_t * allocated = program_end_ptr;

    *program_end_ptr = TOK_ALLOC;
    program_end_ptr++;

    *program_end_ptr = size;
    program_end_ptr++;

    program_end_ptr += size;

    return allocated;
}

/* program_create_array
 *
 * Purpose:
 *     Create a new array with the given name.
 * 
 * Parameters:
 *     name: Name of new array.
 *     size: Size of array.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_create_array(const char * name, tok_size_t size)
{
    tok_t * a = program_alloc(size);

    context_var_t * define = &program_context.defines[program_context.count];
    strcpy(define->name, name);
    define->value.ptr = a;

    program_context.count++;

    return ERROR_NOERROR;
}

/* program_get_array
 *
 * Purpose:
 *     Get the value (pointer to) an array variable.
 * 
 * Parameters:
 *     name: Name of the variable.
 *     val: Reference value of the array.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_get_array(const char * name, tok_t ** array)
{
    if (strlen(name) > VARNAME_SIZE) return ERROR_VARNAME;
    
    for (uint8_t i = 0; i < program_context.count; i++)
    {
        if (strcmp(program_context.defines[i].name, name) == 0)
        {
            *array = program_context.defines[i].value.ptr;
            return ERROR_NOERROR;
        }
    }

    return ERROR_UNDEFINED_VAR;
}
