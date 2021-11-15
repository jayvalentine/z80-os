#include <stdint.h>

#define LED_INT 1
#define LED_SYSCALL 2

/* All LEDs initially off. */
uint8_t current_status = 0b11111111;

void status_set(uint8_t val) __z88dk_fastcall;

void status_led_off(uint8_t led)
{
    /* Register 1 bit means LED off. */
    uint8_t mask = 1 << led;
    current_status |= mask;
    status_set(current_status);
}

void status_led_on(uint8_t led)
{
    /* Register 0 bit means LED on. */
    uint8_t mask = ~(1 << led);
    current_status &= mask;
    status_set(current_status);
}

void status_set_int(void)
{
    status_led_on(LED_INT);
}

void status_clr_int(void)
{
    status_led_off(LED_INT);
}

void status_set_syscall(void)
{
    status_led_on(LED_SYSCALL);
}

void status_clr_syscall(void)
{
    status_led_off(LED_SYSCALL);
}
