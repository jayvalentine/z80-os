#include <syscall.h>
#include <string.h>

int user_main(char ** argv, size_t argc)
{
    if (argc != 3) return 1;
    if (strcmp("some", argv[0]) != 0) return 2;
    if (strcmp("arguments", argv[1]) != 0) return 3;
    if (strcmp("here", argv[2]) != 0) return 4;

    return 0;
}
