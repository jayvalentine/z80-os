#ifndef _DISK_H
#define _DISK_H

#include <stdint.h>

void disk_write(char * buf, uint32_t sector);
void disk_read(char * buf, uint32_t sector);

#endif
