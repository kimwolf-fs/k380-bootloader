# K380 首版 Bootloader Bring-up 实施计划

> **面向智能代理执行者：** 必须使用子技能：推荐使用 `superpowers:subagent-driven-development`，或使用 `superpowers:executing-plans`，按任务逐项实施本计划。步骤使用复选框（`- [ ]`）跟踪进度。

**目标：** 为 K380 的 nRF52840-QIAA 创建可由 SWD 首刷、提供 USB UF2+CDC、并保留 ZMK `&bootloader` 兼容性的首版 Adafruit nRF52 Bootloader board，并只通过 GitHub Actions 构建和交付工件。RESET 双击不再作为常规恢复路径；常规恢复入口改为 Bootloader 冷启动检测上电前按住 `Del`。

**架构：** 新增的 `src/boards/k380/` 仅提供上游 board 抽象所需的 SoC、电源、USB、UF2 和 CF2 配置；不修改通用 Bootloader 逻辑，也不占用 P0.13 或初始化 WS2812B。新增独立的 K380 工作流构建该 board、收集上游默认未复制的原始 HEX 与 linker map，并将所有首刷和分析工件作为单一 artifact 上传。

**技术栈：** Adafruit nRF52 Bootloader 0.11.0、nRF52840、S140 6.1.1、GNU Arm Embedded GCC 12.3.Rel1、Make、GitHub Actions、UF2、TinyUSB。

---

## 文件结构

| 文件 | 职责 |
| --- | --- |
| `.github/workflows/k380-bootloader.yml` | 只构建 K380，并验证和上传首刷、更新、linker map 与内存布局证据工件。 |
| `src/boards/k380/board.mk` | 供 Make 选择 nRF52840 子变体。 |
| `src/boards/k380/board.cmake` | 供 CMake 选择 nRF52840 变体，保持上游 board 文件完整性。 |
| `src/boards/k380/board.h` | 固定 K380 电源、无 LED/无 DFU 按键、USB 和 UF2 身份。 |
| `src/boards/k380/pinconfig.c` | 写入 nRF52840 的 CF2 Flash/RAM/USB/UF2 配置记录。 |
| `docs/k380/hardware-contract.md`（ZMK 仓库，后续） | 在 CI 首次成功且完成 map 审核后记录应用 Flash 边界；本计划不预填数值。 |

## 任务 1：创建 K380 board 配置

**文件：**
- 新建：`src/boards/k380/board.mk`
- 新建：`src/boards/k380/board.cmake`
- 新建：`src/boards/k380/board.h`
- 新建：`src/boards/k380/pinconfig.c`

- [x] **步骤 1：写入 Make 与 CMake 的 nRF52840 board 选择文件**

`src/boards/k380/board.mk`：

```make
MCU_SUB_VARIANT = nrf52840
```

`src/boards/k380/board.cmake`：

```cmake
set(MCU_VARIANT nrf52840)
```

- [x] **步骤 2：写入 K380 的板级身份与电源配置**

创建 `src/boards/k380/board.h`，内容如下：

```c
#ifndef K380_H
#define K380_H

#define UICR_REGOUT0_VALUE UICR_REGOUT0_VOUT_1V8
#define ENABLE_DCDC_0      0
#define ENABLE_DCDC_1      1

/*
 * 首版不初始化任何状态灯，也不占用 P0.13。LED4 的 WS2812B 灯效由后续独立阶段实现。
 */
#define LEDS_NUMBER 0

/*
 * K380 没有用户 DFU 按键或 OTA 按键。冷启动恢复入口由 Del 矩阵坐标触发。
 * 不定义 BUTTON_DFU、BUTTON_DFU_OTA 或 BUTTON_PULL。
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
```

不得增加 `BUTTON_DFU`、`BUTTON_DFU_OTA`、`LED_NEOPIXEL`、`NEOPIXEL_POWER_PIN`、`SIGNED_FW`、
`DUALBANK_FW`、`FORCE_UF2` 或 `DEFAULT_TO_OTA_DFU` 的定义。前五项会改变 K380 首版硬件资源或
入口；后三项应继续使用上游默认关闭值。

