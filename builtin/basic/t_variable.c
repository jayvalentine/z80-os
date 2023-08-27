#include "t_variable.h"

#include "array.h"
#include "program.h"
#include "eval.h"

#include "t_operator.h"

#include <stdio.h>
#include <string.h>

int varcmp(const tok_t * left, const tok_t * right)
{
    if (left == NULL || right == NULL) return 1;
    
    /* If they're not even the same token type
     * then they can't be equal.
     */
    if (*left != *right) return 1;

    if (*left == TOK_REGISTER)
    {
        if (*(left+1) == *(right+1)) return 0;
        else return 1;
    }
    else if (*left == TOK_VARIABLE)
    {
        return strcmp((const char *)(left+1), (const char *)(right+1));
    }

    /* Not variable/register tokens. */
    return 1;
}

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

    /* Can't set tok yet. */
    tok_t * type_ptr = dst;
    dst++;

    /* Can't set size yet. */
    tok_t * size_ptr = dst;
    dst++;

    /* Is this a single-letter variable?
     * If so, parse as the special "register" token type.
     * This allows faster runtime access.
     */
    if (*input >= 'A' && *input <= 'Z'
        && (*(input+1) < 'A' || *(input+1) > 'Z'))
    {
        *type_ptr = TOK_REGISTER;
        *size_ptr = *input - 'A';

        input++;
    }
    else
    {
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

        /* Null-terminate the string. */
        *dst = '\0';
        dst++;
        size++;

        *type_ptr = TOK_VARIABLE;
        *size_ptr = size;
    }

    *dst_ptr = dst;
    *input_ptr = input;

    return 1;
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

    /* Check that first token is a variable or register. */
    const tok_t * var;
    if (*toks == TOK_VARIABLE)
    {
        var = toks;
        toks = t_varlen_skip(toks);
    }
    else if (*toks == TOK_REGISTER)
    {
        var = toks;
        toks += REGISTER_SIZE;
    }
    else return ERROR_SYNTAX;

    /* Is it a left paren?
     * If so this is an array access.
     * Otherwise, it's a normal variable.
     */
    if (*toks == TOK_OPERATOR && *(toks + 1) == OP_LPAREN)
    {
        /* Skip over left paren. */
        toks += OP_SIZE;

        /* Evaluate a sub-expression to get the index. */
        numeric_t index;
        const tok_t * expr_end;
        e = eval_numeric(&index, toks, &expr_end);
        ERROR_HANDLE(e);

        /* Skip ahead to the next right paren (closing the array access) */
        toks = expr_end;

        /* Skip the right paren */
        toks += OP_SIZE;
        
        /* Get the array. */
        tok_t * arr;
        e = program_get_array(var, &arr);
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
        e = program_get_numeric_ref(var, &val);
        ERROR_HANDLE(e);

        /* Set pointer to variable value */
        *value = val;
        *next_toks = toks;
        return ERROR_NOERROR;
    }
}
