#include <syscall.h>
#include <string.h>

#define DRIVE_SECTOR_COUNT 1024
#define DRIVE_SECTOR_SIZE 512
#define DRIVE_SECTORS_PER_CLUSTER 8

/* Mock a "drive" - just a sequential series of sectors. */
uint8_t drive[DRIVE_SECTOR_COUNT][DRIVE_SECTOR_SIZE];

/* Extern the "diskinfo" struct. */
extern DiskInfo_T disk_info;

/* Forward-declare fdtable init function. */
void fdtable_init();

void disk_write(char * buf, uint32_t sector)
{
    if (sector < DRIVE_SECTOR_COUNT) memcpy(drive[sector], buf, DRIVE_SECTOR_SIZE);
}

void disk_read(char * buf, uint32_t sector)
{
    if (sector < DRIVE_SECTOR_COUNT) memcpy(buf, drive[sector], DRIVE_SECTOR_SIZE);
}

const DiskInfo_T * syscall_dinfo(void)
{
    return &disk_info;
}

void mock_drive_init(void)
{
    memset(drive, 0, DRIVE_SECTOR_COUNT*DRIVE_SECTOR_SIZE);

    disk_info.bytes_per_sector = DRIVE_SECTOR_SIZE;
    disk_info.sectors_per_cluster = DRIVE_SECTORS_PER_CLUSTER;
    disk_info.bytes_per_cluster = disk_info.bytes_per_sector * disk_info.sectors_per_cluster;

    disk_info.fat_region = 0;

    uint16_t sectors_per_fat = 64;
    uint16_t number_of_fats = 2;

    /* Calculate start of root directory. */
    disk_info.root_region = disk_info.fat_region + (sectors_per_fat * number_of_fats);

    uint16_t root_directory_size = DRIVE_SECTOR_SIZE / 16;

    /* Calculate start of data region. */
    disk_info.data_region = disk_info.root_region + root_directory_size;

    /* Calculate number of sectors on disk. */
    disk_info.num_sectors = DRIVE_SECTOR_COUNT;

    /* Initialise file descriptor table. */
    fdtable_init();
}
