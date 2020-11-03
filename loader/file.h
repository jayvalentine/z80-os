#ifndef _FILE_H

#include "defs.h"

typedef struct File_T
{
    ulong size;
} File_T;

typedef enum
{
    NOERROR,
    FILENOTFOUND
} FileError_T;

FileError_T filesystem_init(void);
FileError_T file_open(const char * filename, File_T * fd);

#endif /* _FILE_H */
