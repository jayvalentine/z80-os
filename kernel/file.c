#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include <include/file.h>

#define CLUSTER_EOF 0xffff
#define CLUSTER_FREE 0x0000

void disk_read(char * buf, uint32_t sector);
void disk_write(char * buf, uint32_t sector);

uint32_t current_cache_sector;

DiskInfo_T disk_info;

/* Temporary storage for sector read/written from/to disk. */
char temp_sector[512];

/* File descriptor table. */
FileDescriptor_T fdtable[FILE_LIMIT];

/* In-place "upper-cases" the given string. */
void string_toupper(char * s)
{
    while (*s != '\0')
    {
        if (*s >= 'a' && *s <= 'z')
        {
            *s -= ('a' - 'A');
        }
        s++;
    }
}

#define GET_UINT16(_buf, _i) (*(uint16_t *)&_buf[_i])
#define GET_UINT32(_buf, _i) (*(uint32_t *)&_buf[_i])

void read_sector_cached(char * buf, uint32_t sector, bool force_load)
{
    /* Only read the sector if it's not already in cache. */
    /* OR... if we've forced a load. */
    if (force_load || (current_cache_sector != sector))
    {
        disk_read(buf, sector);
        current_cache_sector = sector;
    }
}

void fdtable_init(void)
{
    /* Clear the "used" flag for each fd. */
    for (size_t i = 0; i < FILE_LIMIT; i++)
    {
        fdtable[i].flags = 0x00;
    }
}

void filesystem_calc_fat_region(void)
{
    disk_info.fat_region = GET_UINT16(temp_sector, 0x0e);
}

void filesystem_calc_root_region(void)
{
    static uint16_t sectors_per_fat;
    static uint16_t number_of_fats;

    sectors_per_fat = GET_UINT16(temp_sector, 0x16);
    number_of_fats = (uint16_t)temp_sector[0x10];

    /* Calculate start of root directory. */
    disk_info.root_region = disk_info.fat_region + (sectors_per_fat * number_of_fats);
}

void filesystem_calc_data_region(void)
{
    static uint16_t root_directory_size;

    root_directory_size = GET_UINT16(temp_sector, 0x11) / 16;

    /* Calculate start of data region. */
    disk_info.data_region = disk_info.root_region + root_directory_size;
}

void filesystem_calc_num_sectors(void)
{
    /* Calculate number of sectors on disk. */
    disk_info.num_sectors = GET_UINT16(temp_sector, 0x13);

    /* If the small number of sectors is 0, read the large number. */
    if (disk_info.num_sectors == 0)
    {
        disk_info.num_sectors = GET_UINT32(temp_sector, 0x20);
    }
}

int filesystem_init(void)
{
    disk_read(temp_sector, 0ul);

    /* General disk info. */
    disk_info.bytes_per_sector = GET_UINT16(temp_sector, (size_t)0x0b);
    disk_info.sectors_per_cluster = temp_sector[0x0d];
    disk_info.bytes_per_cluster = disk_info.bytes_per_sector * (uint16_t)disk_info.sectors_per_cluster;

    /* Calculate FAT info. */
    filesystem_calc_fat_region();
    filesystem_calc_root_region();
    filesystem_calc_data_region();
    filesystem_calc_num_sectors();

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

int filesystem_get_directory_entry(DirectoryEntry_T * dir_entry, const char * filename)
{
    static uint32_t sector;

    sector = disk_info.root_region;
    
    bool done = false;

    /* Buffer for filename. */
    char buf[FILENAME_BUF_LEN];

    /* Try to find the file in the root directory. */
    while (!done)
    {
        /* Read the sector. */
        read_sector_cached(temp_sector, sector, true);

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
                memcpy((char *) dir_entry, &temp_sector[f], 32);
                done = true;
                break;
            }
        }

        sector++;
    }

    return 0;
}

int filesystem_set_directory_entry(const DirectoryEntry_T * dir_entry, const char * filename)
{
    static uint32_t sector;
    
    sector = disk_info.root_region;
    bool done = false;

    /* Buffer for filename. */
    char buf[FILENAME_BUF_LEN];

    /* Try to find the file in the root directory. */
    while (!done)
    {
        /* Read the sector. */
        read_sector_cached(temp_sector, sector, true);

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
                memcpy(&temp_sector[f], (char *) dir_entry, 32);
                disk_write(temp_sector, sector);
                done = true;
                break;
            }
        }

        sector++;
    }

    return 0;
}

