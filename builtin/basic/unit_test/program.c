#include "test.h"

#include "program.h"

int test_program_def_variable()
{
    program_new();

    const char * name = "ABC";

    error_t success = program_def_numeric(name, 42);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, success);

    numeric_t num;
    error_t error = program_get_numeric(name, &num);

    ASSERT_EQUAL_UINT(ERROR_NOERROR, error);
    ASSERT_EQUAL_INT(42, num);

    return 0;
}

int test_program_def_2_variables()
{
    program_new();

    const char * name1 = "ABC";
    const char * name2 = "FRED";

    error_t success1 = program_def_numeric(name1, 42);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, success1);

    error_t success2 = program_def_numeric(name2, 13);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, success2);

    numeric_t num1;
    error_t error1 = program_get_numeric(name1, &num1);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, error1);
    ASSERT_EQUAL_INT(42, num1);

    numeric_t num2;
    error_t error2 = program_get_numeric(name2, &num2);
    ASSERT_EQUAL_UINT(ERROR_NOERROR, error2);
    ASSERT_EQUAL_INT(13, num2);

    return 0;
}

int test_program_def_too_many_variables()
{
    program_new();

    char name[2];
    char c = 'A';

    for (int i = 0; i < 16; i++)
    {
        name[0] = c; c++;
        name[1] = '\0';
        error_t success = program_def_numeric(name, i);
        ASSERT_EQUAL_UINT(ERROR_NOERROR, success);
    }

    error_t success2 = program_def_numeric("BOB", 42);
    ASSERT_EQUAL_UINT(ERROR_TOO_MANY_VARS, success2);
    
    c = 'A';
    for (int i = 0; i < 16; i++)
    {
        name[0] = c; c++;
        name[1] = '\0';
        numeric_t num;
        error_t success = program_get_numeric(name, &num);
        ASSERT_EQUAL_UINT(ERROR_NOERROR, success);
        ASSERT_EQUAL_INT(i, num);
    }

    return 0;
}

int test_program_def_variable_toolong()
{
    program_new();

    const char * name = "ABCDE";

    error_t success = program_def_numeric(name, 42);

    ASSERT_EQUAL_UINT(ERROR_VARNAME, success);
    
    return 0;
}

int test_program_get_variable_undefined()
{
    program_new();

    const char * name = "ABC";

    numeric_t num;
    error_t success = program_get_numeric(name, &num);

    ASSERT_EQUAL_UINT(ERROR_UNDEFINED_VAR, success);
    
    return 0;
}
