#include "t_variable.h"

#include "array.h"
#include "program.h"
#include "eval.h"

#include "t_operator.h"

#include <stdio.h>
#include <string.h>

/* t_variable_parse
 *
 * Purpose:
 *     Parse a variable from the input stream.
 * 
 * Parameters:
 *     dst_ptr:   Reference to pointer in destination stream.
 *     input_ptr: Reference to pointer in input stream.
 * 
 * Returns:
 *     1 if successful, 0 otherwise.
 */
int t_variable_parse(tok_t ** dst_ptr, const char ** input_ptr)
{
    tok_t * dst = *dst_ptr;
    const char * input = *input_ptr;

    *dst = TOK_VARIABLE;
    dst++;

    /* Can't set size yet. */
    tok_t * size_ptr = dst;
    dst++;

    tok_size_t size = 0;

    while (*input >= 'A' && *input <= 'Z')
    {
        if (size < VARNAME_SIZE)
        {
            *dst = *input;
            dst++;
            size++;
        }

        input++;
    }

    if (size == 0)
    {
        return 0;
    }

    *size_ptr = size;
    *dst_ptr = dst;
    *input_ptr = input;

    return 1;
}

/* t_variable_size
 *
 * Purpose:
 *     Get the size of a variable token.
 * 
 * Parameters:
 *     toks:   Variable token stream.
 * 
 * Returns:
 *     Size of variable token.
 */
tok_size_t t_variable_size(const tok_t * toks)
{
    tok_size_t size = *toks;

    /* string size + 1 for size byte. */
    return size + 1;
}

/* t_variable_list
 *
 * Purpose:
 *     Print an variable token.
 * 
 * Parameters:
 *     toks:   Variable token stream.
 * 
 * Returns:
 *     Pointer to token after the variable.
 */
const tok_t * t_variable_list(const tok_t * toks)
{
    uint8_t size = *toks;
    toks++;

    for (uint8_t i = 0; i < size; i++)
    {
        char c = *toks;
        toks++;
        putchar(c);
    }
    
    return toks;
}

/* t_variable_get
 *
 * Purpose:
 *     Get a variable name from the given token stream.
 * 
 * Parameters:
 *     var:    Buffer for variable name string.
 *     toks:   Variable token stream.
 * 
 * Returns:
 *     Nothing.
 */
void t_variable_get(char * varname, const tok_t * toks)
{
    tok_size_t size = *toks;
    toks++;
    memcpy(varname, (const char *)toks, size);
    varname[size] = '\0';
}

/* t_variable_get_ptr
 *
 * Purpose:
 *     Returns a pointer to the value of a variable
 *     (scalar or array element) for access.
 * 
 * Parameters:
 *     toks:      Token stream
 *     next_toks: Populated with pointer to next tok after the variable access.
 *     ptr:       Pointer with value pointer
 * 
 * Returns:
 *     Error, if any.
 */
error_t t_variable_get_ptr(const tok_t * toks, const tok_t ** next_toks, numeric_t ** value)
{
    error_t e;

    /* Check that first token is a variable */
    if (*toks != TOK_VARIABLE) return ERROR_SYNTAX;

    /* Construct variable name string. */
    char varname[VARNAME_BUF_SIZE];
    t_variable_get(varname, toks+1);

    /* Get next token. */
    SKIP(toks);

    /* Is it a left paren?
     * If so this is an array access.
     * Otherwise, it's a normal variable.
     */
    if (*toks == TOK_OPERATOR && *(toks + 1) == OP_LPAREN)
    {
        /* Skip over left paren. */
        SKIP(toks);

        /* Evaluate a sub-expression to get the index. */
        numeric_t index;
        e = eval_numeric(&index, toks);
        ERROR_HANDLE(e);

        /* Skip ahead to the next right paren (closing the array access) */
        while (!(*toks == TOK_OPERATOR && *(toks + 1) == OP_RPAREN))
        {
            SKIP(toks);
        }

        /* Skip the right paren */
        SKIP(toks);
        
        /* Get the array. */
        tok_t * arr;
        e = program_get_array(varname, &arr);
        ERROR_HANDLE(e);

        /* Bounds checking of index. */
        tok_size_t array_size = ARRAY_SIZE(arr);
        if (index < 1) return ERROR_RANGE;
        if ((index*sizeof(numeric_t)) > array_size) return ERROR_RANGE;

        /* Set pointer to element in array. */
        *value = &ARRAY_ACCESS(arr, index);
        *next_toks = toks;
        return ERROR_NOERROR;
    }
    else
    {
        /* Get variable value. */
        numeric_t * val;
        e = program_get_numeric_ref(varname, &val);
        ERROR_HANDLE(e);

        /* Set pointer to variable value */
        *value = val;
        *next_toks = toks;
        return ERROR_NOERROR;
    }
}
