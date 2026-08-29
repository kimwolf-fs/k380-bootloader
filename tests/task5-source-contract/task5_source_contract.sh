#!/bin/sh
set -eu

indicator_h="src/boards/k380/status_indicator.h"
indicator_c="src/boards/k380/status_indicator.c"
usb_c="src/usb/usb.c"
msc_c="src/usb/msc_uf2.c"
ghostfat_c="src/usb/uf2/ghostfat.c"
serial_c="lib/sdk11/components/libraries/bootloader_dfu/dfu_transport_serial.c"
main_c="src/main.c"

fail() {
  echo "task5 source contract: $1" >&2
  exit 1
}

grep -q "#define K380_SUCCESS_HOLD_MS 3600U" "$indicator_h" ||
  fail "B4 success hold must be 3600 ms"
grep -q "k380_status_indicator_show_success_blocking" "$indicator_h" ||
  fail "B4 blocking success helper is not declared"
grep -q "k380_status_indicator_delay_ms(K380_SUCCESS_HOLD_MS)" "$indicator_c" ||
  fail "B4 helper does not hold for the configured success duration"

grep -q "k380_status_priority_for_state" "$indicator_c" ||
  fail "status priority helper is missing"
grep -q "k380_accept_state" "$indicator_c" ||
  fail "low-priority status protection is missing"

grep -q "STATE_K380_CDC_ONLY : STATE_USB_MOUNTED" "$usb_c" ||
  fail "USB mount does not select CDC-only state"

grep -q "led_state(STATE_WRITING_STARTED)" "$ghostfat_c" ||
  fail "UF2 path does not enter B3 before flash writes"
grep -q "k380_status_indicator_show_success_blocking" "$msc_c" ||
  fail "UF2 success path does not hold B4"
grep -q "k380_status_indicator_show_success_blocking" "$serial_c" ||
  fail "CDC DFU success path does not hold B4"
grep -q "k380_status_indicator_show_success_blocking" "$main_c" ||
  fail "SoftDevice continuation success path does not hold B4"
