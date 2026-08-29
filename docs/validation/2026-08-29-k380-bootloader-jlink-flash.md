# K380 Bootloader 验证记录

- 日期：2026-08-29
- 记录类型：J-Link/SWD 烧写 + USB 枚举复核
- Bootloader repo commit：`6d99212`
- J-Link：`C:\Program Files\SEGGER\JLink_V970\JLink.exe`
- J-Link GDB Server：`C:\Program Files\SEGGER\JLink_V970\JLinkGDBServerCL.exe`
- J-Link serial：`851000967`
- target：`NRF52840_XXAA`
- interface：`SWD`
- speed：`4000 kHz`
- artifact：`.artifacts/k380-bootloader/33231812234/k380_bootloader-upstream-baseline-c67f0bcf-34-g6d99212_s140_6.1.1.hex`

## 结果

- `JLink.exe` 通过绝对路径成功识别探针，VTref=2.722V。
- `loadfile` 成功写入并验证 merged HEX。
- `reset` 与 `go` 成功执行。
- 本机 `Get-PnpDevice -PresentOnly` 仅看到 J-Link 相关 USB 设备和无关的蓝牙键盘项，未看到 K380 bootloader 的 UF2 / CDC / Mass Storage 枚举。
- `Get-Volume` 未看到带 `K380BOOT` 或类似卷标的可见 USB 卷。

## 限制

- 当前未看到 K380 通过 USB 枚举到本机，B1-B6 目视验证仍不可执行。
- 当前未执行 B1-B6 目视验证。
- temporary directory cleanup result：递归删除 `.artifacts/k380-bootloader/33231812234` 被当前命令策略阻止，目录仍保留。
