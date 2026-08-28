#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

bool bootloader_power_gate_policy_flash_allowed(uint16_t vddh_mv);

int main(void) {
  assert(!bootloader_power_gate_policy_flash_allowed(0));
  assert(!bootloader_power_gate_policy_flash_allowed(3200));
  assert(!bootloader_power_gate_policy_flash_allowed(4500));
  assert(bootloader_power_gate_policy_flash_allowed(4501));
  assert(bootloader_power_gate_policy_flash_allowed(5000));
  return 0;
}
