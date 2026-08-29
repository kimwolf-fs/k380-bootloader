#ifndef K380_STATUS_INDICATOR_H
#define K380_STATUS_INDICATOR_H

#include <stdbool.h>
#include <stdint.h>

#define K380_STATUS_PIXEL_COUNT 4
#define K380_SUCCESS_HOLD_MS 3600U

struct k380_rgb {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

void k380_status_indicator_init(void);
bool k380_status_indicator_led_state(uint32_t state);
void k380_status_indicator_tick(uint32_t millis);
void k380_status_indicator_write(const struct k380_rgb pixels[K380_STATUS_PIXEL_COUNT]);
void k380_status_indicator_delay_ms(uint32_t millis);
void k380_status_indicator_show_success_blocking(void);

#endif
