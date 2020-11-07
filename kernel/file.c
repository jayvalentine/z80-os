#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <syscall.h>

#include "file.h"
#include "defs.h"

#define CLUSTER_EOF 0xffff

uint16_t current_cache_sector;

DiskInfo_T disk_info;

/* Temporary storage for sector read/written from/to disk. */
char temp_sector[512];

/* File descriptor table. */
#define FILE_LIMIT 1
FileDescriptor_T fdtable[FILE_LIMIT];

/* Helper functions. */

/* Get an unsigned integer (in Z80 little-endian)
 * from a buffer at the given position.
 */
uint16_t get_uint16_t(char * buf, size_t i)
{
    uint16_t hi = buf[i+1];
    uint16_t lo = buf[i];
    return (hi << 8) | lo;
}

/* Get an unsigned long (in little-endian)
 * from a buffer at a given location.
 */
uint32_t get_uint32_t(char* buf, size_t i)
{
    uint32_t hi_hi = buf[i+3];
    uint32_t hi_lo = buf[i+2];
    uint32_t lo_hi = buf[i+1];
    uint32_t lo_lo = buf[i];

    return (hi_hi << 24) | (hi_lo << 16) | (lo_hi << 8) | lo_lo;
}

void read_sector_cached(char * buf, uint32_t sector)
{
    /* Only read the sector if it's not already in cache. */
    if (current_cache_sector != sector)
    {
        syscall_dread(buf, sector);
        current_cache_sector = sector;
    }
}

void fdtable_init()
{
    /* Clear the "used" flag for each fd. */
    for (size_t i = 0; i < FILE_LIMIT; i++)
    {
        fdtable[i].flags = 0x00;
    }
}

int filesystem_init()
{
    syscall_dread(temp_sector, 0);

    /* General disk info. */
    disk_info.bytes_per_sector = get_uint16_t(temp_sector, 0x0b);
    disk_info.sectors_per_cluster = temp_sector[0x0d];
    disk_info.bytes_per_cluster = disk_info.bytes_per_sector * disk_info.sectors_per_cluster;

    /* Calculate start of FAT. */
    disk_info.fat_region = get_uint16_t(temp_sector, 0x0e);

    uint16_t sectors_per_fat = get_uint16_t(temp_sector, 0x16);
    uint16_t number_of_fats = temp_sector[0x10];

    /* Calculate start of root directory. */
    disk_info.root_region = disk_info.fat_region + (sectors_per_fat * number_of_fats);

    uint16_t root_directory_size = get_uint16_t(temp_sector, 0x11) / 16;

    /* Calculate start of data region. */
    disk_info.data_region = disk_info.root_region + root_directory_size;

    /* Calculate number of sectors on disk. */
    disk_info.num_sectors = get_uint16_t(temp_sector, 0x13);

    /* If the small number of sectors is 0, read the large number. */
    if (disk_info.num_sectors == 0)
    {
        disk_info.num_sectors = get_uint32_t(temp_sector, 0x20);
    }

    /* Technically, the first sector _is_ in the cache. */
    current_cache_sector = 0;

    /* Initialise file descriptor table. */
    fdtable_init();

    return 0;
}

void filesystem_filename(char * buf, const char * dir_entry)
{
    uint8_t i;

    memcpy(buf, &dir_entry[0], 8);

    /* Get location of first space in filename. This is where we put the '.' */
    for (i = 0; i < 8; i++)
    {
        if (buf[i] == ' ') break;
    }

    buf[i] = '.';

    /* Copy extension too. */
    memcpy(&buf[i+1], &dir_entry[8], 3);

    /* Check the separator for spaces. */
    uint8_t ext_start = i + 1;
    for (i = ext_start; i < ext_start + 3; i++)
    {
        if (buf[i] == ' ') break;
    }

    buf[i] = '\0';
}

int filesystem_directory_entry(char * dir_entry, const char * filename)
{
    uint32_t sector = disk_info.root_region;
    bool done = FALSE;

    /* Buffer for filename. */
    uint8_t buf[13];

    /* Try to find the file in the root directory. */
    while (!done)
    {
        /* Read the sector. */
        read_sector_cached(temp_sector, sector);

        /* Iterate over the files, looking for the one we want. */
        for (uint16_t f = 0; f < 512; f += 32)
        {
            /* If first byte is 0, we've reached the end of the root directory,
             * but not found the file. */
            if (temp_sector[f] == 0) return E_FILENOTFOUND;

            /* If the first byte is e5, this entry is free, so we should skip it. */
            if (temp_sector[f] == 0xe5) continue;

            /* Otherwise this could be a file.
             * Read the attribute bytes to find out. */
            uint8_t attr = temp_sector[f+11];

            /* Ignore directories and volume labels. */
            if (attr & 0b00011000) continue;

            /* We now know this is a file. Load the filename and compare. */
            filesystem_filename(buf, &temp_sector[f]);

            /* Now we can compare the filename. */
            if (strcmp(buf, filename) == 0)
            {
                /* We've found the file! */
                memcpy(dir_entry, &temp_sector[f], 32);
                done = TRUE;
                break;
            }
        }

        sector++;
    }

    return 0;
}

uint16_t fat_next_cluster(uint16_t cluster)
{
    /* Calculate offset into FAT of sector to read. */
    uint32_t fat_offset = (cluster * 2) / disk_info.bytes_per_sector;
    uint32_t fat_sector = disk_info.fat_region + fat_offset;

    /* Calculate offset into that sector of the entry we want. */
    uint16_t entry = (cluster * 2) % disk_info.bytes_per_sector;

    /* Read the sector and return the appropriate entry. */
    read_sector_cached(temp_sector, fat_sector);
    return get_uint16_t(temp_sector, entry);
}

