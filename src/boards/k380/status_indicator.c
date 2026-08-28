#include "status_indicator.h"

#include <string.h>

#include "boards.h"

enum {
  K380_WS1_INDEX = 0,
  K380_WS2_INDEX = 1,
  K380_WS3_INDEX = 2,
  K380_WS4_INDEX = 3,
};

enum k380_status_mode {
  K380_STATUS_OFF = 0,
  K380_STATUS_WAITING,
  K380_STATUS_CDC_ONLY,
  K380_STATUS_WRITING,
  K380_STATUS_WRITE_SUCCESS,
  K380_STATUS_WRITE_FAILED,
  K380_STATUS_POWER_REJECTED,
};

enum {
  K380_BRIGHTNESS_10_PERCENT = 25,
  K380_BRIGHTNESS_20_PERCENT = 51,
  K380_SLOW_BLINK_ON_MS = 1000,
  K380_SLOW_BLINK_OFF_MS = 1000,
  K380_FAST_BLINK_ON_MS = 250,
  K380_FAST_BLINK_OFF_MS = 250,
  K380_WRITE_STEP_MS = 150,
  K380_DOUBLE_FLASH_ON_MS = 100,
  K380_DOUBLE_FLASH_OFF_MS = 100,
  K380_DOUBLE_FLASH_PAUSE_MS = 900,
};

static struct k380_rgb k380_status_pixels[K380_STATUS_PIXEL_COUNT];
static enum k380_status_mode k380_status_mode;
static uint32_t k380_current_millis;
static uint32_t k380_state_start_millis;

static const struct k380_rgb k380_off = {0, 0, 0};
static const struct k380_rgb k380_blue_10 = {0, 0, K380_BRIGHTNESS_10_PERCENT};
static const struct k380_rgb k380_purple_10 = {K380_BRIGHTNESS_10_PERCENT, 0, K380_BRIGHTNESS_10_PERCENT};
static const struct k380_rgb k380_green_10 = {0, K380_BRIGHTNESS_10_PERCENT, 0};
static const struct k380_rgb k380_red_20 = {K380_BRIGHTNESS_20_PERCENT, 0, 0};
static const struct k380_rgb k380_yellow_20 = {K380_BRIGHTNESS_20_PERCENT, K380_BRIGHTNESS_20_PERCENT, 0};

static bool k380_blink_is_on(uint32_t elapsed, uint32_t on_ms, uint32_t off_ms) {
  return (elapsed % (on_ms + off_ms)) < on_ms;
}

static bool k380_double_flash_is_on(uint32_t elapsed) {
  uint32_t const phase = elapsed % (K380_DOUBLE_FLASH_ON_MS * 2 +
                                   K380_DOUBLE_FLASH_OFF_MS +
                                   K380_DOUBLE_FLASH_PAUSE_MS);
  return (phase < K380_DOUBLE_FLASH_ON_MS) ||
         (phase >= (K380_DOUBLE_FLASH_ON_MS + K380_DOUBLE_FLASH_OFF_MS) &&
          phase < (K380_DOUBLE_FLASH_ON_MS * 2 + K380_DOUBLE_FLASH_OFF_MS));
}

static void k380_clear_pixels(struct k380_rgb pixels[K380_STATUS_PIXEL_COUNT]) {
  for (uint8_t i = 0; i < K380_STATUS_PIXEL_COUNT; i++) {
    pixels[i] = k380_off;
  }
}

static void k380_set_ws123(struct k380_rgb pixels[K380_STATUS_PIXEL_COUNT],
                           const struct k380_rgb color) {
  pixels[K380_WS1_INDEX] = color;
  pixels[K380_WS2_INDEX] = color;
  pixels[K380_WS3_INDEX] = color;
}

