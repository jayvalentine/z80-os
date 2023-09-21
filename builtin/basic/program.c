#include <stdio.h>
#include <string.h>

#include "program.h"

#include "statement.h"

#include "t_defs.h"
#include "t_numeric.h"

tok_t * program_end_ptr;

#ifndef Z80
tok_t program_region[4096];
#endif

#define PROGSTATE_RUNNING 0
#define PROGSTATE_READY 1

uint8_t program_state;

/* Union representing a variable type.
 * Type is inferred from usage - there is no indicator
 * for the type in the variable type itself.
 * 
 * A variable can either be:
 *     A numeric
 *     An array (pointer to a region in the program memory)
 */
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


#define MAX_VARS 16

typedef struct _CONTEXT_T
{
    uint8_t count;

    context_var_t defines[MAX_VARS];
} context_t;

/* There is a "register" for each single-letter variable A-Z. */
typedef struct _REGISTER_T
{
    uint8_t defined;
    var_value_t value;
} register_t;

#define NUM_REGISTERS 26
register_t program_registers[NUM_REGISTERS];

context_t program_context;

#define RETURN_STACK_LIMIT 8

/* Type of an entry in the program return stack.
 * line:     Line to return to.
 * vartoken: Token for associated variable (NULL if N/A).
 *           Must have program lifetime.
 */
typedef struct _PROGRAM_RETURN_T
{
    lineptr_t line;
    const tok_t * vartoken;
} program_return_t;

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

const tok_t * program_current_line;
tok_t * program_last_inserted_line;

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
    program_end_ptr = program_start;
    program_current_line = program_start;
    program_last_inserted_line = NULL;

    program_state = PROGSTATE_READY;

    /* Free registers. */
    for (int i = 0; i < NUM_REGISTERS; i++)
    {
        program_registers[i].defined = 0;
    }

    /* Free the variable context. */
    program_context.count = 0;
    for (int i = 0; i < MAX_VARS; i++)
    {
        program_context.defines[i].name[0] = '\0';
    }

    /* Clear the return stack. */
    program_return_stack.count = 0;
}

error_t program_insert(const tok_t * toks)
{
    /* First token should be a numeric. */
    if (*toks != TOK_NUMERIC) return ERROR_SYNTAX;
    numeric_t lineno = NUMERIC_GET(toks);
    toks += NUMERIC_SIZE;

    /* Get size of statement to insert. */
    tok_size_t size = statement_size(toks);

    /* Insertion point of statement. */
    tok_t * line = program_end_ptr;

    /* Set pointer of previous line to this one. */
    if (program_last_inserted_line != NULL) PROG_NEXT(program_last_inserted_line) = line;

    PROG_LINENO(line) = lineno;
    PROG_PREV(line) = program_last_inserted_line;
    PROG_NEXT(line) = NULL;

    /* Insert tokens into program memory. */
    memcpy(PROG_STMT(line), toks, size);

    /* Record that this line has been inserted last so that the next
     * line inserted can reference it.
     */
    program_last_inserted_line = line;

    /* Increment end pointer. */
    program_end_ptr += size + PROG_HDR_SIZE;

    return ERROR_NOERROR;
}

const tok_t dummy[1] = { 0 };

void program_list_stmt(const tok_t * toks)
{
    while (*toks != TOK_TERMINATOR)
    {
        toks = t_defs_list(toks);
    }
    t_defs_list(toks);
}

void program_list(void)
{
    const tok_t * line = program_start;

    while (line != NULL && PROG_LINENO(line) != 0)
    {
        printf("%d ", PROG_LINENO(line));
        program_list_stmt(PROG_STMT(line));
     
        line = PROG_NEXT(line);
    }
}

/* Helper function to search for a listing
 * with a given linenumber.
 *
 * Returns line or NULL if it can't be found.
 */
const tok_t * program_search_lineno(int lineno)
{
    const tok_t * line = program_current_line;
    if (lineno < PROG_LINENO(line))
    {
        /* Search backwards */
        while (PROG_LINENO(line) >= lineno)
        {
            if (PROG_PREV(line) == NULL) return NULL;
            line = PROG_PREV(line);
        }

        /* i now points to the first statement which has a lineno
         * < to the search lineno.
         *
         * we want the statement after that - i.e. the furthest away
         * statement which is >= the search lineno.
         */
        return PROG_NEXT(line);
    }
    else
    {
        /* Search forwards */
        while (PROG_LINENO(line) < lineno)
        {
            if (PROG_NEXT(line) == NULL) return NULL;
            line = PROG_NEXT(line);
        }
        return line;
    }
}

const tok_t * program_next_line;

