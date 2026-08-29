# K380 Bootloader 验证记录

- 日期：2026-08-29
- 记录类型：源码 / CI 远程合同复核
- Bootloader repo commit：`fec3618`
- 复核范围：`power_gate` VDDH 门禁、K380 WS2812B 状态映射、GitHub Actions 源码断言

## 复核方式

- 读取当前源码中的状态映射与门禁实现。
- 读取 `K380 Bootloader` workflow 中的 Python 断言。
- 不依赖本地板上目视观察。

## 已确认项

- `src/power_gate.c` 使用 `SAADC_CH_PSELP_PSELP_VDDHDIV5`。
- `src/power_gate.c` 中 `bootloader_power_gate_policy_flash_allowed()` 以 `VDDH > 4500 mV` 为允许条件。
- `src/boards/k380/board.h` 定义 `LED_NEOPIXEL PINNUM(0, 13)`、`NEOPIXELS_NUMBER 4`、`LEDS_NUMBER 0`。
- `src/boards/k380/status_indicator.c` 定义 `WS1 -> WS2 -> WS3 -> WS4` 位置序。
- `src/boards/k380/status_indicator.c` 中 B6 只点亮 `WS4` 红灯。
- `.github/workflows/k380-bootloader.yml` 断言 `4500U`、`VDDHDIV5`、`PINNUM(0, 13)`、`WS1/WS2/WS3/WS4` 关系和 B6 约束。

## 结果

- 源码合同通过。
- CI 断言合同通过。

## 限制

- 这不是实板验收。
- B1-B6 的现场可视确认仍未完成。
