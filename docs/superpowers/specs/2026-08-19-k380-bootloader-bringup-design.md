# K380 首版 Bootloader Bring-up 设计

**状态：** 首版 CI 验证和 ZMK 分区对接已完成；实板验证待执行。

**仓库与分支：** `kimwolf-fs/k380-bootloader` 的 `feat/k380-bootloader-bringup` 已合并至
`k380`（合并提交 `476577baf9134af8373f420d88a46e3ca2d4d5d9`）。

## 目标

为 K380 的 nRF52840-QIAA 建立首个可通过 SWD 首刷的 Adafruit nRF52 Bootloader board。
首版必须提供 USB UF2+CDC、ZMK `&bootloader` 进入 UF2 所需的基础兼容性，以及可供后续
ZMK board 使用的实际 linker map。RESET 双击不再作为常规恢复路径；常规恢复入口改为
Bootloader 冷启动检测上电前按住 `Del`。ZMK 只负责运行态 `Fn+Del -> &bootloader`。

已完成的首版判据是：GitHub Actions 成功构建并上传工件，且 linker map 已确认应用 Flash
边界。实板经 SWD 首刷后可枚举 USB；RESET 测试点仅保留为调试与救砖入口，不再承担常规
恢复路径，不能由 CI 结果替代。

## 范围

本阶段创建下列 K380 专用 board 文件：

```text
src/boards/k380/
  board.mk
  board.cmake
  board.h
  pinconfig.c
```

本阶段增加只构建 K380 的 GitHub Actions 工作流，上传构建产物和 linker map。

本阶段不实现：

- LED4 的 Bootloader 灯效或任何 WS2812B 初始化。
- ZMK board、`fixed-partitions`、矩阵 DTS、matrix transform、keymap 或 ZMK 应用 UF2。
- BLE OTA 入口、签名固件或双 bank 回滚。
- 对 Adafruit Bootloader 的通用 USB、DFU、SoftDevice 或链接器核心逻辑的重构。

## 上游基线与 SoftDevice

Bootloader 基线固定为 Adafruit nRF52 Bootloader `0.11.0`，提交
`c67f0bcf0fa8e841426335b1bbde91cda6ca1f50`。

该基线的 nRF52840 构建会固定使用 S140 `6.1.1` 和 `SOFTDEVICE_PRESENT`。S140 在本阶段仅是
上游 Bootloader 的运行依赖，不能据此宣称 K380 提供 BLE OTA。

K380 board 必须保持：

- `DEFAULT_TO_OTA_DFU=OFF`。
- 不定义 `BUTTON_DFU_OTA`。
- ZMK 后续不得请求 OTA reset magic。
- `SIGNED_FW=OFF`、`DUALBANK_FW=OFF`、`FORCE_UF2=OFF`。

首版不尝试移除 S140 或 BLE DFU 编译路径。完全移除 SoftDevice 是独立的上游核心改造项目，
不属于本 board bring-up。

## Board 配置

`board.mk` 和 `board.cmake` 选择 nRF52840。

`board.h` 负责以下 K380 专属配置：

| 项目 | 固定值 |
| --- | --- |
| SoC | nRF52840-QIAA |
| `UICR_REGOUT0_VALUE` | `UICR_REGOUT0_VOUT_1V8` |
| `ENABLE_DCDC_0` | `0` |
| `ENABLE_DCDC_1` | `1` |
| USB VID | `0x303A` |
| UF2 PID | `0x1011` |
| CDC-only PID | `0x1012` |
| UF2 产品名 | `K380 Keyboard` |
| UF2 卷标 | `K380BOOT` |
| UF2 Board ID | `nRF52840-K380-v1` |
| UF2 family | `0xADA52840` |

K380 只有 RESET 测试点，没有 DFU 或 OTA 用户按键。首版不定义普通 LED、WS2812B 或其供电
控制 GPIO，避免在 USB、Flash 和复位路径尚未验证时额外占用 P0.13 或引入时序问题。

`pinconfig.c` 使用 nRF52840 的 1 MiB Flash、256 KiB RAM、K380 USB VID/PID 和
`0xADA52840` UF2 family。该文件不定义应用起始地址；应用边界必须由首次成功构建的
linker map 得出。

