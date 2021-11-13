#include <stdint.h>

uint8_t serial_current_mode;

void serial_mode(uint8_t mode)
{
    serial_current_mode = mode;
}
