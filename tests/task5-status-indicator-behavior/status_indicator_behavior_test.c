#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "boards.h"
#include "status_indicator.h"

static struct k380_rgb last_pixels[K380_STATUS_PIXEL_COUNT];
static uint16_t last_pixel_count;
static uint32_t total_delay_ms;

void neopixel_write_pixels(const uint8_t *pixels, uint16_t pixel_count) {
  last_pixel_count = pixel_count;
  memcpy(last_pixels, pixels, sizeof(last_pixels));
}

static void expect_pixels(const struct k380_rgb expected[K380_STATUS_PIXEL_COUNT]) {
  assert(last_pixel_count == K380_STATUS_PIXEL_COUNT);
  assert(memcmp(last_pixels, expected, sizeof(last_pixels)) == 0);
}

static void reset_indicator(void) {
  memset(last_pixels, 0xff, sizeof(last_pixels));
  last_pixel_count = 0;
  total_delay_ms = 0;
  k380_status_indicator_init();
  k380_status_indicator_tick(0);
}

void k380_status_indicator_delay_ms(uint32_t millis) {
  total_delay_ms += millis;
}

static void test_b1_blue_slow_blink(void) {
  const struct k380_rgb on[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 25}, {0, 0, 25}, {0, 0, 25}, {0, 0, 0}
  };
  const struct k380_rgb off[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
  };

  reset_indicator();
  assert(k380_status_indicator_led_state(STATE_USB_MOUNTED));
  expect_pixels(on);
  k380_status_indicator_tick(999);
  expect_pixels(on);
  k380_status_indicator_tick(1000);
  expect_pixels(off);
  k380_status_indicator_tick(2000);
  expect_pixels(on);
}

static void test_b2_purple_slow_blink(void) {
  const struct k380_rgb on[K380_STATUS_PIXEL_COUNT] = {
    {25, 0, 25}, {25, 0, 25}, {25, 0, 25}, {0, 0, 0}
  };
  const struct k380_rgb off[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
  };

  reset_indicator();
  assert(k380_status_indicator_led_state(STATE_K380_CDC_ONLY));
  expect_pixels(on);
  k380_status_indicator_tick(1000);
  expect_pixels(off);
}

static void test_b3_yellow_sequence_and_waiting_priority(void) {
  const struct k380_rgb ws3_on[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 0}, {0, 0, 0}, {51, 51, 0}, {0, 0, 0}
  };
  const struct k380_rgb ws2_on[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 0}, {51, 51, 0}, {0, 0, 0}, {0, 0, 0}
  };
  const struct k380_rgb ws1_on[K380_STATUS_PIXEL_COUNT] = {
    {51, 51, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
  };

  reset_indicator();
  assert(k380_status_indicator_led_state(STATE_WRITING_STARTED));
  expect_pixels(ws3_on);
  k380_status_indicator_tick(149);
  expect_pixels(ws3_on);
  k380_status_indicator_tick(150);
  expect_pixels(ws2_on);
  k380_status_indicator_tick(300);
  expect_pixels(ws1_on);
  k380_status_indicator_tick(450);
  expect_pixels(ws3_on);

  assert(k380_status_indicator_led_state(STATE_USB_MOUNTED));
  assert(k380_status_indicator_led_state(STATE_K380_CDC_ONLY));
  k380_status_indicator_tick(600);
  expect_pixels(ws2_on);
}

static void test_b4_green_double_flash(void) {
  const struct k380_rgb on[K380_STATUS_PIXEL_COUNT] = {
    {0, 25, 0}, {0, 25, 0}, {0, 25, 0}, {0, 0, 0}
  };
  const struct k380_rgb off[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
  };

  reset_indicator();
  assert(k380_status_indicator_led_state(STATE_WRITING_STARTED));
  assert(k380_status_indicator_led_state(STATE_WRITING_FINISHED));
  expect_pixels(on);
  k380_status_indicator_tick(100);
  expect_pixels(off);
  k380_status_indicator_tick(200);
  expect_pixels(on);
  k380_status_indicator_tick(300);
  expect_pixels(off);
  k380_status_indicator_tick(1200);
  expect_pixels(on);
}