int filesystem_mark_directory_entry_free(const char * filename)
{
    static uint32_t sector;
    
    sector = disk_info.root_region;
    bool done = false;

    /* Buffer for filename. */
    char buf[FILENAME_BUF_LEN];

    /* Try to find the file in the root directory. */
    while (!done)
    {
        /* Read the sector. */
        read_sector_cached(temp_sector, sector, true);

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
                /* Put 0xe5 in the first character of the filename to mark
                 * this entry as free. */
                temp_sector[f] = 0xe5u;
                disk_write(temp_sector, sector);
                done = true;
                break;
            }
        }

        sector++;
    }

    return 0;
}

uint16_t fat_next_cluster(uint16_t cluster)
{
    static uint32_t fat_offset;
    static uint32_t fat_sector;
    static uint16_t cluster_bytes;

    cluster_bytes = cluster * 2;
    
    /* Calculate offset into FAT of sector to read. */
    fat_offset = cluster_bytes / disk_info.bytes_per_sector;
    fat_sector = disk_info.fat_region + fat_offset;

    /* Calculate offset into that sector of the entry we want. */
    uint16_t entry = cluster_bytes % disk_info.bytes_per_sector;

    /* Read the sector and return the appropriate entry. */
    read_sector_cached(temp_sector, fat_sector, false);
    return GET_UINT16(temp_sector, entry);
}

void fat_set_cluster(uint16_t cluster, uint16_t next_cluster)
{
    static uint32_t fat_offset;
    static uint32_t fat_sector;
    static uint16_t cluster_bytes;

    cluster_bytes = cluster * 2;
    
    /* Calculate offset into FAT of sector to read. */
    fat_offset = cluster_bytes / disk_info.bytes_per_sector;
    fat_sector = disk_info.fat_region + fat_offset;

    /* Calculate offset into that sector of the entry we want. */
    uint16_t entry = cluster_bytes % disk_info.bytes_per_sector;

    /* Read the sector and set the appropriate entry. */
    read_sector_cached(temp_sector, fat_sector, false);
    GET_UINT16(temp_sector, entry) = next_cluster;
    disk_write(temp_sector, fat_sector);
}

uint16_t fat_find_free_cluster(void)
{
    static uint32_t fat_sector;
    static uint16_t entry;
    static uint16_t allocated_cluster;
    static uint16_t entry_within_sector;

    /* Find first free entry in FAT. */
    allocated_cluster = 2;
    entry_within_sector = allocated_cluster;

    fat_sector = disk_info.fat_region;

    entry = CLUSTER_EOF;
    
    while (true)
    {
        read_sector_cached(temp_sector, fat_sector, false);
        entry = GET_UINT16(temp_sector, entry_within_sector * 2);

        if (entry == CLUSTER_FREE) break;

        allocated_cluster++;
        entry_within_sector++;

        /* If we've gone past this sector, get the next one. */
        if (entry_within_sector > (disk_info.bytes_per_sector / 2))
        {
            entry_within_sector = 0;
            fat_sector++;
        }

        /* If we're now past the FAT then we don't have any free entries. */
        if (fat_sector == disk_info.root_region) break;
    }

    /* If we leave the loop and the entry is not free, we've run out of FAT.
     * In this case we return 0, which isn't a valid cluster.
     * Otherwise we return the cluster which has a free entry.
     */
    if (entry != 0) return 0;
    return allocated_cluster;
}

uint16_t fat_allocate_start_cluster(void)
{
    uint16_t free_cluster = fat_find_free_cluster();

    /* No more free clusters :( */
    if (free_cluster == 0) return 0;

    /* Set this cluster to EOF. */
    fat_set_cluster(free_cluster, CLUSTER_EOF);
    return free_cluster;
}

uint16_t fat_allocate_cluster(uint16_t cluster)
{
    uint16_t next_cluster = fat_find_free_cluster();

    if (next_cluster == 0) return 0;
    else
    {
        /* Set this cluster to point to the newly allocated one. */
        fat_set_cluster(cluster, next_cluster);

        /* Set the newly allocated cluster to be the last one. */
        fat_set_cluster(next_cluster, CLUSTER_EOF);
        return next_cluster;
    }
}