error_t program_run(void)
{
    error_t e;

    program_state = PROGSTATE_RUNNING;

    program_next_line = program_start;
    
    /* Free the variable context. */
    program_context.count = 0;
    for (int i = 0; i < MAX_VARS; i++)
    {
        program_context.defines[i].name[0] = '\0';
    }

    /* Clear the return stack. */
    program_return_stack.count = 0;

    while (program_next_line != NULL)
    {
        program_current_line = program_next_line;
        const tok_t * stmt = PROG_STMT(program_current_line);

        /* Shouldn't ever be an allocation or a numeric. */
        if (*stmt == TOK_ALLOC) return program_end(ERROR_SYNTAX);
        if (*stmt == TOK_NUMERIC) return program_end(ERROR_SYNTAX);

        /* By default control transfers to the next line. */
        program_next_line = PROG_NEXT(program_current_line);

        /* Interpret the statement. */
        e = statement_interpret(stmt);

        /* We could have ended the program. */
        if (program_state != PROGSTATE_RUNNING) break;

        if (e != ERROR_NOERROR) return program_end(e);

        /* Move onto next statement. */
        if (program_next_line == NULL) return program_end(ERROR_GOTO);
    }

    program_state = PROGSTATE_READY;
    return ERROR_NOERROR;
}

lineptr_t program_get_current_line(void)
{
    return GET_LINEPTR(program_current_line);
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
numeric_t program_current_lineno(void)
{
    return PROG_LINENO(program_current_line);
}

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
lineptr_t program_transfer_control(numeric_t lineno)
{
    program_next_line = program_search_lineno(lineno);

    return GET_LINEPTR(program_next_line);
}

void program_transfer_control_direct(lineptr_t line)
{
    program_next_line = GET_LINE(line);
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
    if (program_next_line == NULL) return -1;
    return PROG_LINENO(program_next_line);
}

/* The functions below relate to defining, setting,
 * and getting variables. */

numeric_t * program_get_numeric_ref(const tok_t * toks)
{
    /* Is the variable a register? */
    if (*toks == TOK_REGISTER)
    {
        return &program_registers[*(toks+1)].value.val;
    }

    /* Otherwise look up the variable in the context list. */
    const char * name = VARIABLE_GET(toks);
    
    for (uint8_t i = 0; i < program_context.count; i++)
    {
        if (strcmp(program_context.defines[i].name, name) == 0)
        {
            return &program_context.defines[i].value.val;
        }
    }

    return NULL;
}

error_t program_set_numeric(const tok_t * toks, numeric_t val)
{
    numeric_t * ref = program_get_numeric_ref(toks);

    /* If the variable already exists then just set it. */
    if (ref != NULL)
    {
        *ref = val;
        return ERROR_NOERROR;
    }

    /* Otherwise we're defining a new variable. */
    if (program_context.count == MAX_VARS) return ERROR_TOO_MANY_VARS;

    const char * varname = VARIABLE_GET(toks);
    if (strlen(varname) > VARNAME_SIZE) return ERROR_VARNAME;

    context_var_t * define = &program_context.defines[program_context.count];
    strcpy(define->name, varname);
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
error_t program_get_numeric(const tok_t * toks, numeric_t * val)
{
    numeric_t * ref = program_get_numeric_ref(toks);
    if (ref == NULL) return ERROR_UNDEFINED_VAR;

    *val = *ref;
    return ERROR_NOERROR;
}

/* program_push_return
 *
 * Purpose:
 *     Push a value onto the program's return stack.
 * 
 * Parameters:
 *     line:   Line pointer
 *     vartok: Variable token
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_push_return(lineptr_t line, const tok_t * vartok)
{
    program_return_t * ret = &program_return_stack.stack[program_return_stack.count];
    ret->line = line;
    ret->vartoken = vartok;
    program_return_stack.count++;

    return ERROR_NOERROR;
}

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
error_t program_pop_return(lineptr_t * line, const tok_t ** vartok)
{
    if (program_return_stack.count == 0) return ERROR_RETSTACK_EMPTY;

    program_return_stack.count--;
    program_return_t * ret = &program_return_stack.stack[program_return_stack.count];
    *line = ret->line;
    *vartok = ret->vartoken;

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

    for (tok_size_t i = 0; i < size; i++)
    {
        *program_end_ptr = 0;
        program_end_ptr++;
    }

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
error_t program_create_array(const tok_t * toks, tok_size_t size)
{
    tok_t * a = program_alloc(size);

    /* Is the variable a register? */
    if (*toks == TOK_REGISTER)
    {
        program_registers[*(toks+1)].value.ptr = a;
        program_registers[*(toks+1)].defined = 1;
        return ERROR_NOERROR;
    }

    /* Otherwise add the variable to the context list. */
    const char * name = VARIABLE_GET(toks);

    /* Get next available space in context for new variable */
    context_var_t * define = &program_context.defines[program_context.count];

    /* Set variable name and pointer to array */
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
error_t program_get_array(const tok_t * toks, tok_t ** array)
{
    /* Is the variable a register? */
    if (*toks == TOK_REGISTER)
    {
        toks++;
        register_t * reg = &program_registers[*toks];
        if (!reg->defined) return ERROR_UNDEFINED_ARRAY;

        *array = reg->value.ptr;
        return ERROR_NOERROR;
    }

    /* Otherwise get array from context list. */
    const char * name = VARIABLE_GET(toks);
    if (strlen(name) > VARNAME_SIZE) return ERROR_VARNAME;
    
    for (uint8_t i = 0; i < program_context.count; i++)
    {
        if (strcmp(program_context.defines[i].name, name) == 0)
        {
            *array = program_context.defines[i].value.ptr;
            return ERROR_NOERROR;
        }
    }

    return ERROR_UNDEFINED_ARRAY;
}
