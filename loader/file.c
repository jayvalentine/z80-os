#include "file.h"
#include "syscall_wrappers.h"
#include "utils.h"

#define CLUSTER_EOF 0xffff

/* Information about the disk. Needed for filesystem interaction. */
struct DiskInfo_T
{
    /* Region start sectors. */
    ulong fat_region;
    ulong root_region;
    ulong data_region;

    /* Other info. */
    uint bytes_per_sector;
    ubyte sectors_per_cluster;
    uint bytes_per_cluster;
    ulong num_sectors;

    /* Caching info. */
    /* Last sector read into cache. */
    uint current_cache_sector;
} disk_info;

/* Temporary storage for sector read/written from/to disk. */

/* End of image, when loaded into memory. We can use this to address free RAM
 * without having to declare it in the image. */
extern char _tail;
char * temp_sector;

/* Helper functions. */

/* Get an unsigned integer (in Z80 little-endian)
 * from a buffer at the given position.
 */
uint get_uint(char * buf, uint i)
{
    uint var;
    ubyte * var_ptr = (ubyte *)&var;

    var_ptr[1] = buf[i+1];
    var_ptr[0] = buf[i];

    return var;
}

/* Get an unsigned long (in little-endian)
 * from a buffer at a given location.
 */
ulong get_ulong(char* buf, uint i)
{
    ulong var;
    ubyte * var_ptr = (ubyte *)&var;

    var_ptr[3] = buf[i+3];
    var_ptr[2] = buf[i+2];
    var_ptr[1] = buf[i+1];
    var_ptr[0] = buf[i];

    return var;
}

void read_sector_cached(char * buf, ulong sector)
{
    /* Only read the sector if it's not already in cache. */
    if (disk_info.current_cache_sector != sector)
    {
        syscall_dread(buf, sector);
        disk_info.current_cache_sector = sector;
    }
}

int filesystem_init()
{
    temp_sector = &_tail;
    syscall_dread(temp_sector, 0);

    /* General disk info. */
    disk_info.bytes_per_sector = get_uint(temp_sector, 0x0b);
    disk_info.sectors_per_cluster = temp_sector[0x0d];
    disk_info.bytes_per_cluster = disk_info.bytes_per_sector * disk_info.sectors_per_cluster;

    /* Calculate start of FAT. */
    disk_info.fat_region = get_uint(temp_sector, 0x0e);

    uint sectors_per_fat = get_uint(temp_sector, 0x16);
    uint number_of_fats = temp_sector[0x10];

    /* Calculate start of root directory. */
    disk_info.root_region = disk_info.fat_region + (sectors_per_fat * number_of_fats);

    uint root_directory_size = get_uint(temp_sector, 0x11) / 16;

    /* Calculate start of data region. */
    disk_info.data_region = disk_info.root_region + root_directory_size;

    /* Calculate number of sectors on disk. */
    disk_info.num_sectors = get_uint(temp_sector, 0x13);

    /* If the small number of sectors is 0, read the large number. */
    if (disk_info.num_sectors == 0)
    {
        disk_info.num_sectors = get_ulong(temp_sector, 0x20);
    }

    /* Technically, the first sector _is_ in the cache. */
    disk_info.current_cache_sector = 0;

    return 0;
}

void get_filename(char * buf, const char * dir_entry)
{
    ubyte i;

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
    ubyte ext_start = i + 1;
    for (i = ext_start; i < ext_start + 3; i++)
    {
        if (buf[i] == ' ') break;
    }

    buf[i] = '\0';
}

