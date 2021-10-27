#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stdint.h>

#define FMODE_READ 0x01
#define FMODE_WRITE 0x02

#define FATTR_SYS 0b00000100
#define FATTR_HID 0b00000010
#define FATTR_RO  0b00000001

/* Information about the disk. Needed for filesystem interaction. */
typedef struct _DiskInfo
{
    /* Region start sectors. */
    uint32_t fat_region;
    uint32_t root_region;
    uint32_t data_region;

    /* Other info. */
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t bytes_per_cluster;
    uint32_t num_sectors;
} DiskInfo_T;

/* Information about a particular file. */
typedef struct _FINFO
{
    uint8_t attr;

    uint32_t size;

    uint16_t created_year;
    uint8_t created_month;
    uint8_t created_day;
} FINFO;

typedef enum
{
    E_FILENOTFOUND = -1,
    E_FILELIMIT = -2,
    E_INVALIDDESCRIPTOR = -3,
    E_INVALIDFILENAME = -4,
    E_ACCESSMODE = -5,
    E_DISKFULL = -6,
    E_FILEEXIST = -7,
    E_DIRFULL = -8,
    E_INVALIDMODE = -9
} FileError_T;

void syscall_dwrite(char * buf, uint32_t sector);
void syscall_dread(char * buf, uint32_t sector);
const DiskInfo_T * syscall_dinfo(void);

void mock_drive_init(void);

#endif /* _SYSCALL_H */