void fat_deallocate_cluster(uint16_t cluster)
{
    fat_set_cluster(cluster, CLUSTER_FREE);
}

/* Given a file cluster, return the sector in which that cluster starts. */
uint32_t file_start_sector(uint16_t cluster)
{
    return disk_info.data_region + (cluster - 2) * disk_info.sectors_per_cluster;
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

int file_create(DirectoryEntry_T * entry)
{
    static uint32_t sector;
    
    sector = disk_info.root_region;

    while (true)
    {
        read_sector_cached(temp_sector, sector, false);

        for (size_t f = 0; f < 512; f += 32)
        {
            /* Free entry? */
            if (temp_sector[f] == (char)0 || temp_sector[f] == (char)0xe5)
            {
                /* Yes, copy file entry and write back to disk. */
                memcpy(&temp_sector[f], (char *)entry, 32);
                disk_write(temp_sector, sector);
                return 0;
            }
        }

        /* Increment sector, stop if we've hit the data region. */
        sector++;
        if (sector == disk_info.data_region)
        {
            return 1;
        }
    }
}

/* Open the file with the given name and mode. */
int file_open_read(FileDescriptor_T * file)
{
    int error;

    /* Buffer into which we'll copy the file entry. */
    DirectoryEntry_T file_entry;

    /* Get relevent file entry. */
    error = filesystem_get_directory_entry(&file_entry, file->name);
    if (error != 0) return error;

    /* Get size in bytes. */
    file->size = file_entry.size;

    /* Get start cluster. */
    file->start_cluster = file_entry.starting_cluster;
    file->current_cluster = file->start_cluster;

    /* Sector (relative to cluster start) */
    file->sector = 0;

    /* Set current position to start-of-file. */
    file->fpos_within_sector = 0;
    file->fpos = 0;

    /* Set mode. */
    file->mode = FMODE_READ;

    return 0;
}

/* Open the file with the given name and mode. */
int file_open_write(FileDescriptor_T * file)
{
    /* Create local directory entry. */
    DirectoryEntry_T file_entry;

    /* Make sure the file doesn't already exist. */
    if (filesystem_get_directory_entry(&file_entry, file->name) != E_FILENOTFOUND)
    {
        return E_FILEEXIST;
    }

    /* Try to get a free cluster. */
    uint16_t start_cluster = fat_allocate_start_cluster();
    if (start_cluster == 0) return E_DISKFULL;

    /* Copy filename into a buffer to extract name and extension. */
    char filename_tmp[13];
    strcpy(filename_tmp, file->name);

    char * sep_pos = strchr(filename_tmp, '.');
    if (sep_pos == NULL) return E_INVALIDFILENAME;

    *sep_pos = '\0';

    char * entry_filename = filename_tmp;
    char * entry_ext = sep_pos + 1;

    /* Copy filename into directory entry, padding with spaces. */
    memcpy((char *)(file_entry.name), (const char *)entry_filename, strlen(entry_filename));
    for (size_t i = strlen(entry_filename); i < 8; i++) file_entry.name[i] = ' ';

    /* Copy extension into directory entry, padding with spaces. */
    memcpy((char *)(file_entry.ext), (const char *)entry_ext, strlen(entry_ext));
    for (size_t i = strlen(entry_ext); i < 3; i++) file_entry.ext[i] = ' ';

    /* Don't set any of the attributes. */
    file_entry.attributes = 0x00;

    /* Reserved for Windows NT. We're not Windows NT, so we don't care :) */
    file_entry.reserved_for_windows_nt = 0x00;

    /* Created at the dawn of time, for now! */
    /* FIXME: Actual creation date/time, once we can know what it is... */
    file_entry.creation_milliseconds = 0x00;
    file_entry.creation_time = 0x0000;
    file_entry.creation_date = 0x0021; /* Created: 1980-01-01... */

    /* Last access date; also the dawn of time. */
    file_entry.last_access_date = 0x0000;

    /* Reserved for FAT32. We set this field to 0. */
    file_entry.reserved_for_fat32 = 0x0000;

    /* Last write time. */
    file_entry.last_write_time = 0x0000;
    file_entry.last_write_date = 0x0000;

    /* We know the starting cluster must be valid. */
    file_entry.starting_cluster = start_cluster;

    /* Size is initially 0. */
    file_entry.size = 0;

    /* Try to create a directory entry. */
    if (file_create(&file_entry))
    {
        return E_DIRFULL;
    }

    /* Get size in bytes. */
    file->size = file_entry.size;

    /* Get start cluster. */
    file->start_cluster = file_entry.starting_cluster;
    file->current_cluster = file->start_cluster;

    /* Sector (relative to cluster start) */
    file->sector = 0;

    /* Set current position to start-of-file. */
    file->fpos_within_sector = 0;
    file->fpos = 0;

    /* Set mode. */
    file->mode = FMODE_WRITE;

    return 0;
}

int file_open(const char * filename, uint8_t mode)
{
    /* Truncate and upper-case the filename. */
    char filename_upper[13];
    memcpy(filename_upper, filename, 12);
    filename_upper[12] = '\0';
    string_toupper(filename_upper);

    /* Try to find a free file descriptor. */
    int fd = filesystem_assign_fd();
    if (fd < 0) return fd;

    FileDescriptor_T * file = &fdtable[fd];
    strcpy(file->name, filename_upper);

    int error;

    switch (mode)
    {
        case FMODE_READ:    error = file_open_read(file); break;
        case FMODE_WRITE:   error = file_open_write(file); break;
        default: return E_INVALIDMODE;
    }

    if (error < 0)
    {
        /* Free the file descriptor and return the error code. */
        file->flags &= ~FD_FLAGS_CLAIMED;
        return error;
    }

    return fd;
}

/* Close the file indicated by the given file descriptor. */
void file_close(int fd)
{
    static DirectoryEntry_T entry;

    FileDescriptor_T * file = &fdtable[fd];

    /* Update size in directory entry, if opened for writing. */
    if (file->mode == FMODE_WRITE)
    {
        filesystem_get_directory_entry(&entry, file->name);
        entry.size = file->size;
        filesystem_set_directory_entry(&entry, file->name);
    }

    file->flags &= ~FD_FLAGS_CLAIMED;
}

/* Create a new file with the given name. */
int file_new(const char * filename)
{
    /* Truncate and upper-case the filename. */
    char filename_upper[13];
    memcpy(filename_upper, filename, 12);
    filename_upper[12] = '\0';
    string_toupper(filename_upper);

    /* Try to find a free file descriptor. */
    int fd = filesystem_assign_fd();
    if (fd < 0) return fd;

    FileDescriptor_T * file = &fdtable[fd];
    strcpy(file->name, filename_upper);

    int error = file_open_write(file);

    /* We always free the file descriptor because
     * we are only creating the file.
     */
    file->flags &= ~FD_FLAGS_CLAIMED;

    return error;
}

int file_delete(const char * filename)
{
    /* Truncate and upper-case the filename. */
    char filename_upper[13];
    memcpy(filename_upper, filename, 12);
    filename_upper[12] = '\0';
    string_toupper(filename_upper);

    /* Get start cluster of file. */
    DirectoryEntry_T direntry;

    int error = filesystem_get_directory_entry(&direntry, filename_upper);

    if (error != 0) return error;

    uint16_t cluster = direntry.starting_cluster;

    /* De-allocate each cluster allocated to this file. */
    while (cluster != CLUSTER_EOF)
    {
        /* Get the next allocated cluster after this one. */
        uint16_t next_cluster = fat_next_cluster(cluster);

        /* Allocate *this* cluster. */
        fat_deallocate_cluster(cluster);

        /* Now move onto the next allocated cluster. */
        cluster = next_cluster;
    }

    error = filesystem_mark_directory_entry_free(filename_upper);

    return error;
}

int file_readbyte(int fd)
{
    static uint32_t sector;

    FileDescriptor_T * file = &fdtable[fd];

    /* Return EOF if we've hit the end of the file. */
    if (file->fpos == file->size) return KERNEL_EOF;

    /* Shouldn't happen (maybe?) but return EOF if we have no more
     * clusters to read. */
    if (file->current_cluster == CLUSTER_EOF)
    {
        return KERNEL_EOF;
    }

    /* Otherwise read a byte from the current cluster. */

    /* Read current sector. */
    sector = file_start_sector(file->current_cluster) + file->sector;

    read_sector_cached(temp_sector, sector, false);

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
    static uint32_t sector;

    FileDescriptor_T * file = &fdtable[fd];

    /* Return EOF if we've hit the end of the file. */
    if (file->fpos == file->size) return KERNEL_EOF;

    /* Return EOF if we have no more clusters to read. */
    if (file->current_cluster == CLUSTER_EOF) return KERNEL_EOF;

    /* Otherwise read a full sector. */

    /* Read current sector. */
    sector = file_start_sector(file->current_cluster) + file->sector;

    /* Don't cache the sector - we're unlikely to read it again. */
    disk_read(ptr, sector);

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

int file_writesector(char * ptr, size_t offset, size_t n, int fd)
{
    static uint32_t sector;

    /* Can't write past end of sector. */
    if (offset + n > disk_info.bytes_per_sector) return 1;

    FileDescriptor_T * file = &fdtable[fd];

    /* Read current sector. */
    sector = file_start_sector(file->current_cluster) + file->sector;

    read_sector_cached(temp_sector, sector, false);

    /* Overwrite bytes in memory, write back to disk. */
    memcpy(temp_sector + offset, ptr, n);
    disk_write(temp_sector, sector);

    /* Increment size. fpos_within_sector doesn't change because we've read an entire sector. */
    file->fpos += n;
    file->size += n;
    file->fpos_within_sector += n;

    if (file->fpos_within_sector == disk_info.bytes_per_sector)
    {
        file->fpos_within_sector = 0;
        file->sector++;

        /* Do we need the next cluster? If so we need to allocate one (if it's not already) */
        if (file->sector == disk_info.sectors_per_cluster)
        {
            uint16_t next_cluster = fat_next_cluster(file->current_cluster);

            if (next_cluster == CLUSTER_EOF)
            {
                next_cluster = fat_allocate_cluster(file->current_cluster);

                /* Check that the allocation succeeded. If not we're out of disk space :( */
                if (next_cluster == 0) return E_DISKFULL;
            }

            file->current_cluster = next_cluster;
            file->sector = 0;
        }
    }

    return 0;
}

size_t file_write(char * ptr, size_t n, int fd)
{
    /* Guard against an obviously invalid descriptor, that would cause
     * us to index out of the fdtable. */
    if (fd < 0 || fd >= FILE_LIMIT) return 0;

    size_t bytes = 0;

    /* Get the entry associated with this file descriptor */
    FileDescriptor_T * file = &fdtable[fd];

    /* Make sure the file has been opened for writing. */
    if (file->mode != FMODE_WRITE) return 0;

    /* Partial write of the first sector. */
    if (file->fpos_within_sector != 0)
    {
        /* Don't want to write past end of sector with this partial write. */
        size_t limit = disk_info.bytes_per_sector - file->fpos_within_sector;

        size_t partial_write_size = (n < limit) ? n : limit;
        int e = file_writesector(ptr, file->fpos_within_sector, partial_write_size, fd);
        if (e != 0) return 0;

        ptr += partial_write_size;
        bytes += partial_write_size;
        n -= partial_write_size;
    }

    if (n == 0) return bytes;

    /* How many full sectors do we need to write? */
    size_t full_sectors = n / disk_info.bytes_per_sector;

    /* Write 0 or more *full* sectors. */
    for (size_t i = 0; i < full_sectors; i++)
    {
        int e = file_writesector(ptr, 0, disk_info.bytes_per_sector, fd);
        if (e != 0) return 0;

        /* We've now written a full sector. */
        ptr += disk_info.bytes_per_sector;
        bytes += disk_info.bytes_per_sector;
        n -= disk_info.bytes_per_sector;
    }

    if (n == 0) return bytes;

    /* We must now have <X bytes remaining (where X is the number of bytes per sector),
     * and be aligned on a sector boundary. */
    file_writesector(ptr, 0, n, fd);
    bytes += n;

    return bytes;
}

size_t file_read(char * ptr, size_t n, int fd)
{
#ifndef UNIT_TEST
    if (((uint16_t)ptr) < 0x6000) return 0;
#endif

    /* Guard against an obviously invalid descriptor, that would cause
     * us to index out of the fdtable. */
    if (fd < 0 || fd >= FILE_LIMIT) return 0;

    size_t bytes = 0;

    /* Get the entry associated with this file descriptor */
    FileDescriptor_T * file = &fdtable[fd];

    /* Make sure the file has been opened for reading. */
    if (file->mode != FMODE_READ) return 0;

    /* If the number of bytes left to read is <n, set n=number of bytes left */
    if ((file->size - file->fpos) < n)
    {
        n = file->size - file->fpos;
    }

    /* Read byte-by-byte up to the first sector boundary. */
    while (file->fpos_within_sector != 0)
    {
        int c = file_readbyte(fd);

        if (c == KERNEL_EOF) return bytes;
        *ptr = (uint8_t)c;
        
        ptr++;
        bytes++;
        n--;

        if (n == 0) return bytes;
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

    if (n == 0) return bytes;

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

int file_info(const char * filename, FINFO * finfo)
{
    /* Truncate and upper-case the filename. */
    char filename_upper[13];
    memcpy(filename_upper, filename, 12);
    filename_upper[12] = '\0';
    string_toupper(filename_upper);

    DirectoryEntry_T direntry;
    
    /* Search for the file in the directory. */
    /* Return error code if it does not exist. */
    int error = filesystem_get_directory_entry(&direntry, filename_upper);
    if (error != 0) return error;

    finfo->attr = direntry.attributes;

    uint16_t creation_date = direntry.creation_date;

    finfo->created_year = 1980 + (creation_date >> 9);
    finfo->created_month = (creation_date >> 5) & 0x000f;
    finfo->created_day = creation_date & 0x001f;

    finfo->size = direntry.size;

    return 0;
}

/* Returns the number of file entries in the root directory. */
uint16_t file_entries(void)
{
    static uint32_t sector;

    uint16_t entries = 0;

    sector = disk_info.root_region;
    
    /* Try to find the file in the root directory. */
    while (sector != disk_info.data_region)
    {
        /* Read the sector. */
        read_sector_cached(temp_sector, sector, true);

        /* Iterate over the files, looking for the one we want. */
        for (uint16_t f = 0; f < 512; f += 32)
        {
            /* If first byte is 0, we've reached the end of the root directory. */
            if (temp_sector[f] == 0) return entries;

            /* If the first byte is e5, this entry is free, so we should skip it. */
            if (temp_sector[f] == (char)0xe5) continue;

            /* Otherwise this could be a file.
             * Read the attribute bytes to find out. */
            uint8_t attr = temp_sector[f+11];

            /* Ignore directories and volume labels. */
            if (attr & 0b00011000) continue;

            /* We now know this is a file. Increment the count. */
            entries++;
        }

        sector++;
    }

    return entries;
}

/* Gets the name of the nth entry in the root directory.
 * Returns 0 on success (indicating the string is valid)
 * or an error code on failure (indicating the string is invalid). */
int file_entry(char * s, uint16_t entry)
{
    static uint32_t sector;

    sector = disk_info.root_region;
    
    uint16_t n = 0;

    /* Try to find the file in the root directory. */
    while (sector != disk_info.data_region)
    {
        /* Read the sector. */
        read_sector_cached(temp_sector, sector, true);

        /* Iterate over the files, looking for the one we want. */
        for (uint16_t f = 0; f < 512; f += 32)
        {
            /* If first byte is 0, we've reached the end of the root directory. */
            if (temp_sector[f] == 0) return E_FILENOTFOUND;

            /* If the first byte is e5, this entry is free, so we should skip it. */
            if (temp_sector[f] == (char)0xe5) continue;

            /* Otherwise this could be a file.
             * Read the attribute bytes to find out. */
            uint8_t attr = temp_sector[f+11];

            /* Ignore directories and volume labels. */
            if (attr & 0b00011000) continue;

            /* We now know this is a file. If this is the nth entry we've seen,
             * populate the filename string and return. */
            if (n == entry)
            {
                filesystem_filename(s, &temp_sector[f]);
                return 0;
            }

            /* Otherwise increment the count. */
            n++;
        }

        sector++;
    }

    return E_FILENOTFOUND;
}