/* Given a file cluster, return the sector in which that cluster starts. */
uint32_t file_start_sector(uint16_t cluster)
{
    return disk_info.data_region + ((cluster - 2) * disk_info.sectors_per_cluster);
}

int filesystem_assign_fd(void)
{
    for (size_t i = 0; i < FILE_LIMIT; i++)
    {
        if (fdtable[i].flags & FD_FLAGS_CLAIMED) continue;
        return i;
    }

    return E_FILELIMIT;
}

/* Open the file with the given name and mode. */
int file_open(const char * filename, uint8_t mode)
{
    int error;

    /* Find a free file descriptor. */
    error = filesystem_assign_fd();
    if (error < 0) return error;

    /* File descriptor is valid. */
    int fd = error;
    FileDescriptor_T * file = &fdtable[fd];

    /* Buffer into which we'll copy the file entry. */
    uint8_t file_entry[32];

    /* Get relevent file entry. */
    error = filesystem_directory_entry(file_entry, filename);
    if (error != 0) return error;

    /* Get size in bytes. */
    file->size = get_uint32_t(file_entry, 0x1c);

    /* Get start cluster. */
    file->start_cluster = get_uint16_t(file_entry, 0x1a);
    file->current_cluster = file->start_cluster;

    /* Sector (relative to cluster start) */
    file->sector = 0;

    /* Set current position to start-of-file. */
    file->fpos_within_sector = 0;
    file->fpos = 0;

    return fd;
}

/* Close the file indicated by the given file descriptor. */
void file_close(int fd)
{
    FileDescriptor_T * file = &fdtable[fd];
    file->flags &= ~FD_FLAGS_CLAIMED;
}

int file_readbyte(int fd)
{
    FileDescriptor_T * file = &fdtable[fd];

    /* Return EOF if we've hit the end of the file. */
    if (file->fpos == file->size) return EOF;

    /* Shouldn't happen (maybe?) but return EOF if we have no more
     * clusters to read. */
    if (file->current_cluster == CLUSTER_EOF)
    {
        return EOF;
    }

    /* Otherwise read a byte from the current cluster. */

    /* Read current sector. */
    uint32_t sector = file_start_sector(file->current_cluster) + file->sector;

    read_sector_cached(temp_sector, sector);

    /* Get byte. */
    uint8_t byte = temp_sector[file->fpos_within_sector];

    /* Increment size. */
    file->fpos++;
    file->fpos_within_sector++;

    /* If we've reached the end of this sector,
     * we need to fetch the next sector next time around. This may even be
     * in a different cluster. */
    if (file->fpos_within_sector == disk_info.bytes_per_sector)
    {
        file->sector++;
        file->fpos_within_sector = 0;

        /* Do we need the next cluster? */
        if (file->sector == disk_info.sectors_per_cluster)
        {
            file->current_cluster = fat_next_cluster(file->current_cluster);
            file->sector = 0;
        }
    }

    return (int)byte;
}

int file_readsector(char * ptr, int fd)
{
    FileDescriptor_T * file = &fdtable[fd];

    /* Return EOF if we've hit the end of the file. */
    if (file->fpos == file->size) return EOF;

    /* Return EOF if we have no more clusters to read. */
    if (file->current_cluster == CLUSTER_EOF) return EOF;

    /* Otherwise read a full sector. */

    /* Read current sector. */
    uint32_t sector = file_start_sector(file->current_cluster) + file->sector;

    /* Don't cache the sector - we're unlikely to read it again. */
    syscall_dread(ptr, sector);

    /* Increment size. fpos_within_sector doesn't change because we've read an entire sector. */
    file->fpos += disk_info.bytes_per_sector;

    /* We know we've reached the end of the sector, so we need to fetch the next one
     * next time around. This may be in a different cluster. */
    file->sector++;

    /* Do we need the next cluster? */
    if (file->sector == disk_info.sectors_per_cluster)
    {
        file->current_cluster = fat_next_cluster(file->current_cluster);
        file->sector = 0;
    }

    return 0;
}

size_t file_read(char * ptr, size_t n, int fd)
{
    /* Guard against an obviously invalid descriptor, that would cause
     * us to index out of the fdtable. */
    if (fd < 0 || fd >= FILE_LIMIT) return E_INVALIDDESCRIPTOR;

    size_t bytes = 0;

    /* Get the entry associated with this file descriptor */
    FileDescriptor_T * file = &fdtable[fd];

    /* Read byte-by-byte up to the first sector boundary. */
    while (file->fpos_within_sector != 0)
    {
        int c = file_readbyte(fd);

        if (c == EOF) return bytes;
        *ptr = (uint8_t)c;
        
        ptr++;
        bytes++;
        n--;
    }

    /* How many full sectors do we need to read? */
    size_t full_sectors = n / disk_info.bytes_per_sector;

    /* Read 0 or more *full* sectors. */
    for (size_t i = 0; i < full_sectors; i++)
    {
        int c = file_readsector(ptr, fd);

        /* We _shouldn't_ ever hit EOF part-way through a sector,
         * so if readsector returns EOF then we didn't read the sector at all. */
        if (c == EOF) return bytes;

        /* Otherwise we've read a full sector. */
        ptr += disk_info.bytes_per_sector;
        bytes += disk_info.bytes_per_sector;
        n -= disk_info.bytes_per_sector;
    }

    /* We must now have <X bytes remaining (where X is the number of bytes per sector). */
    for (size_t i = 0; i < n; i++)
    {
        int c = file_readbyte(fd);
        if (c == EOF) return bytes;

        *ptr = (uint8_t)c;
        ptr++;
        bytes++;
    }

    return bytes;
}
