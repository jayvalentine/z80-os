#include <stddef.h>

/* In-place "upper-cases" the given string. */
void utils_toupper(char * s)
{
    while (*s != '\0')
    {
        if (*s >= 'a' && *s <= 'z')
        {
            *s -= ('a' - 'A');
        }
        s++;
    }
}

uint8_t utils_chartonib(char c)
{
    if (c <= '9' && c >= '0') return (uint8_t)c - '0';
    if (c <= 'f' && c >= 'a') return (uint8_t)c - 'a' + 10;
    
    /* Invalid, return some obvious error value. */
    return 0xff;
}

uint16_t utils_strtou(const char * s)
{
    uint16_t val = 0;

    /* At most 4 characters for a 16-bit int. */
    for (char * c = s; c < (s+4); c++)
    {
        if (*c == '\0') return val;

        val = val << 4;
        val |= utils_chartonib(*c);
    }

    return val;
}
