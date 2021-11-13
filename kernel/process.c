#include <stddef.h>
#include <syscall.h>

#include <include/file.h>

typedef int (*Command_T)(char **, size_t);

#define user_ram(addr) ((char *)addr)

#define PROGRAM_KB_LIMIT 28

#define PHDR_SIZE 2

#define PHDR_ID 0
#define PHDR_ID_EXEC 0x0a

#define PHDR_PAGE 1
#define USER_RAM_START_PAGE 0x80

int process_exec(uint16_t address, char ** argv, size_t argc)
{
    Command_T process_main = (Command_T)address;
    return process_main(argv, argc);
}

/* pload syscall
 *
 * Summary:
 * Loads a file with the given name into user RAM.
 * 
 * Parameters:
 * address:  Pointer to address to update with location
 *           of loaded executable.
 * filename: Name of the executable file to load.
 * 
 * Returns:
 * 0 if loaded successfully, <0 if file error occurred.
 */
int process_load(uint16_t * address, const char * filename)
{
    int fd = file_open(filename, FMODE_READ);

    /* Return early if error. */
    if (fd < 0) return fd;

    /* Read header of executable file. */
    char header[PHDR_SIZE];
    size_t header_size = file_read(header, PHDR_SIZE, fd);

    /* Check header size. We should have loaded the right number of bytes. */
    if (header_size != PHDR_SIZE) return E_INVALIDHEADER;

    /* Check header byte. 0x0a is executable file. */
    if (header[PHDR_ID] != PHDR_ID_EXEC) return E_INVALIDHEADER;

    /* Base address page is second byte of header. */
    uint8_t base_addr_page = header[PHDR_PAGE];

    /* Check page is valid. */
    if (base_addr_page < USER_RAM_START_PAGE) return E_INVALIDPAGE;

    /* Yes, so load the file into that page. */
    uint16_t base_addr = (uint16_t)base_addr_page << 8;

    char * user_ram_ptr = user_ram(base_addr);

    /* Otherwise read the contents of the file
     * and write to user RAM.
     * We load 1kb at a time. */
    for (int i = 0; i < PROGRAM_KB_LIMIT; i++)
    {
        size_t bytes = file_read(user_ram_ptr, 1024, fd);
        if (bytes != 1024) break;
        user_ram_ptr += 1024;
    }

    /* Update address. Return success. */
    *address = base_addr;
    return 0;
}
