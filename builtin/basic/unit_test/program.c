#include "test.h"

#include "program.h"

#include <string.h>

int test_program_def_variable()
{
    program_new();

    tok_t var[6];
    MAKE_VAR_TOK(var, "ABC");

    error_t success = program_set_numeric(var, 42);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, success);

    numeric_t num;
    error_t error = program_get_numeric(var, &num);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, error);
    ASSERT_EQUAL_INT(42, num);

    return 0;
}

int test_program_def_2_variables()
{
    program_new();

    tok_t var1[6];
    tok_t var2[7];
    MAKE_VAR_TOK(var1, "ABC");
    MAKE_VAR_TOK(var2, "FRED");

    error_t success1 = program_set_numeric(var1, 42);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, success1);

    error_t success2 = program_set_numeric(var2, 13);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, success2);

    numeric_t num1;
    error_t error1 = program_get_numeric(var1, &num1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, error1);
    ASSERT_EQUAL_INT(42, num1);

    numeric_t num2;
    error_t error2 = program_get_numeric(var2, &num2);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, error2);
    ASSERT_EQUAL_INT(13, num2);

    return 0;
}

int test_program_def_too_many_variables()
{
    program_new();

    tok_t var[6];
    char name[3];
    char c = 'A';

    for (int i = 0; i < 16; i++)
    {
        name[0] = 'V';
        name[1] = c; c++;
        name[2] = '\0';
        MAKE_VAR_TOK(var, name);

        error_t success = program_set_numeric(var, i);
        ASSERT_EQUAL_UINT(ERROR_NOERROR, success);
    }

    tok_t bob[6];
    MAKE_VAR_TOK(bob, "BOB");
    error_t success2 = program_set_numeric(bob, 42);
    ASSERT_EQUAL_UINT(ERROR_TOO_MANY_VARS, success2);
    
    c = 'A';
    for (int i = 0; i < 16; i++)
    {
        name[0] = 'V';
        name[1] = c; c++;
        name[2] = '\0';
        MAKE_VAR_TOK(var, name);

        numeric_t num;
        error_t success = program_get_numeric(var, &num);
        ASSERT_EQUAL_UINT(ERROR_NOERROR, success);
        ASSERT_EQUAL_INT(i, num);
    }

    return 0;
}

int test_program_def_variable_toolong()
{
    program_new();

    tok_t var[8];
    MAKE_VAR_TOK(var, "ABCDE");

    error_t success = program_set_numeric(var, 42);

    ASSERT_EQUAL_UINT(ERROR_VARNAME, success);
    
    return 0;
}

int test_program_get_variable_undefined()
{
    program_new();

    tok_t var[8];
    MAKE_VAR_TOK(var, "ABC");

    numeric_t num;
    error_t success = program_get_numeric(var, &num);

    ASSERT_EQUAL_UINT(ERROR_UNDEFINED_VAR, success);
    
    return 0;
}

int test_program_set_variable_twice()
{
    program_new();

    tok_t var[8];
    MAKE_VAR_TOK(var, "ABC");

    error_t success = program_set_numeric(var, 42);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, success);

    numeric_t num;
    error_t error = program_get_numeric(var, &num);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, error);
    ASSERT_EQUAL_INT(42, num);

    error_t error2 = program_set_numeric(var, 15);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, error2);

    error_t error3 = program_get_numeric(var, &num);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, error3);
    ASSERT_EQUAL_INT(15, num);

    return 0;
}

int test_program_alloc()
{
    program_new();

    tok_t * a = program_alloc(5);

    ASSERT_EQUAL_UINT(TOK_ALLOC, a[0]);
    ASSERT_EQUAL_UINT(5, a[1]);
    ASSERT_EQUAL_UINT(0, a[2]);
    ASSERT_EQUAL_UINT(0, a[3]);
    ASSERT_EQUAL_UINT(0, a[4]);
    ASSERT_EQUAL_UINT(0, a[5]);
    ASSERT_EQUAL_UINT(0, a[6]);

    return 0;
}
