#ifndef _FILE_H
#define _FILE_H

#include <stdint.h>

#define EOF -1

typedef struct File_T
{
    uint32_t size;
    uint32_t fpos; /* Not actually used for indexing the file, just for keeping track of size. */
    
    uint16_t start_cluster;
    uint16_t current_cluster;

    uint8_t sector; /* Limitation - can't handle more than 256 sectors per cluster. */
    uint16_t fpos_within_sector;
} File_T;

typedef enum
{
    NOERROR,
    FILENOTFOUND
} FileError_T;

FileError_T filesystem_init(void);

FileError_T file_open(const char * filename, File_T * fd);
int file_readbyte(File_T * fd);
uint32_t file_read(uint8_t * buf, File_T * fd, uint32_t n);

#endif /* _FILE_H */
