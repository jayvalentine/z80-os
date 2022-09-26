#include <string.h>

#include <include/file.h>

#include <syscall.h>
#include <test.h>
#include <disk.h>

#define DRIVE_SECTOR_COUNT 1024
#define DRIVE_SECTOR_SIZE 512
#define DRIVE_SECTORS_PER_CLUSTER 8

extern FileDescriptor_T fdtable[FILE_LIMIT];
extern DiskInfo_T disk_info;

/* Given an open file descriptor with <512 bytes, ensure that file_read
 * reads the correct number of bytes.
 */
int test_read_less_than_512_bytes()
{
    mock_drive_init();

    disk_write("HelloAndSomeGarbage", disk_info.data_region);

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
    ASSERT(bytes == 5);

    /* We should only have overwritten the first 5 bytes of buf. */
    ASSERT(buf[0] == 'H');
    ASSERT(buf[1] == 'e');
    ASSERT(buf[2] == 'l');
    ASSERT(buf[3] == 'l');
    ASSERT(buf[4] == 'o');

    ASSERT(buf[5] == '!');
    ASSERT(buf[15] == '!');
    ASSERT(buf[31] == '!');

    return 0;
}

/* Given an open file descriptor with <512 bytes, ensure that file_read
 * reads the correct number of bytes when we try to read a full sector.
 */
int test_read_512_bytes()
{
    mock_drive_init();

    disk_write("HelloAndSomeGarbage", disk_info.data_region);

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
    ASSERT(bytes == 5);

    /* We should only have overwritten the first 5 bytes of buf. */
    ASSERT(buf[0] == 'H');
    ASSERT(buf[1] == 'e');
    ASSERT(buf[2] == 'l');
    ASSERT(buf[3] == 'l');
    ASSERT(buf[4] == 'o');

    ASSERT(buf[5] == '!');
    ASSERT(buf[15] == '!');
    ASSERT(buf[31] == '!');
    ASSERT(buf[511] == '!');

    return 0;
}

/* Given a filesystem with a file, ensure that calling file_delete
 * on that file causes it to be deleted.
 */
int test_file_info_for_nonexistent_file()
{
    mock_drive_init();

    /* Get file info for non-existent file. */
    FINFO finfo;
    int error = file_info("test.txt", &finfo);

    /* Check that error is E_FILENOTFOUND. */
    ASSERT(error == E_FILENOTFOUND);

    return 0;
}

/* Given a filesystem, ensure that creating a file
 * creates a file of 0 bytes with correct attributes.
 */
int test_file_info_for_created_file()
{
    mock_drive_init();

    /* Create a file. */
    file_new("test.txt");

    /* Get file info for new file. */
    FINFO finfo;
    int error = file_info("test.txt", &finfo);

    /* Check that there is no error. */
    ASSERT(error == 0);

    /* Check info. */
    ASSERT(finfo.size == 0);
    ASSERT(finfo.created_year == 1980);
    ASSERT(finfo.created_month == 1);
    ASSERT(finfo.created_day == 1);

    /* Check attrs. */
    ASSERT(finfo.attr == 0);

    return 0;
}

/* Given a filesystem, ensure that creating a file
 * twice returns an error the second time.
 */
int test_file_create_twice()
{
    mock_drive_init();

    /* Create a file. */
    file_new("test.txt");

    /* Get file info for new file. */
    FINFO finfo;
    int error = file_info("test.txt", &finfo);

    /* Check that there is no error. */
    ASSERT(error == 0);

    /* Check info. */
    ASSERT(finfo.size == 0);
    ASSERT(finfo.created_year == 1980);
    ASSERT(finfo.created_month == 1);
    ASSERT(finfo.created_day == 1);

    /* Check attrs. */
    ASSERT(finfo.attr == 0);

    /* Try to create the file again. */
    int error2 = file_new("test.txt");
    ASSERT(error2 == E_FILEEXIST);

    return 0;
}

uint16_t fat_next_cluster(uint16_t cluster);

/* Given a filesystem, ensure that a file can be deleted.
 * We check that the allocated start cluster has been freed too.
 */
int test_file_delete()
{
    mock_drive_init();

    /* Create a file. */
    file_new("test.txt");

    /* Open new file. */
    int fd = file_open("test.txt", FMODE_READ);

    /* Check that we have a valid file descriptor. */
    ASSERT(fd == 0);

    /* Get info about start cluster. */
    FileDescriptor_T * file = &fdtable[fd];

    uint16_t cluster = file->start_cluster;

    /* Get value of this entry in the fat cluster. */
    /* Check that cluster is taken. */
    ASSERT(fat_next_cluster(cluster) == 0xffff);

    /* Try to delete the file. */
    int error2 = file_delete("test.txt");

    ASSERT(error2 == 0);

    /* Attempting to open the file should fail. */
    int fd2 = file_open("test.txt", FMODE_READ);

    ASSERT(fd2 == E_FILENOTFOUND);

    /* Check that cluster is free. */
    ASSERT(fat_next_cluster(cluster) == 0x0000);

    return 0;
}

