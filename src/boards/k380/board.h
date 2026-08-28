#ifndef K380_H
#define K380_H

#define UICR_REGOUT0_VALUE UICR_REGOUT0_VOUT_1V8
#define ENABLE_DCDC_0      0
#define ENABLE_DCDC_1      1

#define K380_BOOTLOADER_STATUS_INDICATOR 1
#define LED_NEOPIXEL PINNUM(0, 13)
#define NEOPIXELS_NUMBER 4
#define LEDS_NUMBER 0

/*
 * Cold-boot recovery only probes the Del matrix coordinate: RC(4,7).
 * R4 is driven high briefly and C7 is read with pulldown.
 */
#define K380_RECOVERY_ROW_PIN PINNUM(0, 4)
#define K380_RECOVERY_COL_PIN PINNUM(0, 31)

/*
 * K380 没有用户 DFU 按键或 OTA 按键。冷启动恢复入口由上面的 Del 矩阵坐标触发。
 * 不定义任何按键输入相关宏。
 */

/*
 * 这些字符串满足上游 USB/BLE 描述符的编译依赖；K380 不对用户提供 BLE OTA。
 */
#define BLEDIS_MANUFACTURER "K380"
#define BLEDIS_MODEL        "K380 Keyboard"

#define USB_DESC_VID          0x303A
#define USB_DESC_UF2_PID      0x1011
#define USB_DESC_CDC_ONLY_PID 0x1012

#define UF2_PRODUCT_NAME "K380 Keyboard"
#define UF2_VOLUME_LABEL "K380BOOT"
#define UF2_BOARD_ID     "nRF52840-K380-v1"
#define UF2_INDEX_URL    "https://github.com/kimwolf-fs/k380-bootloader"

#endif
