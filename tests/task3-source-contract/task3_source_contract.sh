#!/bin/sh
set -eu

serial="lib/sdk11/components/libraries/bootloader_dfu/dfu_transport_serial.c"
ghostfat="src/usb/uf2/ghostfat.c"

fail() {
  echo "task3 source contract: $1" >&2
  exit 1
}

grep -q "SERIAL_DFU_OP_CODE_RESPONSE" "$serial" ||
  fail "serial DFU response opcode is not defined"
grep -q "SERIAL_DFU_RESP_VAL_OPER_FAILED" "$serial" ||
  fail "serial DFU operation-failed response value is not defined"
grep -q "hci_transport_tx_alloc" "$serial" ||
  fail "serial DFU rejection does not allocate an HCI TX response"
grep -q "hci_transport_pkt_write" "$serial" ||
  fail "serial DFU rejection does not write an HCI response packet"
grep -q "p_tx_buffer\\[0\\] = SERIAL_DFU_OP_CODE_RESPONSE" "$serial" ||
  fail "serial DFU response does not encode response opcode"
grep -q "p_tx_buffer\\[1\\] = procedure" "$serial" ||
  fail "serial DFU response does not encode procedure"
grep -q "p_tx_buffer\\[2\\] = response_value" "$serial" ||
  fail "serial DFU response does not encode response value"
grep -q "hci_transport_pkt_write(p_tx_buffer, 3)" "$serial" ||
  fail "serial DFU response is not the expected 3-byte payload"
grep -q "serial_dfu_operation_failed_send(packet_type)" "$serial" ||
  fail "power-gate rejection does not send operation failure"
grep -q "reject_flash_if_power_low(DATA_PACKET)" "$serial" ||
  fail "DATA_PACKET rejection does not use the power-gate response path"
grep -q "reject_flash_if_power_low(START_PACKET)" "$serial" ||
  fail "START_PACKET rejection does not use the power-gate response path"

grep -q "uf2_block_has_required_magic" "$ghostfat" ||
  fail "UF2 magic detection helper is missing"
grep -q "state->aborted = true" "$ghostfat" ||
  fail "invalid UF2 paths do not record aborted state"