static void k380_render_status(void) {
  struct k380_rgb pixels[K380_STATUS_PIXEL_COUNT];
  uint32_t const elapsed = k380_current_millis - k380_state_start_millis;

  k380_clear_pixels(pixels);

  switch (k380_status_mode) {
    case K380_STATUS_WAITING:
      if (k380_blink_is_on(elapsed, K380_SLOW_BLINK_ON_MS, K380_SLOW_BLINK_OFF_MS)) {
        k380_set_ws123(pixels, k380_blue_10);
      }
      break;

    case K380_STATUS_CDC_ONLY:
      if (k380_blink_is_on(elapsed, K380_SLOW_BLINK_ON_MS, K380_SLOW_BLINK_OFF_MS)) {
        k380_set_ws123(pixels, k380_purple_10);
      }
      break;

    case K380_STATUS_WRITING: {
      static const uint8_t write_order[] = {
        K380_WS3_INDEX,
        K380_WS2_INDEX,
        K380_WS1_INDEX,
      };
      uint8_t const index = write_order[(elapsed / K380_WRITE_STEP_MS) % sizeof(write_order)];
      pixels[index] = k380_yellow_20;
      break;
    }

    case K380_STATUS_WRITE_SUCCESS:
      if (k380_double_flash_is_on(elapsed)) {
        k380_set_ws123(pixels, k380_green_10);
      }
      break;

    case K380_STATUS_WRITE_FAILED:
      if (k380_blink_is_on(elapsed, K380_FAST_BLINK_ON_MS, K380_FAST_BLINK_OFF_MS)) {
        k380_set_ws123(pixels, k380_red_20);
      }
      break;

    case K380_STATUS_POWER_REJECTED:
      if (k380_blink_is_on(elapsed, K380_FAST_BLINK_ON_MS, K380_FAST_BLINK_OFF_MS)) {
        pixels[K380_WS4_INDEX] = k380_red_20;
      }
      break;

    case K380_STATUS_OFF:
    default:
      break;
  }

  k380_status_indicator_write(pixels);
}

static void k380_set_status_mode(enum k380_status_mode mode) {
  if (k380_status_mode == K380_STATUS_WRITING &&
      (mode == K380_STATUS_WAITING || mode == K380_STATUS_CDC_ONLY || mode == K380_STATUS_OFF)) {
    k380_render_status();
    return;
  }

  if (k380_status_mode != mode) {
    k380_status_mode = mode;
    k380_state_start_millis = k380_current_millis;
  }

  k380_render_status();
}

void k380_status_indicator_write(const struct k380_rgb pixels[K380_STATUS_PIXEL_COUNT]) {
  memcpy(k380_status_pixels, pixels, sizeof(k380_status_pixels));
  neopixel_write_pixels((const uint8_t *)k380_status_pixels, K380_STATUS_PIXEL_COUNT);
}

void k380_status_indicator_init(void) {
  k380_status_mode = K380_STATUS_OFF;
  k380_current_millis = 0;
  k380_state_start_millis = 0;
  memset(k380_status_pixels, 0, sizeof(k380_status_pixels));
  k380_status_indicator_write(k380_status_pixels);
}

bool k380_status_indicator_led_state(uint32_t state) {
  switch (state) {
    case STATE_BOOTLOADER_STARTED:
    case STATE_USB_MOUNTED:
      k380_set_status_mode(K380_STATUS_WAITING);
      break;

    case STATE_K380_CDC_ONLY:
      k380_set_status_mode(K380_STATUS_CDC_ONLY);
      break;

    case STATE_WRITING_STARTED:
      k380_set_status_mode(K380_STATUS_WRITING);
      break;

    case STATE_WRITING_FINISHED:
      k380_set_status_mode(K380_STATUS_WRITE_SUCCESS);
      break;

    case STATE_K380_WRITE_FAILED:
      k380_set_status_mode(K380_STATUS_WRITE_FAILED);
      break;

    case STATE_K380_POWER_REJECTED:
      k380_set_status_mode(K380_STATUS_POWER_REJECTED);
      break;

    case STATE_USB_UNMOUNTED:
      k380_set_status_mode(K380_STATUS_OFF);
      break;

    default:
      k380_render_status();
      break;
  }

  return true;
}

void k380_status_indicator_tick(uint32_t millis) {
  k380_current_millis = millis;
  k380_render_status();
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
