#include <stdint.h>

#include <include/interrupt.h>

uint8_t serial_current_mode;

#define BUFFER_SIZE 256

typedef struct _CircularBuffer_T
{
    char * head;
    char * tail;
    char buf[BUFFER_SIZE];
} CircularBuffer_T;

CircularBuffer_T tx_buf;

void serial_init(void)
{
    tx_buf.head = tx_buf.buf;
    tx_buf.tail = tx_buf.buf;
}

void serial_mode(uint8_t mode)
{
    serial_current_mode = mode;
}
