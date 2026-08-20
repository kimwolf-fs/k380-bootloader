#ifndef K380_H
#define K380_H

#define UICR_REGOUT0_VALUE UICR_REGOUT0_VOUT_2V7
#define ENABLE_DCDC_0      0
#define ENABLE_DCDC_1      1

/*
 * 首版不初始化任何状态灯，也不占用 P0.13。LED4 的 WS2812B 灯效由后续独立阶段实现。
 */
#define LEDS_NUMBER 0

/*
 * K380 没有用户 DFU 按键或 OTA 按键。RESET 双击由上游 nRF52840 通用逻辑处理。
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
