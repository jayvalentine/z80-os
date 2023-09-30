#include <stddef.h>
#include <syscall.h>
#include <string.h>

#include <include/file.h>
#include <include/process.h>
#include <include/memory.h>
#include <include/ram.h>
#include <include/scheduler.h>

typedef int (*Command_T)(char **, size_t);

#define user_ram(addr) ((char *)addr)

#define PROGRAM_KB_LIMIT (28 * 1024)

#define PHDR_SIZE 2

#define PHDR_ID 0
#define PHDR_ID_EXEC 0x0a

#define PHDR_PAGE 1
#define USER_RAM_START_PAGE 0x80

#define PROCS_MAX 16

ProcessDescriptor_T process_table[PROCS_MAX];

void process_init(void)
{
    for (int i = 0; i < PROCS_MAX; i++)
    {
        process_table[i].base_address = 0x0000;
    }
#ifdef DEBUG
    process_table[0].base_address = 0x8000;
    process_table[0].bank = 0;
    process_table[0].termstatus = 0;
#endif
}

const ProcessDescriptor_T * process_info(int pid)
{
    return &process_table[pid];
}

ProcessDescriptor_T * process_current(void)
{
    int pid = scheduler_current();
    return &process_table[pid];
}

#define process_argc ((char *)0xf800)
#define process_argv ((char *)0xf810)
#define process_args ((char *)0xf820)

int process_spawn(int pd, char ** argv, size_t argc)
{
    /* Copy argv strings onto stack. */
    char arg_buffer[512];
    char * arg_buffer_ptr = arg_buffer;

    char * argv_buffer[16];

    for (size_t i = 0; i < argc; i++)
    {
        char * s1 = argv[i];
        size_t l = strlen(s1);

        char * s = strcpy(arg_buffer_ptr, s1);
        
        /* Calculate offset into buffer of the string. */
        char * buf_addr = (s - arg_buffer) + process_args;
        argv_buffer[i] = buf_addr;
        
        arg_buffer_ptr += l + 1;
    }

    uint8_t bank = process_table[pd].bank;

    size_t argc_local = argc;

    ram_copy(process_argc, bank, (char*) &argc_local, 2);
    ram_copy(process_argv, bank, (char*) argv_buffer, 16);
    ram_copy(process_args, bank, arg_buffer, 512);

    /* Create a scheduler entry for this process. */
    int success = scheduler_add(pd);
    if (success) return success;
    else return 0;
}

int process_allocate(void)
{
    for (int i = 0; i < PROCS_MAX; i++)
    {
        if (process_table[i].base_address == 0x0000) return i;
    }

    return -1;
}

/* Has to be global or else it gets corrupted
 * when we switch banks. */
uint8_t current_bank;
char * p;
char * user_ram_ptr;
int fd;
char base_address_high_byte;

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
 * process descriptor if loaded successfully, <0 if error occurred.
 */
int process_load(const char * filename)
{
    fd = file_open(filename, FMODE_READ);

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
    uintptr_t base_addr_page = header[PHDR_PAGE];

    /* Check page is valid. */
    if (base_addr_page < USER_RAM_START_PAGE) return E_INVALIDPAGE;

    /* Find the next available process descriptor. */
    int pd = process_allocate();

    /* Allocate a bank of memory and load the file into that page. */
    int bank = memory_allocate();
    uintptr_t base_addr = base_addr_page << 8;
    process_table[pd].base_address = base_addr;
    process_table[pd].bank = bank;

    user_ram_ptr = user_ram(base_addr);
    base_address_high_byte = (char)base_addr_page;

    /* Otherwise read the contents of the file
     * and write to user RAM. */
    current_bank = ram_bank_current();
    
    ram_bank_set(process_table[pd].bank);

    file_read(user_ram_ptr, PROGRAM_KB_LIMIT, fd);
    
    p = (char*)0xffff;

    /* Stack pointer on entry. */
    *p-- = 0xf7;
    *p-- = 0xec;

    /* Skip space on stack for register values
     * (because they don't matter when the process starts)
     */
    p = (char*)0xf7ff;

    /* Set up argv and argc. */
    *p-- = 0x00;
    *p-- = 0x00;
    *p-- = 0x00;
    *p-- = 0x00;

    /* Return address from process main() */
    *p-- = 0x00;
    *p-- = 0x00;

    /* Entry point. */
    *p-- = base_address_high_byte;
    *p-- = 0x00;

    ram_bank_set(current_bank);

    /* Set other process attributes. */
    process_table[pd].termstatus = 0;
    process_table[pd].sigstatus = 0;
    process_table[pd].sighandlers.cancel = NULL;
    process_table[pd].sighandlers.brk = NULL;

    /* Update address. Return process descriptor. */
    return pd;
}

void process_exit(int code)
{
    int s = scheduler_current();
    scheduler_exit(s, code);
}
