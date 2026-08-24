# K380 CDC-only SWD Trigger Helper 设计

**状态：** 设计待审阅，未实现，未执行实板验证。

## 目标

为 K380 Bootloader 的 CDC-only 枚举验证定义一个受控 SWD helper。该 helper 只用于测试
`DFU_MAGIC_SERIAL_ONLY_RESET` 路径：在复位前写入 `GPREGRET = 0x4e`，让 Bootloader 下次启动
选择 CDC-only USB 描述符并枚举为 `0x303A:0x1012`。

本阶段不改变 Bootloader 源码、不新增应用固件、不写 Flash、不写 UICR，也不把 CDC-only 作为
K380 用户恢复入口。

## 当前依据

Bootloader 已有逻辑：

- `DFU_MAGIC_SERIAL_ONLY_RESET` 为 `0x4e`。
- Bootloader 启动时读取 `NRF_POWER->GPREGRET`。
- 当 `GPREGRET == 0x4e` 时，`serial_only_dfu` 为真。
- Bootloader 随后清除 `GPREGRET`，并调用 `usb_init(serial_only_dfu)`。
- CDC-only 使用 `USB_DESC_CDC_ONLY_PID`，K380 为 `0x1012`。

因此 helper 的职责不是新增功能，而是给实板测试提供一个可重复、可审计的触发方式。

## 方案选择

采用 SWD 调试器触发流程，而不是最小应用触发流程。

理由：

- 不依赖 ZMK 应用、应用 UF2 或有效 application。
- 不扩大 Bootloader 仓库到应用固件构建。
- 只验证 Bootloader 对 `GPREGRET = 0x4e` 的既有响应。

被明确排除：

- 不创建最小 K380 application。
- 不修改 `src/main.c`、USB 描述符或 board 配置。
- 不使用 `nrfjprog --recover`。
- 不写入 Flash、UICR、Bootloader settings 或 MBR 参数页。
- 不把未实测命令写成“已验证”。

## Helper 形态

新增一个文档化脚本资源，推荐路径：

```text
tools/k380/cdc-only-swd-trigger.jlink
```

该文件面向 J-Link Commander。实施前必须确认调试器支持对 nRF52840 POWER 外设寄存器执行
32-bit 写入和复位。脚本应表达以下操作：

1. 连接 nRF52840 Cortex-M4 目标。
2. 写入 `GPREGRET = 0x0000004e`。
3. 复位并运行目标。
4. 不执行擦除、不编程、不写 UICR。

`GPREGRET` 的实际地址必须在实施阶段从 nRF52840 Product Specification、Nordic CMSIS/SVD 或
J-Link 目标符号中确认后写入脚本。确认依据应记录在验证文档中。若地址无法确认，本阶段停止，
不得生成可执行脚本。

## 数据流

1. 操作者先完成 SWD 连接和 USB 数据线预检。
2. 操作者运行 J-Link Commander 脚本。
3. SWD 写入只影响易失性的 POWER `GPREGRET`。
4. 目标复位。
5. Bootloader 读取 `GPREGRET == 0x4e`，进入 CDC-only。
6. 主机使用 USB PnP 查询确认 VID/PID 为 `0x303A:0x1012`。
7. 验证记录保存脚本提交 SHA、J-Link 版本、命令输出和 USB 枚举输出。

## 错误处理

- 如果 J-Link 无法连接目标，记录失败，不重试破坏性操作。
- 如果写寄存器命令失败，记录失败，不执行 `recover`。
- 如果复位后仍枚举 UF2+CDC 或无 USB 设备，记录实际 VID/PID 或无枚举状态。
- 如果枚举为 CDC-only 后不能恢复 UF2+CDC，使用 RESET 双击或 SWD 首刷路径恢复；该恢复过程
  单独记录，不覆盖失败记录。

## 验证门禁

无实板时只能验证：

- 脚本不包含 erase、loadfile、recover、写 Flash、写 UICR 或编程类命令。
- 脚本只允许连接目标、写入 `GPREGRET`、复位和运行目标。
- 文档引用的 magic 值、PID 和 Bootloader 逻辑与源码一致。
- 验证记录仍保持 `未执行`。

实板到位后才能验证：

- J-Link 脚本实际写入 `GPREGRET = 0x4e` 并复位。
- K380 枚举为 CDC-only `0x303A:0x1012`。
- `GPREGRET` 被 Bootloader 清除，后续普通复位不持续进入 CDC-only。
- UF2+CDC 路径仍可通过双击 RESET 进入。

## 后续计划

批准本设计后，实施计划应只包含：

1. 新增 J-Link Commander 脚本和说明文档。
2. 更新 `docs/k380/bootloader-hardware-validation.md` 的 CDC-only 触发 helper 条目。
3. 运行静态检查，确认脚本不含擦除/编程命令。
4. 提交文档与脚本，不执行硬件验证。