int get_directory_entry(char * dir_entry, const char * filename)
{
    ulong sector = disk_info.root_region;
    bool done = FALSE;

    /* Buffer for filename. */
    ubyte buf[13];

    /* Try to find the file in the root directory. */
    while (!done)
    {
        /* Read the sector. */
        read_sector_cached(temp_sector, sector);

        /* Iterate over the files, looking for the one we want. */
        for (uint f = 0; f < 512; f += 32)
        {
            /* If first byte is 0, we've reached the end of the root directory,
             * but not found the file. */
            if (temp_sector[f] == 0) return E_FILENOTFOUND;

            /* Otherwise this could be a file.
             * Read the attribute bytes to find out. */
            ubyte attr = temp_sector[f+11];

            /* Ignore directories and volume labels. */
            if (attr & 0b00011000) continue;

            /* We now know this is a file. Load the filename and compare. */
            get_filename(buf, &temp_sector[f]);

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

ulong get_start_sector(uint cluster)
{
    return disk_info.data_region + ((cluster - 2) * disk_info.sectors_per_cluster);
}

uint get_next_cluster(uint cluster)
{
    /* Calculate offset into FAT of sector to read. */
    ulong fat_offset = (cluster * 2) / disk_info.bytes_per_sector;
    ulong fat_sector = disk_info.fat_region + fat_offset;

    /* Calculate offset into that sector of the entry we want. */
    uint entry = (cluster * 2) % disk_info.bytes_per_sector;

    /* Read the sector and return the appropriate entry. */
    read_sector_cached(temp_sector, fat_sector);
    return get_uint(temp_sector, entry);
}

int file_open(const char * filename, File_T * fd)
{
    int error;

    /* Buffer into which we'll copy the file entry. */
    ubyte file_entry[32];

    /* Get relevent file entry. */
    error = get_directory_entry(file_entry, filename);

    if (error != 0) return error;

    /* Get size in bytes. */
    fd->size = get_ulong(file_entry, 0x1c);

    /* Get start cluster. */
    fd->start_cluster = get_uint(file_entry, 0x1a);
    fd->current_cluster = fd->start_cluster;

    /* Sector (relative to cluster start) */
    fd->sector = 0;

    /* Set current position to start-of-file. */
    fd->fpos_within_sector = 0;
    fd->fpos = 0;

    return 0;
}

#ifdef Z88DK
int file_readbyte(File_T * fd) __z88dk_fastcall
#else
int file_readbyte(File_T * fd)
#endif
{
    /* Return EOF if we've hit the end of the file. */
    if (fd->fpos == fd->size) return EOF;

    /* Shouldn't happen (maybe?) but return EOF if we have no more
     * clusters to read. */
    if (fd->current_cluster == CLUSTER_EOF)
    {
        return EOF;
    }

    /* Otherwise read a byte from the current cluster. */

    /* Read current sector. */
    ulong sector = get_start_sector(fd->current_cluster) + fd->sector;

    read_sector_cached(temp_sector, sector);

    /* Get byte. */
    ubyte byte = temp_sector[fd->fpos_within_sector];

    /* Increment size. */
    fd->fpos++;
    fd->fpos_within_sector++;

    /* If we've reached the end of this sector,
     * we need to fetch the next sector next time around. This may even be
     * in a different cluster. */
    if (fd->fpos_within_sector == disk_info.bytes_per_sector)
    {
        fd->sector++;
        fd->fpos_within_sector = 0;

        /* Do we need the next cluster? */
        if (fd->sector == disk_info.sectors_per_cluster)
        {
            fd->current_cluster = get_next_cluster(fd->current_cluster);
            fd->sector = 0;
        }
    }

    return (int)byte;
}

int file_readsector(char * ptr, File_T * fd)
{
    /* Return EOF if we've hit the end of the file. */
    if (fd->fpos == fd->size) return EOF;

    /* Return EOF if we have no more clusters to read. */
    if (fd->current_cluster == CLUSTER_EOF) return EOF;

    /* Otherwise read a full sector. */

    /* Read current sector. */
    ulong sector = get_start_sector(fd->current_cluster) + fd->sector;

    /* Don't cache the sector - we're unlikely to read it again. */
    syscall_dread(ptr, sector);

    /* Increment size. fpos_within_sector doesn't change because we've read an entire sector. */
    fd->fpos += disk_info.bytes_per_sector;

    /* We know we've reached the end of the sector, so we need to fetch the next one
     * next time around. This may be in a different cluster. */
    fd->sector++;

    /* Do we need the next cluster? */
    if (fd->sector == disk_info.sectors_per_cluster)
    {
        fd->current_cluster = get_next_cluster(fd->current_cluster);
        fd->sector = 0;
    }

    return 0;
}

size_t file_read(ubyte * buf, File_T * fd, size_t n)
{
    size_t bytes = 0;

    /* Read byte-by-byte up to the first sector boundary. */
    while (fd->fpos_within_sector != 0)
    {
        int c = file_readbyte(fd);

        if (c == EOF) return bytes;
        *buf = (ubyte)c;
        
        buf++;
        bytes++;
        n--;
    }

    /* How many full sectors do we need to read? */
    size_t full_sectors = n / disk_info.bytes_per_sector;

    /* Read 0 or more *full* sectors. */
    for (size_t i = 0; i < full_sectors; i++)
    {
        int c = file_readsector(buf, fd);

        /* We _shouldn't_ ever hit EOF part-way through a sector,
         * so if readsector returns EOF then we didn't read the sector at all. */
        if (c == EOF) return bytes;

        /* Otherwise we've read a full sector. */
        buf += disk_info.bytes_per_sector;
        bytes += disk_info.bytes_per_sector;
        n -= disk_info.bytes_per_sector;
    }

    /* We must now have <X bytes remaining (where X is the number of bytes per sector). */
    for (size_t i = 0; i < n; i++)
    {
        int c = file_readbyte(fd);
        if (c == EOF) return bytes;

        *buf = (ubyte)c;
        buf++;
        bytes++;
    }

    return bytes;
}
