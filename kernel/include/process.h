#ifndef _PROCESS_H
#define _PROCESS_H

#include <stddef.h>
#include <stdint.h>

#include <include/terminal.h>

typedef struct _ProcessDescriptor_T
{
    uintptr_t base_address;
    uint8_t bank;
    termstatus_t termstatus;
} ProcessDescriptor_T;


int process_spawn(int pd, char ** argv, size_t argc);
int process_load(const char * filename);
void process_init(void);

/* process_current
 *
 * Returns a pointer to the process descriptor
 * for the current process.
 */
ProcessDescriptor_T * process_current(void);

/* process_info
 *
 * Returns a read-only pointer to the process descriptor
 * for the process with given process ID.
 */
const ProcessDescriptor_T * process_info(int pid);

#endif
