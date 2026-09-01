#!/bin/sh
set -eu

power_gate_h="src/power_gate.h"
power_gate_c="src/power_gate.c"
board_h="src/boards/k380/board.h"
indicator_c="src/boards/k380/status_indicator.c"
main_c="src/main.c"

fail() {
  echo "task6 source contract: $1" >&2
  exit 1
}

grep -q "#define BOOTLOADER_POWER_GATE_USB_PRESENT_MV 4500U" "$power_gate_h" ||
  fail "VDDH USB power threshold is not 4500 mV"
grep -q "SAADC_CH_PSELP_PSELP_VDDHDIV5" "$power_gate_c" ||
  fail "power gate does not sample VDDHDIV5"
grep -q "bootloader_power_gate_policy_flash_allowed" "$power_gate_c" ||
  fail "flash gate does not use the VDDH policy"
! grep -q "POFTHRVDDH\|POFWARN" "$power_gate_c" ||
  fail "flash gate still uses VDDH POFWARN"

grep -q "#define LED_NEOPIXEL PINNUM(0, 13)" "$board_h" ||
  fail "K380 WS2812B data pin is not P0.13"
grep -q "#define NEOPIXELS_NUMBER 4" "$board_h" ||
  fail "K380 WS2812B pixel count is not 4"

grep -q "K380_WS1_INDEX = 0" "$indicator_c" || fail "WS1 index mapping is missing"
grep -q "K380_WS2_INDEX = 1" "$indicator_c" || fail "WS2 index mapping is missing"
grep -q "K380_WS3_INDEX = 2" "$indicator_c" || fail "WS3 index mapping is missing"
grep -q "K380_WS4_INDEX = 3" "$indicator_c" || fail "WS4 index mapping is missing"
write_order="$(sed -n '/write_order\[\]/,/};/p' "$indicator_c" | tr -d '[:space:]')"
test "$write_order" = "staticconstuint8_twrite_order[]={K380_WS3_INDEX,K380_WS2_INDEX,K380_WS1_INDEX,};" ||
  fail "B3 sequence is not WS3 -> WS2 -> WS1"
power_rejected_body="$(sed -n '/case K380_STATUS_POWER_REJECTED:/,/break;/p' "$indicator_c")"
printf '%s\n' "$power_rejected_body" | grep -q "pixels\[K380_WS4_INDEX\]" ||
  fail "B6 does not map to WS4"
printf '%s\n' "$power_rejected_body" | grep -Eq "pixels\[K380_WS[123]_INDEX\]" &&
  fail "B6 must be WS4 only"

grep -q "bool const del_recovery = k380_del_recovery_pressed();" "$main_c" ||
  fail "Del recovery is not sampled once for DFU selection"
grep -q "gpregret == DFU_MAGIC_SERIAL_ONLY_RESET) && !del_recovery" "$main_c" ||
  fail "Del recovery does not override CDC-only DFU magic"
grep -q "gpregret == DFU_MAGIC_UF2_RESET) || del_recovery" "$main_c" ||
  fail "Del recovery does not force UF2 DFU mode"
