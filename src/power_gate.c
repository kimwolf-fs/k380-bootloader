#include "power_gate.h"

bool bootloader_power_gate_policy_flash_allowed(uint16_t vddh_mv) {
  return vddh_mv > BOOTLOADER_POWER_GATE_USB_PRESENT_MV;
}

#ifndef BOOTLOADER_POWER_GATE_POLICY_TEST
#include "boards.h"

#if NRF_POWER_HAS_VDDH
#define BOOTLOADER_POWER_GATE_SAADC_FULL_SCALE_MV 3600U
#define BOOTLOADER_POWER_GATE_VDDH_DIVIDER       5U
#define BOOTLOADER_POWER_GATE_SAADC_DENOMINATOR 1024U
#endif

void bootloader_power_gate_init(void) {
}

uint16_t bootloader_power_gate_vddh_mv(void) {
#if NRF_POWER_HAS_VDDH
  volatile int16_t raw = 0;

  NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos;
  NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_10bit << SAADC_RESOLUTION_VAL_Pos;
  NRF_SAADC->CH[0].CONFIG =
      (SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos) |
      (SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos) |
      (SAADC_CH_CONFIG_GAIN_Gain1_6 << SAADC_CH_CONFIG_GAIN_Pos) |
      (SAADC_CH_CONFIG_REFSEL_Internal << SAADC_CH_CONFIG_REFSEL_Pos) |
      (SAADC_CH_CONFIG_TACQ_10us << SAADC_CH_CONFIG_TACQ_Pos) |
      (SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos) |
      (SAADC_CH_CONFIG_BURST_Disabled << SAADC_CH_CONFIG_BURST_Pos);
  NRF_SAADC->CH[0].PSELP =
      SAADC_CH_PSELP_PSELP_VDDHDIV5 << SAADC_CH_PSELP_PSELP_Pos;
  NRF_SAADC->CH[0].PSELN = SAADC_CH_PSELN_PSELN_NC << SAADC_CH_PSELN_PSELN_Pos;
  NRF_SAADC->RESULT.PTR = (uint32_t)&raw;
  NRF_SAADC->RESULT.MAXCNT = 1;
  NRF_SAADC->EVENTS_STARTED = 0;
  NRF_SAADC->EVENTS_END = 0;
  NRF_SAADC->TASKS_START = 1;
  while (NRF_SAADC->EVENTS_STARTED == 0) {
  }
  NRF_SAADC->TASKS_SAMPLE = 1;
  while (NRF_SAADC->EVENTS_END == 0) {
  }
  NRF_SAADC->TASKS_STOP = 1;
  NRF_SAADC->CH[0].PSELP = SAADC_CH_PSELP_PSELP_NC << SAADC_CH_PSELP_PSELP_Pos;
  NRF_SAADC->ENABLE = SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos;

  if (raw <= 0) {
    return 0;
  }

  return (uint16_t)(((uint32_t)raw * BOOTLOADER_POWER_GATE_SAADC_FULL_SCALE_MV *
                     BOOTLOADER_POWER_GATE_VDDH_DIVIDER) /
                    BOOTLOADER_POWER_GATE_SAADC_DENOMINATOR);
#else
  return UINT16_MAX;
#endif
}

bool bootloader_power_gate_usb_power_present(void) {
  return bootloader_power_gate_policy_flash_allowed(
      bootloader_power_gate_vddh_mv());
}

bool bootloader_power_gate_flash_allowed(void) {
  const bool flash_allowed = bootloader_power_gate_usb_power_present();
  if (!flash_allowed) {
    PRINTF("VDDH at or below 4.5V, flash writes disabled\r\n");
  }

  return flash_allowed;
}
#endif
