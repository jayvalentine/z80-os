#include <include/file.h>

#include <syscall.h>

extern FileDescriptor_T fdtable[FILE_LIMIT];
extern DiskInfo_T disk_info;

/* Given an open file descriptor with <512 bytes, ensure that file_read
 * reads the correct number of bytes.
 */
int test_read_less_than_512_bytes()
{
    mock_drive_init();

    disk_info.bytes_per_sector = 512;
    disk_info.data_region = 32;
    disk_info.sectors_per_cluster = 4;

    syscall_dwrite("HelloAndSomeGarbage", 32);

    FileDescriptor_T * file = &fdtable[0];
    file->mode = FMODE_READ;
    
    file->current_cluster = 2;
    file->sector = 0;
    file->fpos_within_sector = 0;
    file->fpos = 0;

    file->size = 5;

    char buf[32];
    memset(buf, '!', 32);

    size_t bytes = file_read(buf, 32, 0);

    /* We should only have read 5 bytes. */
    if (bytes != 5) return 1;

    /* We should only have overwritten the first 5 bytes of buf. */
    if (buf[0] != 'H') return 1;
    if (buf[1] != 'e') return 1;
    if (buf[2] != 'l') return 1;
    if (buf[3] != 'l') return 1;
    if (buf[4] != 'o') return 1;

    if (buf[5] != '!') return 1;
    if (buf[15] != '!') return 1;
    if (buf[31] != '!') return 1;

    return 0;
}

/* Given an open file descriptor with <512 bytes, ensure that file_read
 * reads the correct number of bytes when we try to read a full sector.
 */
int test_read_512_bytes()
{
    mock_drive_init();

    disk_info.bytes_per_sector = 512;
    disk_info.data_region = 32;
    disk_info.sectors_per_cluster = 4;

    syscall_dwrite("HelloAndSomeGarbage", 32);

    FileDescriptor_T * file = &fdtable[0];
    file->mode = FMODE_READ;
    
    file->current_cluster = 2;
    file->sector = 0;
    file->fpos_within_sector = 0;
    file->fpos = 0;

    file->size = 5;

    char buf[512];
    memset(buf, '!', 512);

    size_t bytes = file_read(buf, 512, 0);

    /* We should only have read 5 bytes. */
    if (bytes != 5) return 1;

    /* We should only have overwritten the first 5 bytes of buf. */
    if (buf[0] != 'H') return 1;
    if (buf[1] != 'e') return 1;
    if (buf[2] != 'l') return 1;
    if (buf[3] != 'l') return 1;
    if (buf[4] != 'o') return 1;

    if (buf[5] != '!') return 1;
    if (buf[15] != '!') return 1;
    if (buf[31] != '!') return 1;
    if (buf[511] != '!') return 1;

    return 0;
}
