#include <syscall.h>
#include <stdio.h>

int user_main(char ** argv, size_t argc)
{
    argv; argc;
    
    getchar();

    return 42;
}
