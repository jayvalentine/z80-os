#include <syscall.h>
#include <string.h>

/* Mock a "drive" - just a sequential series of sectors. */
uint8_t drive[128][512];

/* Extern the "diskinfo" struct. */
extern DiskInfo_T disk_info;

void syscall_dwrite(char * buf, uint32_t sector)
{
    if (sector < 128) memcpy(drive[sector], buf, 512);
}

void syscall_dread(char * buf, uint32_t sector)
{
    if (sector < 128) memcpy(buf, drive[sector], 512);
}

const DiskInfo_T * syscall_dinfo(void)
{
    return &disk_info;
}

void mock_drive_init(void)
{
    memset(drive, 0, 128*512);
}
