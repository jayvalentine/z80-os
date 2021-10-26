#include <stddef.h>
#include <syscall.h>

#include <include/file.h>

typedef int (*Command_T)(char **, size_t);

#define process_main ((Command_T)0x8000)
#define user_ram ((char *)0x8000)

#define PROGRAM_KB_LIMIT 28

int process_exec(char ** argv, size_t argc)
{
    return process_main(argv, argc);
}

/* pload syscall
 *
 * Summary:
 * Loads a file with the given name into user RAM.
 * 
 * Parameters:
 * filename: Name of the executable file to load.
 * 
 * Returns:
 * 0 if loaded successfully, <0 if file error occurred.
 */
int process_load(const char * filename)
{
    char * user_ram_ptr = user_ram;

    int fd = file_open(filename, FMODE_READ);

    /* Return early if error. */
    if (fd < 0) return fd;

    /* Otherwise read the contents of the file
     * and write to user RAM.
     * We load 1kb at a time. */
    for (int i = 0; i < PROGRAM_KB_LIMIT; i++)
    {
        size_t bytes = file_read(user_ram_ptr, 1024, fd);
        if (bytes != 1024) break;
    }

    return 0;
}
