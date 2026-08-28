#ifndef NEOPIXEL_ENCODING_H
#define NEOPIXEL_ENCODING_H

#include <stdint.h>

static inline void neopixel_rgb_triplet_to_wire_grb(const uint8_t rgb[3], uint8_t grb[3]) {
  grb[0] = rgb[1];
  grb[1] = rgb[0];
  grb[2] = rgb[2];
}

static inline void neopixel_legacy_word_bytes_to_wire_grb(const uint8_t bytes[3], uint8_t grb[3]) {
  grb[0] = bytes[1];
  grb[1] = bytes[2];
  grb[2] = bytes[0];
}

static inline void neopixel_append_reset_padding(uint16_t *pattern, uint16_t *pos) {
  pattern[(*pos)++] = 0x8000;
  pattern[(*pos)++] = 0x8000;
}

#endif
