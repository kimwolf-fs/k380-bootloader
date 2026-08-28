#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "neopixel_encoding.h"

int main(void) {
  const uint8_t rgb_triplet[3] = {0x12, 0x34, 0x56};
  uint8_t wire_grb[3] = {0};

  neopixel_rgb_triplet_to_wire_grb(rgb_triplet, wire_grb);
  assert(wire_grb[0] == 0x34);
  assert(wire_grb[1] == 0x12);
  assert(wire_grb[2] == 0x56);

  const uint32_t legacy_rgb_word = 0x00123456;
  const uint8_t *legacy_bytes = (const uint8_t *)&legacy_rgb_word;
  neopixel_legacy_word_bytes_to_wire_grb(legacy_bytes, wire_grb);
  assert(wire_grb[0] == 0x34);
  assert(wire_grb[1] == 0x12);
  assert(wire_grb[2] == 0x56);

  uint16_t pattern[6] = {1, 2, 3, 4, 5, 6};
  uint16_t pos = 2;
  neopixel_append_reset_padding(pattern, &pos);
  assert(pos == 4);
  assert(pattern[0] == 1);
  assert(pattern[1] == 2);
  assert(pattern[2] == 0x8000);
  assert(pattern[3] == 0x8000);
  assert(pattern[4] == 5);
  assert(pattern[5] == 6);

  return 0;
}
