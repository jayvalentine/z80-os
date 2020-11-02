#include <stdio.h>
#include <string.h>

#include "disk_drivers.h"

typedef unsigned long ulong;
typedef unsigned int uint;
typedef unsigned char ubyte;

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

/* Initialises filesystem handlers from disk. */
void get_disk_info()
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
}

void dir()
{
    char filename[9];
    char extension[4];
    char volume_label[12];

    ulong sector = disk_info.root_region;

    ubyte done = 0;
    ubyte volume = 1;

    while (!done)
    {
        read_sector(temp_sector, sector);

        for (uint f = 0; f < 512; f += 32)
        {
            /* If we see a 0, we've hit the end of the root directory. */
            if (temp_sector[f] == 0)
            {
                done = 1;
                break;
            }

            ubyte attr = temp_sector[f+11];

            /* Do we expect a volume label? If so, print it. */
            if (volume && (attr & 0b00001000))
            {
                memcpy(volume_label, &temp_sector[f], 11);
                volume_label[11] = '\0';

                printf("VOLUME LABEL: %s\n\r\n\r", volume_label);

                /* We expect at most one volume label. Ignore any others. */
                volume = 0;
            }

            /* Otherwise, this is potentially a file, depending on the attribute bytes. */
            /* Skip if not actually a file. */
            if (attr & 0b00011010) continue;

            /* This is a file, so construct the filename and print. */
            memcpy(filename, &temp_sector[f], 8);
            memcpy(extension, &temp_sector[f+8], 3);
            filename[8] = '\0';
            extension[3] = '\0';

            printf("%s.%s\n\r", filename, extension);
        }

        /* Next sector. */
        sector++;
    }
}

char input[256];

size_t argc;
char * argv[256];
char * cmd;

void main(void)
{
    puts("Z80-OS\n\rCopyright (C) 2020 Jay Valentine.\n\r");
    
    puts("Initialising disk... ");
    init_disk();
    get_disk_info();
    puts("Done.\n\r");

    puts("\n\r");

    /* Command prompt. */
    while (1)
    {
        memset(input, 0, 256);

        puts("Z80-OS> ");
        gets(input);

        argc = 0;

        /* We expect at least one token. */
        cmd = strtok(input, " ");

        if (cmd == NULL) continue;

        /* Process the rest of the tokens, if any. */
        while (1)
        {
            char * p = strtok(NULL, " ");
            if (p == NULL) break;

            argv[argc] = p;
            argc++;
        }

        if (strcmp(cmd, "dir") == 0)
        {
            dir();
        }
        else if (strcmp(cmd, "quit") == 0)
        {
            break;
        }
        else
        {
            printf("Unknown command: %s\n\r", cmd);
        }
    }

    return;
}
