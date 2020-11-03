#include <string.h>
#include <stdio.h>

#include "file.h"
#include "disk_drivers.h"

/* Information about the disk. Needed for filesystem interaction. */
struct DiskInfo_T
{
    /* Region start sectors. */
    ulong fat_region;
    ulong root_region;
    ulong data_region;

    /* Other info. */
    ubyte sectors_per_cluster;
    ulong num_sectors;
} disk_info;

/* Temporary storage for sector read/written from/to disk. */
char temp_sector[512];

/* Helper functions. */

/* Get an unsigned integer (in Z80 little-endian)
 * from a buffer at the given position.
 */
uint get_uint(char * buf, uint i)
{
    uint hi = buf[i+1];
    uint lo = buf[i];
    return (hi << 8) | lo;
}

/* Get an unsigned long (in little-endian)
 * from a buffer at a given location.
 */
ulong get_ulong(char* buf, uint i)
{
    ulong hi_hi = buf[i+3];
    ulong hi_lo = buf[i+2];
    ulong lo_hi = buf[i+1];
    ulong lo_lo = buf[i];

    return (hi_hi << 24) | (hi_lo << 16) | (lo_hi << 8) | lo_lo;
}

FileError_T filesystem_init()
{
    read_sector(temp_sector, 0);

    /* General disk info. */
    disk_info.sectors_per_cluster = temp_sector[0x0d];

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

    return NOERROR;
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

FileError_T file_open(const char * filename, File_T * fd)
{
    ulong sector = disk_info.root_region;
    bool done = FALSE;

    /* Buffer into which we'll copy the file entry. */
    ubyte file_entry[32];

    /* Buffer for filename. */
    ubyte buf[13];

    /* Try to find the file in the root directory. */
    while (!done)
    {
        /* Read the sector. */
        read_sector(temp_sector, sector);

        /* Iterate over the files, looking for the one we want. */
        for (uint f = 0; f < 512; f += 32)
        {
            /* If first byte is 0, we've reached the end of the root directory,
             * but not found the file. */
            if (temp_sector[f] == 0) return FILENOTFOUND;

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
                memcpy(file_entry, &temp_sector[f], 32);
                done = TRUE;
                break;
            }
        }

        sector++;
    }

    return NOERROR;
}
