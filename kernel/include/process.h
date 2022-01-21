#ifndef _PROCESS_H
#define _PROCESS_H

#include <stddef.h>
#include <stdint.h>

typedef struct _ProcessDescriptor_T
{
    uintptr_t base_address;
    uint8_t bank;
} ProcessDescriptor_T;

const ProcessDescriptor_T * process_info(int pid);

int process_spawn(int pd, char ** argv, size_t argc);
int process_load(const char * filename);
void process_init(void);

#endif
