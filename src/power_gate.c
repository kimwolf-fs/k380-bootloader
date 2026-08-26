#include "boards.h"
#include "power_gate.h"

#include "nrfx.h"
#include "nrf_power.h"

static bool flash_allowed = true;

void bootloader_power_gate_init(void) {
#if NRF_POWER_HAS_VDDH
  nrf_power_event_clear(NRF_POWER, NRF_POWER_EVENT_POFWARN);
  nrf_power_pofcon_set(NRF_POWER, true, NRF_POWER_POFTHR_V21);
  nrf_power_pofcon_vddh_set(NRF_POWER, NRF_POWER_POFTHRVDDH_V32);
  NRFX_DELAY_US(50);

  flash_allowed = !nrf_power_event_check(NRF_POWER, NRF_POWER_EVENT_POFWARN);
  if (!flash_allowed) {
    PRINTF("VDDH below 3.2V, flash writes disabled\r\n");
  }
#else
  flash_allowed = true;
#endif
}

bool bootloader_power_gate_flash_allowed(void) {
#if NRF_POWER_HAS_VDDH
  if (nrf_power_event_get_and_clear(NRF_POWER, NRF_POWER_EVENT_POFWARN)) {
    flash_allowed = false;
  }
#endif

  return flash_allowed;
}