- [x] **步骤 3：写入 nRF52840 的 CF2 配置记录**

创建 `src/boards/k380/pinconfig.c`，内容如下：

```c
#include "boards.h"
#include "uf2/configkeys.h"

__attribute__((used, section(".bootloaderConfig")))
const uint32_t bootloaderConfig[] =
{
  /* CF2 START */
  CFG_MAGIC0, CFG_MAGIC1,
  5, 100,

  204, 0x100000,
  205, 0x40000,
  208, (USB_DESC_VID << 16) | USB_DESC_UF2_PID,
  209, 0xada52840,
  210, 0x20,

  0, 0, 0, 0, 0, 0, 0, 0
  /* CF2 END */
};
```

- [x] **步骤 4：执行不依赖交叉编译器的静态检查**

运行：

```powershell
git diff --check
rg -n '^#define (UICR_REGOUT0_VALUE|ENABLE_DCDC_0|ENABLE_DCDC_1|LEDS_NUMBER|USB_DESC_VID|USB_DESC_UF2_PID|USB_DESC_CDC_ONLY_PID|UF2_PRODUCT_NAME|UF2_VOLUME_LABEL|UF2_BOARD_ID)' src/boards/k380/board.h
if (rg -n 'BUTTON_DFU|BUTTON_DFU_OTA|LED_NEOPIXEL|NEOPIXEL_POWER_PIN|SIGNED_FW|DUALBANK_FW|FORCE_UF2|DEFAULT_TO_OTA_DFU' src/boards/k380) { exit 1 }
```

预期结果：`git diff --check` 无输出且退出码为 0；第二条命令显示 10 个预期宏；第三条命令无输出且退出码为 0。

- [x] **步骤 5：提交 board 配置**

```powershell
git add src/boards/k380
git commit -m "feat(k380): add nrf52840 bootloader board"
```

预期结果：产生只包含 4 个 K380 board 文件的提交。

## 任务 2：新增只构建 K380 的 GitHub Actions 工作流

**文件：**
- 新建：`.github/workflows/k380-bootloader.yml`
- 依赖：`src/boards/k380/board.mk`
- 依赖：`Makefile`

- [x] **步骤 1：写入 K380 专用工作流**

创建 `.github/workflows/k380-bootloader.yml`，内容如下：

