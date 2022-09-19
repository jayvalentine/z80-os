#define ZEBRA_LINES 28
#define ZEBRA_COLUMNS 32

#include <stdio.h>
#include <stdint.h>

const uint32_t zebra_pattern[ZEBRA_LINES] =
{
    0,
    0b00000000000110000000000000000000,
    0b00000000000111000000000000000000,
    0b00000001101111110000000000000000,
    0b00000000010011101000000000000000,
    0b00000011100010011100000000000000,
    0b00000100100110001010000000000000,
    0b00000001000000010111000000000000,
    0b00000010001000010010110000000000,
    0b00000010011000100011101100000000,
    0b00000100000001100110111011000000,
    0b00001000000001000100001010110000,
    0b00010001100011000100001110101100,
    0b00011000111110000100001011101011,
    0b00011100011001100100011001111010,
    0b00011110110000100100010001001110,
    0b00001111100000010100010001001011,
    0b00000111000000001100110011001001,
    0b00000000000000001001100010001001,
    0b00000000000000000101000010011001,
    0b00000000000000000101000110010001,
    0b00000000000000000011001000010001,
    0b00000000000000000010011000110001,
    0b00000000000000000001010001100011,
    0b00000000000000000001010010001010,
    0b00000000000000000001010010001000,
    0b00000000000000000000100010001000,
    0b00000000000000000000100010010000,
};

#define UPPER_MASK 0x80000000ul

void zebra_display(void)
{
    char zebra_image[(ZEBRA_LINES * ((ZEBRA_COLUMNS * 2) + 2)) + 1];
    char * zebra_image_ptr = &zebra_image[0];

    for (uint8_t i = 0; i < ZEBRA_LINES; i++)
    {
        uint32_t line = zebra_pattern[i];
        for (uint8_t j = 0; j < ZEBRA_COLUMNS; j++)
        {
            if (line == 0)
            {
                break;
            }
            
            if (line & UPPER_MASK)
            {
                *zebra_image_ptr = '#';
                zebra_image_ptr++;
                *zebra_image_ptr = '#';
                zebra_image_ptr++;
            }
            else
            {
                *zebra_image_ptr = ' ';
                zebra_image_ptr++;
                *zebra_image_ptr = ' ';
                zebra_image_ptr++;
            }

            line <<= 1;
        }

        *zebra_image_ptr = '\r';
        zebra_image_ptr++;
        *zebra_image_ptr = '\n';
        zebra_image_ptr++;
    }

    *zebra_image_ptr = '\0';

    puts(zebra_image);

    puts("\r\nZeus the Zebra by Katie Buttriss.\r\n\r\n");
}
