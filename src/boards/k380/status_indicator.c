#include "status_indicator.h"

#include <string.h>

#include "boards.h"

enum {
  K380_WS1_INDEX = 0,
  K380_WS2_INDEX = 1,
  K380_WS3_INDEX = 2,
  K380_WS4_INDEX = 3,
};

static struct k380_rgb k380_status_pixels[K380_STATUS_PIXEL_COUNT];

void k380_status_indicator_write(const struct k380_rgb pixels[K380_STATUS_PIXEL_COUNT]) {
  memcpy(k380_status_pixels, pixels, sizeof(k380_status_pixels));
  neopixel_write_pixels((const uint8_t *)k380_status_pixels, K380_STATUS_PIXEL_COUNT);
}

void k380_status_indicator_init(void) {
  memset(k380_status_pixels, 0, sizeof(k380_status_pixels));
  k380_status_indicator_write(k380_status_pixels);
}

bool k380_status_indicator_led_state(uint32_t state) {
  (void)state;
  k380_status_indicator_write(k380_status_pixels);
  return true;
}

void k380_status_indicator_tick(uint32_t millis) {
  (void)millis;
}

void board_init2(void) {
  k380_status_indicator_init();
}

bool board_led_state_override(uint32_t state) {
  return k380_status_indicator_led_state(state);
}

bool board_led_tick_override(uint32_t millis) {
  k380_status_indicator_tick(millis);
  return true;
}