```yaml
name: K380 Bootloader

on:
  workflow_dispatch:
  pull_request:
    branches: [k380]
    paths:
      - ".github/workflows/k380-bootloader.yml"
      - "src/**"
      - "linker/**"
      - "lib/uf2/**"
      - "tools/**"
      - "Makefile"
      - "CMakeLists.txt"
      - "docs/superpowers/specs/2026-08-19-k380-bootloader-bringup-design.md"
  push:
    branches: [k380]
    paths:
      - ".github/workflows/k380-bootloader.yml"
      - "src/**"
      - "linker/**"
      - "lib/uf2/**"
      - "tools/**"
      - "Makefile"
      - "CMakeLists.txt"
      - "docs/superpowers/specs/2026-08-19-k380-bootloader-bringup-design.md"

permissions:
  contents: read

concurrency:
  group: ${{ github.workflow }}-${{ github.ref }}
  cancel-in-progress: true

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - name: Checkout Code
        uses: actions/checkout@v4
        with:
          fetch-depth: 0
          submodules: true

      - name: Install ARM GCC
        uses: carlosperate/arm-none-eabi-gcc-action@v1
        with:
          release: "12.3.Rel1"

      - name: Install Python tools
        run: pip3 install adafruit-nrfutil uritemplate requests intelhex setuptools

      - name: Build K380 Bootloader
        run: |
          set -euxo pipefail
          make BOARD=k380 all
          make BOARD=k380 copy-artifact

      - name: Collect and validate artifacts
        run: |
          set -euxo pipefail
          mkdir -p _bin/k380
          cp _build/build-k380/k380_bootloader-*.hex _bin/k380/
          cp _build/build-k380/k380_bootloader-*.out.map _bin/k380/
          python3 - <<'PY'
          from pathlib import Path

          from intelhex import IntelHex

          softdevice_hex = IntelHex(
              "lib/softdevice/s140_nrf52_6.1.1/s140_nrf52_6.1.1_softdevice.hex"
          )
          mbr_flash_start = 0x00000000
          mbr_flash_end = 0x00001000
          softdevice_flash_start = mbr_flash_end
          softdevice_flash_end = softdevice_flash_start + 0x00025000
          softdevice_hex_start = softdevice_hex.minaddr()
          softdevice_hex_end = softdevice_hex.maxaddr() + 1
          bootloader_region_start = 0x000F4000
          dfu_app_data_reserved = 10 * 4096
          application_start = softdevice_flash_end
          application_end = bootloader_region_start - dfu_app_data_reserved

          assert softdevice_hex_start == mbr_flash_start
          assert softdevice_hex_end <= softdevice_flash_end
          assert application_start < application_end

          Path("_bin/k380/k380-memory-layout.txt").write_text(
              "mbr_flash_start=0x%08X\n"
              "mbr_flash_end=0x%08X\n"
              "s140_6.1.1_flash_start=0x%08X\n"
              "s140_6.1.1_flash_end=0x%08X\n"
              "s140_6.1.1_hex_data_end=0x%08X\n"
              "bootloader_region_start=0x%08X\n"
              "dfu_app_data_reserved=0x%08X\n"
              "zmk_application_flash_start=0x%08X\n"
              "zmk_application_flash_end=0x%08X\n"
              "zmk_application_flash_length=0x%08X\n"
              % (
                  mbr_flash_start,
                  mbr_flash_end,
                  softdevice_flash_start,
                  softdevice_flash_end,
                  softdevice_hex_end,
                  bootloader_region_start,
                  dfu_app_data_reserved,
                  application_start,
                  application_end,
                  application_end - application_start,
              ),
              encoding="ascii",
          )
          PY
          test -n "$(find _bin/k380 -maxdepth 1 -type f -name 'k380_bootloader-*_s140_6.1.1.hex' -print -quit)"
          test -n "$(find _bin/k380 -maxdepth 1 -type f -name 'update-k380_bootloader-*_nosd.uf2' -print -quit)"
          test -n "$(find _bin/k380 -maxdepth 1 -type f -name 'k380_bootloader-*_s140_6.1.1.zip' -print -quit)"
          test -n "$(find _bin/k380 -maxdepth 1 -type f -name 'k380_bootloader-*.hex' -print -quit)"
          test -n "$(find _bin/k380 -maxdepth 1 -type f -name 'k380_bootloader-*.out.map' -print -quit)"
          test -s _bin/k380/k380-memory-layout.txt
          find _bin/k380 -maxdepth 1 -type f -printf '%f\n' | sort

      - name: Upload K380 Bootloader artifact
        uses: actions/upload-artifact@v4
        with:
          name: k380-bootloader
          path: _bin/k380
          if-no-files-found: error
```

该工作流不能调用 `python3 tools/build_all.py`，也不能修改 `.github/workflows/githubci.yml` 的上游全 board
矩阵。K380 仅在针对 `k380` 的 pull request、`k380` 分支推送或手动触发时构建。

- [x] **步骤 2：检查 workflow 结构和工件模式**

运行：

```powershell
git diff --check
rg -n 'make BOARD=k380 (all|copy-artifact)|k380_bootloader-\*\.out\.map|update-k380_bootloader-\*_nosd\.uf2|k380-memory-layout\.txt|actions/upload-artifact@v4' .github/workflows/k380-bootloader.yml
```

预期结果：`git diff --check` 无输出且退出码为 0；`rg` 显示 K380 构建、map 复制、自更新 UF2 检查、内存布局证据和 artifact 上传五类语句。

- [x] **步骤 3：提交 K380 CI**

```powershell
git add .github/workflows/k380-bootloader.yml
git commit -m "ci(k380): build and publish bootloader artifacts"
```

预期结果：产生只包含 K380 专用 workflow 的提交。

## 任务 3：通过 GitHub Actions 验证构建和交付工件

**文件：**
- 验证：`.github/workflows/k380-bootloader.yml`
- 验证：`src/boards/k380/`

