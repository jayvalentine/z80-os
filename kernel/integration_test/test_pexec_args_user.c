#include <syscall.h>
#include <string.h>

void user_main(char ** argv, size_t argc)
{
    if (argc != 2) return 1;
    if (strcmp("hello", argv[0]) != 0) return 2;
    if (strcmp("world!", argv[1]) != 0) return 3;

    return 0;
}
