#include <stdint.h>

#include "status_indicator.h"

void neopixel_write_pixels(const uint8_t *pixels, uint16_t pixel_count) {
  (void)pixels;
  (void)pixel_count;
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
  if (!k380_status_indicator_led_state(0)) {
    return 1;
  }
  k380_status_indicator_tick(0);

  return 0;
}
