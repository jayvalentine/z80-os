#include <stdint.h>
#include <stddef.h>

uint8_t ram_bank;

void ram_copy(char * dst, uint8_t bank, char * src, size_t n)
{

}

void ram_bank_set(uint8_t bank)
{
    ram_bank = bank;
}

uint8_t ram_bank_current(void)
{
    return ram_bank;
}

uint16_t ram_test(void)
{
    return 0;
}
