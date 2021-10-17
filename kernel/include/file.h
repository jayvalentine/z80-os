#ifndef _FILE_H
#define _FILE_H

#include <stdint.h>
#include <stdio.h>

#include <syscall.h>

#define KERNEL_EOF -1

#define FD_FLAGS_CLAIMED 0x01

#define FILE_LIMIT 1

typedef struct _DirectoryEntry
{
    char name[8];
    char ext[3];

    uint8_t attributes;

    uint8_t reserved_for_windows_nt;

    uint8_t creation_milliseconds;
    uint16_t creation_time;
    uint16_t creation_date;

    uint16_t last_access_date;

    uint16_t reserved_for_fat32;

    uint16_t last_write_time;
    uint16_t last_write_date;

    uint16_t starting_cluster;
    uint32_t size;
} DirectoryEntry_T;

typedef struct _FileDescriptor
{
    char name[13];
    
    uint8_t flags;
    uint8_t mode;
    uint32_t size;
    uint32_t fpos; /* Not actually used for indexing the file, just for keeping track of size. */
    
    uint16_t start_cluster;
    uint16_t current_cluster;

    uint8_t sector; /* Limitation - can't handle more than 256 sectors per cluster. */
    uint16_t fpos_within_sector;
} FileDescriptor_T;

int filesystem_init(void);

int file_new(const char * filename);
int file_delete(const char * filename);
int file_open(const char * filename, uint8_t mode);
int file_readbyte(int fd);
size_t file_read(char * ptr, size_t n, int fd);
size_t file_write(char * ptr, size_t n, int fd);
void file_close(int fd);
int file_info(const char * filename, FINFO * finfo);

#endif /* _FILE_H */
