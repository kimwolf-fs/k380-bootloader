#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "status_indicator.h"

static uint8_t last_pixels[K380_STATUS_PIXEL_COUNT * 3];
static uint16_t last_pixel_count;

void neopixel_write_pixels(const uint8_t *pixels, uint16_t pixel_count) {
  last_pixel_count = pixel_count;
  memcpy(last_pixels, pixels, sizeof(last_pixels));
}

int main(void) {
  const struct k380_rgb pixels[K380_STATUS_PIXEL_COUNT] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9},
    {10, 11, 12},
  };

  k380_status_indicator_init();
  k380_status_indicator_write(pixels);
  assert(last_pixel_count == K380_STATUS_PIXEL_COUNT);
  assert(memcmp(last_pixels, pixels, sizeof(pixels)) == 0);

  assert(k380_status_indicator_led_state(0));
  assert(last_pixel_count == K380_STATUS_PIXEL_COUNT);
  assert(memcmp(last_pixels, pixels, sizeof(pixels)) == 0);

  k380_status_indicator_tick(0);

  return 0;
}
