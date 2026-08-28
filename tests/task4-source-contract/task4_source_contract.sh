#!/bin/sh
set -eu

board_h="src/boards/k380/board.h"
board_mk="src/boards/k380/board.mk"
board_cmake="src/boards/k380/board.cmake"
boards_c="src/boards/boards.c"
boards_h="src/boards/boards.h"
indicator_h="src/boards/k380/status_indicator.h"
indicator_c="src/boards/k380/status_indicator.c"

fail() {
  echo "task4 source contract: $1" >&2
  exit 1
}

grep -q "#define K380_BOOTLOADER_STATUS_INDICATOR 1" "$board_h" ||
  fail "K380 status indicator feature flag is missing"
grep -q "#define LED_NEOPIXEL PINNUM(0, 13)" "$board_h" ||
  fail "K380 WS2812B data pin is not P0.13"
grep -q "#define NEOPIXELS_NUMBER 4" "$board_h" ||
  fail "K380 WS2812B pixel count is not 4"
grep -q "#define LEDS_NUMBER 0" "$board_h" ||
  fail "K380 must not enable generic board LEDs"

grep -q "src/boards/k380/status_indicator.c" "$board_mk" ||
  fail "Makefile board entry does not include K380 status indicator source"
grep -q "src/boards/k380/status_indicator.c" "$board_cmake" ||
  fail "CMake board entry does not include K380 status indicator source"

grep -q "board_led_state_override" "$boards_h" ||
  fail "board_led_state_override declaration is missing"
grep -q "board_led_tick_override" "$boards_h" ||
  fail "board_led_tick_override declaration is missing"
grep -q "__attribute__((weak)) board_led_state_override" "$boards_c" ||
  fail "weak board_led_state_override implementation is missing"
grep -q "__attribute__((weak)) board_led_tick_override" "$boards_c" ||
  fail "weak board_led_tick_override implementation is missing"
grep -q "board_led_state_override(state)" "$boards_c" ||
  fail "led_state does not call the state override hook"
grep -q "board_led_tick_override(millis)" "$boards_c" ||
  fail "led_tick does not call the tick override hook"

test -f "$indicator_h" || fail "status_indicator.h is missing"
test -f "$indicator_c" || fail "status_indicator.c is missing"
grep -q "struct k380_rgb" "$indicator_h" ||
  fail "k380_rgb type is missing"
grep -q "k380_status_indicator_init" "$indicator_h" ||
  fail "status indicator init API is missing"
grep -q "k380_status_indicator_led_state" "$indicator_h" ||
  fail "status indicator led_state API is missing"
grep -q "k380_status_indicator_tick" "$indicator_h" ||
  fail "status indicator tick API is missing"
grep -q "k380_status_indicator_write" "$indicator_h" ||
  fail "per-pixel write API is missing"
grep -q "K380_WS1_INDEX = 0" "$indicator_c" ||
  fail "WS1 index mapping is missing"
grep -q "K380_WS2_INDEX = 1" "$indicator_c" ||
  fail "WS2 index mapping is missing"
grep -q "K380_WS3_INDEX = 2" "$indicator_c" ||
  fail "WS3 index mapping is missing"
grep -q "K380_WS4_INDEX = 3" "$indicator_c" ||
  fail "WS4 index mapping is missing"
grep -q "k380_status_indicator_write(k380_status_pixels)" "$indicator_c" ||
  fail "indicator hook does not write the per-pixel buffer"
