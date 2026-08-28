#pragma once

#include <stdbool.h>
#include <stdint.h>

#define BOOTLOADER_POWER_GATE_USB_PRESENT_MV 4500U

void bootloader_power_gate_init(void);
bool bootloader_power_gate_policy_flash_allowed(uint16_t vddh_mv);
uint16_t bootloader_power_gate_vddh_mv(void);
bool bootloader_power_gate_usb_power_present(void);
bool bootloader_power_gate_flash_allowed(void);
bool bootloader_power_gate_rejected(void);
void bootloader_power_gate_clear_rejected(void);