- [x] **步骤 1：推送功能分支并创建针对 `k380` 的 pull request**

运行：

```powershell
git push -u origin feat/k380-bootloader-bringup
```

随后在 GitHub 将 `feat/k380-bootloader-bringup` 创建为目标分支 `k380` 的 pull request。

预期结果：pull request 的 Checks 中出现名为 `K380 Bootloader` 的 workflow；该 workflow 使用
`make BOARD=k380 all`，不要求开发机安装 Arm GCC、nrfjprog 或任何 Python 包。

- [x] **步骤 2：检查 GitHub Actions 的构建日志**

在该 pull request 的 `K380 Bootloader` workflow 中确认下列命令均成功：

```text
make BOARD=k380 all
make BOARD=k380 copy-artifact
```

预期结果：`Build K380 Bootloader` 成功；日志显示 nRF52840、S140 `6.1.1`，且没有
`DEFAULT_TO_OTA_DFU`、签名固件或 dual-bank 的 K380 专用启用记录。

- [x] **步骤 3：检查上传的 artifact 内容**

下载名为 `k380-bootloader` 的 artifact，确认压缩包中至少包含以下文件模式：

```text
k380_bootloader-*_s140_6.1.1.hex
update-k380_bootloader-*_nosd.uf2
k380_bootloader-*_s140_6.1.1.zip
k380_bootloader-*.hex
k380_bootloader-*.out.map
k380-memory-layout.txt
```

预期结果：

- `*_s140_6.1.1.hex` 是 SWD 首刷所用的 Bootloader 与 S140 合并 HEX。
- `update-*_nosd.uf2` 仅用于更新 Bootloader 本身。
- `.zip` 是上游 CDC DFU 包。
- 非 merged 的 `k380_bootloader-*.hex` 和 `.out.map` 均存在，供首刷诊断和应用分区分析使用。
- `k380-memory-layout.txt` 由 CI 解析实际 S140 6.1.1 HEX，并记录 ZMK 分区计算所需的输入与结果。

- [x] **步骤 4：在 GitHub 验证成功后提交或合并前的最终检查**

运行：

```powershell
git status --short
git log --oneline k380..HEAD
```

实际结果：功能分支已合并至 `k380`，合并提交为
`476577baf9134af8373f420d88a46e3ca2d4d5d9`。该分支包含 K380 board、K380 专用 CI、
构建工件和内存布局验证；合并后的 `k380` 工作区保持干净。

## 任务 4：以首次成功构建的 linker map 和内存布局证据作为 ZMK 分区门禁

**文件：**
- 输入：GitHub artifact 中的 `k380_bootloader-*.out.map`
- 输入：GitHub artifact 中的 `k380-memory-layout.txt`
- 修改（仅在完成 map 审核后）：`E:\project\k380-keyboard\zmk\docs\k380\hardware-contract.md`

- [x] **步骤 1：从 map 确认 Bootloader 保留区域**

在下载后的 artifact 目录中运行：

```powershell
Select-String -Path '.\k380_bootloader-*.out.map' -Pattern 'Memory Configuration|FLASH|BOOTLOADER_CONFIG|MBR_PARAMS_PAGE|BOOTLOADER_SETTINGS' -Context 0,8
```

预期结果：map 的 Memory Configuration 显示：

```text
FLASH                 0x000f4000  0x00009800
BOOTLOADER_CONFIG     0x000fd800  0x00000800
MBR_PARAMS_PAGE       0x000fe000  0x00001000
BOOTLOADER_SETTINGS   0x000ff000  0x00001000
```

这些地址与 `linker/nrf52840.ld` 的保留区定义一致。该步骤只确认 Bootloader 侧的保留区；
`0x000F4000` 是 Bootloader 区起点，不是 ZMK 可写应用末端。

- [x] **步骤 2：读取 CI 生成的应用 Flash 边界证据**

在同一成功构建下载的 artifact 目录中运行：

```powershell
Get-Content '.\k380-memory-layout.txt'
Get-FileHash '.\k380_bootloader-*.out.map' -Algorithm SHA256
```