static void test_b5_red_fast_blink(void) {
  const struct k380_rgb on[K380_STATUS_PIXEL_COUNT] = {
    {51, 0, 0}, {51, 0, 0}, {51, 0, 0}, {0, 0, 0}
  };
  const struct k380_rgb off[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
  };

  reset_indicator();
  assert(k380_status_indicator_led_state(STATE_K380_WRITE_FAILED));
  expect_pixels(on);
  k380_status_indicator_tick(250);
  expect_pixels(off);
  k380_status_indicator_tick(500);
  expect_pixels(on);
}

static void test_b6_ws4_red_fast_blink(void) {
  const struct k380_rgb on[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {51, 0, 0}
  };
  const struct k380_rgb off[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}
  };

  reset_indicator();
  assert(k380_status_indicator_led_state(STATE_K380_POWER_REJECTED));
  expect_pixels(on);
  k380_status_indicator_tick(250);
  expect_pixels(off);
  k380_status_indicator_tick(500);
  expect_pixels(on);
}

static void test_lower_priority_states_do_not_override_terminal_hints(void) {
  const struct k380_rgb b6_on[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {51, 0, 0}
  };
  const struct k380_rgb b5_on[K380_STATUS_PIXEL_COUNT] = {
    {51, 0, 0}, {51, 0, 0}, {51, 0, 0}, {0, 0, 0}
  };
  const struct k380_rgb b4_on[K380_STATUS_PIXEL_COUNT] = {
    {0, 25, 0}, {0, 25, 0}, {0, 25, 0}, {0, 0, 0}
  };

  reset_indicator();
  assert(k380_status_indicator_led_state(STATE_K380_POWER_REJECTED));
  assert(k380_status_indicator_led_state(STATE_USB_MOUNTED));
  assert(k380_status_indicator_led_state(STATE_USB_UNMOUNTED));
  expect_pixels(b6_on);
  assert(k380_status_indicator_led_state(STATE_WRITING_STARTED));
  k380_status_indicator_tick(150);
  const struct k380_rgb b3_ws2_on[K380_STATUS_PIXEL_COUNT] = {
    {0, 0, 0}, {51, 51, 0}, {0, 0, 0}, {0, 0, 0}
  };
  expect_pixels(b3_ws2_on);

  reset_indicator();
  assert(k380_status_indicator_led_state(STATE_K380_WRITE_FAILED));
  assert(k380_status_indicator_led_state(STATE_K380_CDC_ONLY));
  expect_pixels(b5_on);

  reset_indicator();
  assert(k380_status_indicator_led_state(STATE_WRITING_STARTED));
  assert(k380_status_indicator_led_state(STATE_WRITING_FINISHED));
  assert(k380_status_indicator_led_state(STATE_USB_MOUNTED));
  expect_pixels(b4_on);
}

static void test_success_helper_holds_b4_for_three_double_flash_cycles(void) {
  const struct k380_rgb b4_on[K380_STATUS_PIXEL_COUNT] = {
    {0, 25, 0}, {0, 25, 0}, {0, 25, 0}, {0, 0, 0}
  };

  reset_indicator();
  assert(k380_status_indicator_led_state(STATE_WRITING_STARTED));
  assert(k380_status_indicator_led_state(STATE_WRITING_FINISHED));
  k380_status_indicator_show_success_blocking();
  assert(total_delay_ms == 3600);
  expect_pixels(b4_on);
}

int main(void) {
  test_b1_blue_slow_blink();
  test_b2_purple_slow_blink();
  test_b3_yellow_sequence_and_waiting_priority();
  test_b4_green_double_flash();
  test_b5_red_fast_blink();
  test_b6_ws4_red_fast_blink();
  test_lower_priority_states_do_not_override_terminal_hints();
  test_success_helper_holds_b4_for_three_double_flash_cycles();
  return 0;
}
