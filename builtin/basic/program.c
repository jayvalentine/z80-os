#include <stdio.h>
#include <string.h>

#include "program.h"

#include "statement.h"

#include "t_defs.h"
#include "t_numeric.h"

#ifdef Z80
#define TGT_FASTCALL __z88dk_fastcall

#include <os.h>

extern tok_t end;

#define program_start (&end)
#define program_max ((tok_t *)USER_PROG_END)

#else
#define TGT_FASTCALL

tok_t program_region[4096];

#define program_start (&program_region[0])
#define program_max (program_start + 4096)
#endif

tok_t * program_end_ptr;

numeric_t next_lineno;

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

/* Program listing.
 *
 * Matches line numbers to program statements.
 */
typedef struct _LISTING_ENTRY_T
{
    numeric_t lineno;
    const tok_t * stmt;
} listing_entry_t;

typedef struct _LISTING_T
{
    int16_t count;
    int16_t index; /* -1 if invalid. */
    listing_entry_t entries[100];
} listing_t;

listing_t program_listing;

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
    program_listing.count = 0;
    program_listing.index = 0;

    program_end_ptr = program_start;
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

void program_listing_insert(numeric_t lineno, const tok_t * toks)
{
    program_listing.entries[program_listing.count].lineno = lineno;
    program_listing.entries[program_listing.count].stmt = toks;
    program_listing.count++;
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
    const tok_t * stmt = program_end_ptr;

    /* Insert tokens into program memory. */
    memcpy(program_end_ptr, toks, size);
    program_end_ptr += size;

    /* Insert into listing. */
    program_listing_insert(lineno, stmt);

    return ERROR_NOERROR;
}

void program_list(void)
{
    for (int i = 0; i < program_listing.count; i++)
    {
        printf("%d ", program_listing.entries[i].lineno);

        const tok_t * p = program_listing.entries[i].stmt;
        while (*p != TOK_TERMINATOR)
        {
            p = t_defs_list(p);
        }
        t_defs_list(p);
    }
}

/* Helper function to search for a listing
 * with a given linenumber.
 *
 * Returns listing index or -1 if it can't be found.
 */
int16_t program_search_lineno(int lineno) TGT_FASTCALL
{
    if (lineno < program_listing.entries[program_listing.index].lineno)
    {
        /* Search backwards */
        uint16_t i = program_listing.index - 1;
        while (program_listing.entries[i].lineno >= lineno)
        {
            if (i == 0) return -1;
            i--;
        }

        /* i now points to the first statement which has a lineno
         * < to the search lineno.
         *
         * we want the statement after that - i.e. the furthest away
         * statement which is >= the search lineno.
         */
        return i+1;
    }
    else
    {
        /* Search forwards */
        uint16_t i = program_listing.index + 1;
        while (program_listing.entries[i].lineno < lineno)
        {
            if (i == program_listing.count) return -1;
            i++;
        }
        return i;
    }
}

/* Helper function to move to the next line in the program.
 */
static error_t program_nextline(numeric_t lineno) TGT_FASTCALL
{
    int16_t index = program_search_lineno(lineno);
    if (index == -1) return ERROR_GOTO;

    program_listing.index = index;
    return ERROR_NOERROR;
}

error_t program_run(void)
{
    error_t e;

    program_state = PROGSTATE_RUNNING;

    program_listing.index = 0;
    next_lineno = 0;
    
    /* Free the variable context. */
    program_context.count = 0;
    for (int i = 0; i < MAX_VARS; i++)
    {
        program_context.defines[i].name[0] = '\0';
    }

    /* Clear the return stack. */
    program_return_stack.count = 0;

    while (program_listing.index < program_listing.count)
    {
        const tok_t * stmt = program_listing.entries[program_listing.index].stmt;

        /* Shouldn't ever be an allocation or a numeric. */
        if (*stmt == TOK_ALLOC) return program_end(ERROR_SYNTAX);
        if (*stmt == TOK_NUMERIC) return program_end(ERROR_SYNTAX);

        /* Get next line number. */
        next_lineno = program_current_lineno() + 1;

        /* Interpret the statement. */
        e = statement_interpret(stmt);

        /* We could have ended the program. */
        if (program_state != PROGSTATE_RUNNING) break;

        if (e != ERROR_NOERROR) return program_end(e);

        /* Move onto next statement. */
        e = program_nextline(next_lineno);
        if (e != ERROR_NOERROR) return program_end(e);
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
numeric_t program_current_lineno(void)
{
    return program_listing.entries[program_listing.index].lineno;
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
void program_set_next_lineno(numeric_t lineno)
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

numeric_t * program_get_numeric_ref(const tok_t * toks) TGT_FASTCALL
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
 *     ret: Reference to program_return_t object.
 * 
 * Returns:
 *     Error, if any.
 */
error_t program_push_return(const program_return_t * ret)
{
    uint8_t i = program_return_stack.count;
    program_return_stack.stack[i].lineno = ret->lineno;
    program_return_stack.stack[i].vartoken = ret->vartoken;
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
    ret->lineno = program_return_stack.stack[i].lineno;
    ret->vartoken = program_return_stack.stack[i].vartoken;

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
        if (!program_registers[*(toks+1)].defined) return ERROR_UNDEFINED_ARRAY;

        *array = program_registers[*(toks+1)].value.ptr;
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