/* Given a filesystem, ensure that a large file (distributed over 2 clusters)
 * can be deleted.
 */
int test_file_delete_large()
{
    mock_drive_init();

    /* Create a file. */
    int fd = file_open("test.txt", FMODE_WRITE);

    ASSERT(fd == 0);

    /* Write to file. */
    char buf[(DRIVE_SECTORS_PER_CLUSTER * DRIVE_SECTOR_SIZE) + 22];
    memset(buf, 'a', (DRIVE_SECTORS_PER_CLUSTER * DRIVE_SECTOR_SIZE) + 22);
    size_t bytes = file_write(buf, (DRIVE_SECTORS_PER_CLUSTER * DRIVE_SECTOR_SIZE) + 22, fd);

    ASSERT(bytes == (DRIVE_SECTORS_PER_CLUSTER * DRIVE_SECTOR_SIZE) + 22);

    /* We should have allocated two clusters. */
    /* Get info about start cluster. */
    FileDescriptor_T * file = &fdtable[fd];

    uint16_t cluster = file->start_cluster;
    uint16_t cluster2 = fat_next_cluster(file->start_cluster);
    uint16_t cluster3 = fat_next_cluster(cluster2);

    /* Both should be real, allocated clusters. */
    ASSERT(cluster != 0xffff);
    ASSERT(cluster != 0x0000);

    ASSERT(cluster2 != 0xffff);
    ASSERT(cluster2 != 0x0000);

    /* No more than 2 clusters allocated. */
    ASSERT(cluster3 == 0xffff);

    /* Now delete the file. We should see all clusters freed. */
    file_delete("test.txt");

    ASSERT(fat_next_cluster(cluster) == 0x0000);
    ASSERT(fat_next_cluster(cluster2) == 0x0000);

    return 0;
}

/* Check that attempting to delete a non-existent file results in an error.
 */
int test_file_delete_nonexistent()
{
    mock_drive_init();

    /* Try to delete a non-existent file. */
    int error2 = file_delete("test.txt");

    ASSERT(error2 == E_FILENOTFOUND);

    return 0;
}

/* Check that file_entry returns the correct filename
 * with a single file on disk.
 */
int test_file_entry()
{
    mock_drive_init();

    /* Open new file. */
    int fd = file_open("test.txt", FMODE_WRITE);

    ASSERT(fd == 0);

    /* Close new file. */
    file_close(fd);

    /* Check that file_entry returns the right filename. */
    char filename[20];
    int e = file_entry(filename, 0);

    ASSERT(e == 0);
    ASSERT_EQUAL_STRING("TEST.TXT", filename);

    return 0;
}

/* Check that file_entry returns the correct filename
 * with multiple files on disk.
 */
int test_file_entry_multiple()
{
    mock_drive_init();

    /* Open new file. */
    int fd = file_open("test.txt", FMODE_WRITE);

    ASSERT(fd == 0);

    /* Close new file. */
    file_close(fd);

    /* Open new file. */
    int fd2 = file_open("test2.log", FMODE_WRITE);

    ASSERT(fd2 == 0);

    /* Close new file. */
    file_close(fd2);

    /* Check that file_entry returns the right filename. */
    char filename[20];

    int e = file_entry(filename, 0);

    ASSERT(e == 0);
    ASSERT_EQUAL_STRING("TEST.TXT", filename);

    int e2 = file_entry(filename, 1);

    ASSERT(e2 == 0);
    ASSERT_EQUAL_STRING("TEST2.LOG", filename);

    return 0;
}

/* Check that file_entry returns an error with an
 * out-of-bounds number.
 */
int test_file_entry_nonexistent()
{
    mock_drive_init();

    /* Open new file. */
    int fd = file_open("test.txt", FMODE_WRITE);

    ASSERT(fd == 0);

    /* Close new file. */
    file_close(fd);

    /* Check that file_entry returns the right filename. */
    char filename[20];
    int e = file_entry(filename, 1);

    ASSERT(e == E_FILENOTFOUND);

    return 0;
}

/* Tests with multiple files on disk, checking that the results of file_entry
 * are correct for all of them.
 */
