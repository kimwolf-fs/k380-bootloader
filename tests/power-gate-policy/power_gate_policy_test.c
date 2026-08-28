#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "power_gate.h"

int main(void) {
  assert(!bootloader_power_gate_policy_flash_allowed(0));
  assert(!bootloader_power_gate_policy_flash_allowed(3200));
  assert(!bootloader_power_gate_policy_flash_allowed(4500));
  assert(bootloader_power_gate_policy_flash_allowed(4501));
  assert(bootloader_power_gate_policy_flash_allowed(5000));

  bootloader_power_gate_clear_rejected();
  assert(!bootloader_power_gate_rejected());

  bootloader_power_gate_test_set_vddh_mv(4500);
  assert(!bootloader_power_gate_flash_allowed());
  assert(bootloader_power_gate_rejected());

  bootloader_power_gate_clear_rejected();
  assert(!bootloader_power_gate_rejected());

  bootloader_power_gate_test_set_vddh_mv(4501);
  assert(bootloader_power_gate_flash_allowed());
  assert(!bootloader_power_gate_rejected());

  return 0;
}
