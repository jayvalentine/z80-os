#ifndef _FILE_H

#include "defs.h"

#define EOF -1

typedef struct File_T
{
    ulong size;
    ulong fpos; /* Not actually used for indexing the file, just for keeping track of size. */
    
    uint start_cluster;
    uint current_cluster;

    ubyte sector; /* Limitation - can't handle more than 256 sectors per cluster. */
    uint fpos_within_sector;
} File_T;

int filesystem_init(void);

int file_open(const char * filename, File_T * fd);
int file_readbyte(File_T * fd);
size_t file_read(ubyte * buf, File_T * fd, size_t n);

#endif /* _FILE_H */