int test_file_entry_check_all()
{
    int fd;
    int e;
    char filename[20];

    mock_drive_init();

    fd = file_open("afile.txt", FMODE_WRITE);
    file_close(fd);

    fd = file_open("file2.log", FMODE_WRITE);
    file_close(fd);

    fd = file_open("nextfile.asm", FMODE_WRITE);
    file_close(fd);
    
    /* Check names of files returned by fentry syscall. */
    e = file_entry(filename, 0);
    ASSERT_EQUAL_INT(0, e);
    ASSERT_EQUAL_STRING("AFILE.TXT", filename);

    e = file_entry(filename, 1);
    ASSERT_EQUAL_INT(0, e);
    ASSERT_EQUAL_STRING("FILE2.LOG", filename);

    e = file_entry(filename, 2);
    ASSERT_EQUAL_INT(0, e);
    ASSERT_EQUAL_STRING("NEXTFILE.ASM", filename);
    
    /* Should return an error
     * because there are only three files on disk. */
    e = file_entry(filename, 3);
    ASSERT_EQUAL_INT(E_FILENOTFOUND, e);
    
    /* Test passed. */
    return 0;
}

/* Check that file_entries returns the correct number of files.
 */
int test_file_entries()
{
    mock_drive_init();

    ASSERT(0 == file_entries());

    /* Open new file. */
    int fd = file_open("test.txt", FMODE_WRITE);

    ASSERT(fd == 0);

    /* Close new file. */
    file_close(fd);

    ASSERT(1 == file_entries());

    /* Open new file. */
    int fd2 = file_open("test2.txt", FMODE_WRITE);

    ASSERT(fd2 == 0);

    /* Close new file. */
    file_close(fd2);

    /* Open new file. */
    int fd3 = file_open("test3.txt", FMODE_WRITE);

    ASSERT(fd3 == 0);

    /* Close new file. */
    file_close(fd3);

    ASSERT(3 == file_entries());

    /* Delete a file. */
    int e = file_delete("test2.txt");
    ASSERT(0 == e);

    ASSERT(2 == file_entries());

    return 0;
}

/* Stress test to ensure that:
 * 1. Deleting a file does not affect other files.
 * 2. Files can be allocated/deleted repeatedly without unexpected side effects.
 * 
 * Note: This tests only the logic of the filesystem, and does not guarantee
 * the performance of a disk under this kind of stress-testing.
 */
typedef struct _test_file
{
    const char * filename;
    char character;
    size_t size;
} test_file_t;

#define TEST_FILES 5

int test_file_stress_test()
{
    mock_drive_init();

    char fbuf[DRIVE_SECTOR_SIZE * DRIVE_SECTORS_PER_CLUSTER * 10];

    test_file_t files[TEST_FILES] =
    {
        {
            .filename = "a.txt",
            .character = 'a',
            .size = 24
        },
        {
            .filename = "b.txt",
            .character = 'b',
            .size = 1024
        },
        {
            .filename = "c.txt",
            .character = 'c',
            .size = DRIVE_SECTOR_SIZE * DRIVE_SECTORS_PER_CLUSTER
        },
        {
            .filename = "d.txt",
            .character = 'd',
            .size = DRIVE_SECTOR_SIZE * DRIVE_SECTORS_PER_CLUSTER * 2 + 123
        },
        {
            .filename = "e.txt",
            .character = 'e',
            .size = DRIVE_SECTOR_SIZE * DRIVE_SECTORS_PER_CLUSTER * 8 + 99
        }
    };

    for (int i = 0; i < 512; i++)
    {
        /* Create each file. */
        for (int f = 0; f < TEST_FILES; f++)
        {
            memset(fbuf, files[f].character, files[f].size);
            int fd = file_open(files[f].filename, FMODE_WRITE);
            ASSERT(fd == 0);
            size_t bytes = file_write(fbuf, files[f].size, fd);
            ASSERT(bytes == files[f].size);
            file_close(fd);
        }

        /* Delete each file. */
        for (int f = 0; f < TEST_FILES; f++)
        {
            int error = file_delete(files[f].filename);
            ASSERT(error == 0);

            /* Check each of the other files can still be read. */
            for (int f2 = f+1; f2 < TEST_FILES; f2++)
            {
                int fd = file_open(files[f2].filename, FMODE_READ);
                ASSERT(fd == 0);
                size_t bytes = file_read(fbuf, files[f2].size, fd);

                for (int c = 0; c < files[f2].size; c++)
                {
                    ASSERT(fbuf[c] == files[f2].character);
                }

                ASSERT(bytes == files[f2].size);
                file_close(fd);
            }
        }
    }
}