预期结果：文件内容包含下列边界：

```text
mbr_flash_start=0x00000000
mbr_flash_end=0x00001000
s140_6.1.1_flash_start=0x00001000
s140_6.1.1_flash_end=0x00026000
s140_6.1.1_hex_data_end=0x00025DE8
bootloader_region_start=0x000F4000
dfu_app_data_reserved=0x0000A000
zmk_application_flash_start=0x00026000
zmk_application_flash_end=0x000EA000
zmk_application_flash_length=0x000C4000
```

保存 `k380-memory-layout.txt`、map 的 SHA-256、构建提交 SHA、workflow run 编号和 artifact 名称。
S140 的 HEX 同时携带 MBR，故其实际数据从 `0x00000000` 开始；`s140_6.1.1_hex_data_end` 是
原始 HEX 的末端，S140 以 4 KiB MBR 加 `SD_FLASH_SIZE=0x25000` 预留至页边界
`0x00026000`，该地址才是应用起点。`0x000EA000` 是上游 USB UF2 的 `USER_FLASH_END`，由
`BOOTLOADER_REGION_START - DFU_APP_DATA_RESERVED` 计算得到；`0x000EA000` 至 `0x000F4000`
的 40 KiB 是上游保留的应用数据区，不能分配给 ZMK。Bootloader settings 和 MBR 参数页也不属于应用。

- [x] **步骤 3：创建独立 ZMK 分区变更，不在本分支预写数值**

在 map 审核完成后，在 `E:\project\k380-keyboard\zmk` 从 `k380` 创建新的功能分支：

```powershell
git -C E:\project\k380-keyboard\zmk switch k380
git -C E:\project\k380-keyboard\zmk switch -c feat/k380-flash-partitions
```

实际结果：ZMK 已在独立变更中将已审核的 map 与 `k380-memory-layout.txt` 证据、应用 Flash
区间 `0x00026000..0x000EA000`（长度 `0x000C4000`）记录到
`docs/k380/hardware-contract.md`，并单独创建 K380 `fixed-partitions`。ZMK CI 检查生成
DTS、内部 Flash HEX 和应用 UF2 均不越过该窗口；本 Bootloader 仓库未创建 ZMK 分区文件。

## 本仓库收尾状态

K380 Bootloader 的 board 配置、专用 CI、工件交付、linker map 审核及 ZMK 分区门禁均已完成。
本阶段没有待实现的 Bootloader 源码、链接器或 CI 功能。下列实板验证保留为未完成任务，不得
因 CI 或跨仓库构建成功而勾选。

## 实板验证边界

本实施计划的 GitHub Actions 只能证明 K380 Bootloader 可构建并产生完整工件，不能证明硬件行为已经通过。
每次实板验证必须使用
[`docs/k380/bootloader-hardware-validation.md`](../../k380/bootloader-hardware-validation.md) 记录
GitHub Actions run、artifact、merged HEX SHA-256、命令输出、测量值和失败现象。该记录初始状态为
“未执行”；完成某项实板操作后才能更新对应结果。
在合并至 `k380` 后，使用 artifact 的 merged HEX 按以下顺序执行实板 bring-up：

1. 通过 SWD 擦除并首刷 merged `*_s140_6.1.1.hex`。
2. 读取 `UICR.REGOUT0` 并测量 VDD，确认 REG0 输出为 1.8 V；确认 DCDC0 未启用、DCDC1 已启用，并验证 DFU/UF2 写入前的 VDDH 门禁。
3. 使用 USB-C 枚举 UF2+CDC，确认 USB ID 为 `0x303A:0x1011`。
4. 触发 CDC-only 路径，确认 USB ID 为 `0x303A:0x1012`。
5. 确认常规恢复路径为 Bootloader 冷启动检测上电前按住 `Del`；ZMK 仓库单独验证运行态 `Fn+Del -> &bootloader`。

不要将 LED4 状态灯、ZMK `&bootloader` 跳转、ZMK 应用 UF2、电池采样、蓝牙或无二极管矩阵标记为本计划已验证；它们依赖后续独立的 ZMK board、分区和实板测试。
