#ifndef TASK4_STATUS_INDICATOR_COMPILE_BOARDS_H
#define TASK4_STATUS_INDICATOR_COMPILE_BOARDS_H

#include <stdbool.h>
#include <stdint.h>

#define K380_BOOTLOADER_STATUS_INDICATOR 1
#define K380_STATUS_PIXEL_COUNT 4

void neopixel_write_pixels(const uint8_t *pixels, uint16_t pixel_count);

#endif