## 启动与恢复行为

首版不再沿用上游 nRF52840 的双击 RESET 检测。RESET 测试点仅用于调试与救砖；Bootloader
在冷启动窗口只探测 `Del = RC(4,7)`，即 R4 `P0.04` 和 C7 `P0.31`。应用调用 ZMK
`&bootloader` 后仍必须进入 UF2+CDC。

日常应用更新使用后续 ZMK 生成、且与 linker map 应用边界一致的应用 UF2。Bootloader
构建输出的 `update-*_bootloader_*.uf2` 仅用于更新 Bootloader 本身，不能替代日常应用
UF2。

SWD 是首刷、擦除和救砖路径。首刷必须验证 `UICR.REGOUT0` 设为 1.8 V，DCDC0 不启用，
DCDC1 启用。

## Flash 边界门禁

不得在本设计、board 文件或 ZMK 仓库中猜测应用 Flash 起始地址和长度。该门禁已由 K380
Bootloader CI 工件满足。

K380 Bootloader 的 linker map 已确认：

- MBR 与 S140 6.1.1 占用 `0x00000000..0x00026000`。
- ZMK 可写应用窗口为 `0x00026000..0x000EA000`，长度 `0x000C4000`。
- Adafruit DFU/UF2 应用保存数据保留区为 `0x000EA000..0x000F4000`。
- Bootloader 和其配置页占用 `0x000F4000..0x00100000`。

这些值已回填到 ZMK 仓库的 `docs/k380/hardware-contract.md`，并已用于 K380 的
`fixed-partitions`。ZMK CI 继续负责检查生成的 DTS、内部 Flash HEX 和应用 UF2 不越过
该应用窗口。

## GitHub Actions

用户不在本地安装交叉编译工具链。K380 Bootloader 的构建验证仅在 GitHub Actions 执行。

已新增 K380 专用工作流，触发条件为：

- 推送到 `k380`。
- 针对 `k380` 的 pull request。
- 修改 `src/boards/k380/**`、K380 workflow、K380 文档或共享 Bootloader 构建文件。
- 手动触发。

工作流执行 `make BOARD=k380 all`，并上传下列 K380 工件：

- Bootloader 与 S140 合并的 SWD 首刷 `.hex`。
- Bootloader 自更新 `.uf2`。
- 生成的 `.zip`。
- 原始 bootloader `.hex`。
- linker `.map`。

首版 CI 不构建 ZMK board，也不宣称已验证实体矩阵、低电量或 WS2812B 灯效。应用分区已由
独立 ZMK CI 验证，实板应用 UF2 行为仍未验证。

## 实板验证顺序

1. 使用 SWD 擦除并首刷 CI 产出的 K380 Bootloader 工件。
2. 记录 UICR、VDD 和 VDDH 测量结果，确认 1.8 V LDO 与 DCDC 配置。
3. 使用 USB-C 确认 UF2+CDC 枚举的 VID/PID 为 `0x303A:0x1011`。
4. 确认 CDC-only 路径的 VID/PID 为 `0x303A:0x1012`。
5. 确认常规恢复路径为 Bootloader 冷启动检测上电前按住 `Del`。
6. 应用 Flash 边界已记录在 ZMK 硬件契约；实板验证完成后记录使用的工件、测量值、枚举
   结果和异常项。

LED4 灯效、ZMK `&bootloader`、应用 UF2 跳转和电池模式验证属于后续阶段，必须在 ZMK
board 和应用分区参数已存在后验证。

## 后续阶段

首版 Bootloader 的源码、CI 和 ZMK 分区对接均已完成。本仓库在实板可用前不再需要新增
功能性改动。后续工作按以下顺序继续：

1. 使用已发布的 merged HEX 执行 Bootloader 的 SWD 和 USB 枚举实板验证。
2. 在 Bootloader 仓库验证冷启动 `Del` 恢复入口；在 ZMK 仓库验证运行态 `Fn+Del -> &bootloader`、应用 UF2 写入和重新启动。
3. 为 Bootloader 单独设计并实现 LED4 状态灯；该阶段必须先确认状态灯时序、颜色和亮度
   上限。
4. 完成 USB、电源、电池、蓝牙槽位、矩阵和 LED4 的整机实板验证。
