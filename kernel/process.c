#include <stddef.h>

typedef int (*Command_T)(char **, size_t);

#define process_main ((Command_T)0x8000)

int process_exec(char ** argv, size_t argc)
{
    return process_main(argv, argc);
}
