#include <syscall.h>
#include <string.h>

const char other_proc[16] = {
    0x0a, 0x80,
    0x21, 0x14, 0xc0,   /* ld HL, 0xc014 */
    0x3e, 99,           /* ld A, 99 */
    0x06, 5,            /* ld B, 5 */
    0x77,               /* ld (HL), A */
    0x23,               /* inc HL */
    0x3c,               /* inc A */
    0x10, 0xfb,         /* djnz -5 */
    0x18, 0xfe          /* infinite loop */
};

volatile char ret;

void loop(void);

void main(void)
{
    ret = 255;

    int fd = syscall_fopen("test.exe", FMODE_WRITE);

    if (fd < 0)
    {
        ret = 3;
        return;
    }

    syscall_fwrite(other_proc, 16, fd);
    
    syscall_fclose(fd);

    int pd = syscall_pload("test.exe");
    if (pd != 1)
    {
        ret = 4;
        return; 
    }

    int pd2 = syscall_pload("test.exe");
    if (pd2 != 2)
    {
        ret = 6;
        return;
    }
    
    int e = syscall_pspawn(pd, NULL, 0);
    if (e < 0)
    {
        ret = 5;
        return;
    }

    int e2 = syscall_pspawn(pd2, NULL, 0);
    if (e2 < 0)
    {
        ret = 7;
        return;
    }

    char * addr = (char*)0xd123;
    char v = 23;

    for (int i = 0; i < 5; i++)
    {
        *addr = v;
        v += 11;
        addr++;
    }

    ret = 0;
    loop();
}
