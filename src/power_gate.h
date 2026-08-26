#pragma once

#include <stdbool.h>

void bootloader_power_gate_init(void);
bool bootloader_power_gate_flash_allowed(void);
