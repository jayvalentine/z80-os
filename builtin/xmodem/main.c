#include <syscall.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#define ACK 0x06
#define NAK 0x15
#define EOT 0x04

char data[128];

int user_main(char ** argv, size_t argc)
{
    if (argc != 1)
    {
        puts("Usage: xmodem <file>\r\n");
        return 1;
    }

    int fd = syscall_fopen(argv[0], FMODE_WRITE);

    /* Wait for user confirmation. */
    puts("Press <r> to begin receiving...\r\n");
    while (1)
    {
        char c = getchar();
        if (c == 'r') break;
    }

    syscall_smode(SMODE_BINARY);

    /* Send initial NAK */
    putchar(NAK);
    
    while (1)
    {
        /* Get header. */
        uint8_t header = getchar();

        /* End of transmission? */
        if (header == EOT) break;

        /* Get packet number. */
        getchar();
        getchar();

        /* Get data - 128 bytes. */
        for (uint8_t i = 0; i < 128; i++)
        {
            char c = getchar();
            data[i] = c;
        }

        /* Get checksum. */
        getchar();

        /* Write data to file. */
        syscall_fwrite(data, 128, fd);

        /* Now send ACK. */
        putchar(ACK);
    }

    /* Send final ACK? */
    putchar(ACK);

    syscall_fclose(fd);

    syscall_smode(0x00);

    return 0;
}
