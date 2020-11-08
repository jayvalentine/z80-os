#ifndef _FILE_H
#define _FILE_H

#include <stdint.h>
#include <stdio.h>

#define EOF -1

#define FD_FLAGS_CLAIMED 0x01

typedef struct _FileDescriptor
{
    uint8_t flags;
    uint32_t size;
    uint32_t fpos; /* Not actually used for indexing the file, just for keeping track of size. */
    
    uint16_t start_cluster;
    uint16_t current_cluster;

    uint8_t sector; /* Limitation - can't handle more than 256 sectors per cluster. */
    uint16_t fpos_within_sector;
} FileDescriptor_T;

int filesystem_init(void);

int file_open(const char * filename, uint8_t mode);
int file_readbyte(int fd);
size_t file_read(char * ptr, size_t n, int fd);
void file_close(int fd);
int file_info(const char * filename, FINFO * finfo);

#endif /* _FILE_H */
