# K380 Bootloader 实板验证记录

**状态：** 未执行。实板、SWD 调试器和测量设备到位前，任何检查不得改为通过。

## 验证范围

本记录仅覆盖 K380 Adafruit nRF52 Bootloader 的以下实板验证：SWD 首次烧录、电源配置、USB 枚举、RESET 双击进入 UF2 模式。

以下项目属于后续独立的硬件验证，不在本记录范围内：ZMK `&bootloader`、应用 UF2、矩阵、BLE、WS2812B，以及电池功能，即 VDDHDIV5 采样、低电量行为/UI 和充电状态解释。VDDH <2.75 V 断开检查仍在本记录范围内，但仅作为 Bootloader/电源路径的电气验收，不是电池功能验证。

## 不可变契约

| 项目 | 约定值 |
| --- | --- |
| 合并提交 | `476577baf9134af8373f420d88a46e3ca2d4d5d9` |
| 芯片 | nRF52840-QIAA |
| SWD 首次烧录 HEX 模式 | `k380_bootloader-*_s140_6.1.1.hex` |
| USB | `0x303A:0x1011` |
| CDC-only | `0x303A:0x1012` |
| UF2 卷标 | `K380BOOT` |
| REGOUT0 | 2.7 V |
| DCDC0 | disabled |
| DCDC1 | enabled |
| 应用范围 | `0x00026000..0x000EA000` |
| Bootloader 起始地址 | `0x000F4000` |

源文件必须来自成功的 `K380 Bootloader` workflow。`update-k380_bootloader-*_nosd.uf2` 仅用于更新 bootloader，既不是 SWD 首次烧录 artifact，也不是 ZMK 应用 UF2。

## 会话元数据

| 项目 | 结果 |
| --- | --- |
| 日期 | 未执行 |
| 操作者 | 未执行 |
| PCB revision | 未执行 |
| Bootloader git commit | 未执行 |
| Actions run URL | 未执行 |
| artifact name | 未执行 |
| 合并 HEX name + SHA256 | 未执行 |
| SWD debugger/version | 未执行 |
| USB host/OS | 未执行 |
| 万用表型号 | 未执行 |

## 测试前检查

- [ ] 实板、SWD 调试器、USB 线和测量设备已到位
- [ ] 已确认目标芯片为 nRF52840-QIAA
- [ ] 已确认 artifact 来自成功的 `K380 Bootloader` workflow
- [ ] 已确认合并提交为 `476577baf9134af8373f420d88a46e3ca2d4d5d9`
- [ ] 已核对 HEX 文件名符合 `k380_bootloader-*_s140_6.1.1.hex`
- [ ] 已记录 HEX SHA256
- [ ] 已用万用表确认 SWDIO、SWCLK、RESET、GND、VTref 各测试点分别与预期目标网络导通
- [ ] 已测量 VTref，并确认其等于 nRF VDD
- [ ] 已确认 USB-C 具备数据能力
- [ ] 已确认外部供电已隔离

## SWD 首次烧录与电源

> **危险提示：** `nrfjprog --recover` 会擦除 Flash 和 UICR。执行前必须确认这是可擦除的目标板，并准备好重新烧录所需的 artifact。

```powershell
$hexFiles = Get-ChildItem -File -Filter 'k380_bootloader-*_s140_6.1.1.hex'
if ($hexFiles.Count -ne 1) {
  throw "Expected exactly one merged bootloader HEX artifact; found $($hexFiles.Count)."
}
$hexFile = $hexFiles[0]
Get-FileHash $hexFile.FullName -Algorithm SHA256
nrfjprog --recover -f NRF52
nrfjprog --program $hexFile.FullName --verify -f NRF52 --reset
nrfjprog --memrd 0x10001304 --w 32 -f NRF52
```

### Bootloader 运行期间的 DCDC 读取

Bootloader 启动后，必须在 Bootloader 正在运行期间执行以下 SWD 读取，以复核运行时 DCDC 配置：

```powershell
nrfjprog --memrd 0x40000580 --w 32 -f NRF52
nrfjprog --memrd 0x40000578 --w 32 -f NRF52
```

`DCDCEN0` at `0x40000580` must read disabled/0 for DCDC0；`DCDCEN` at `0x40000578` must read enabled/1 for DCDC1。

| 检查项 | 结果 | 证据 |
| --- | --- | --- |
| recovery | 未执行 | 未执行 |
| program/verify | 未执行 | 未执行 |
| REGOUT0 VOUT 2.7 V | 未执行 | 未执行 |
| VDD measurement | 未执行 | 未执行 |
| DCDC0 off | 未执行 | 未执行 |
| DCDC1 on | 未执行 | 未执行 |
| battery VDDH <2.75 V disconnect | 未执行 | 未执行 |

REGOUT0 只有在完整记录 `nrfjprog --memrd 0x10001304 --w 32 -f NRF52` 输出并确认 VOUT 字段为 2.7 V 配置后，才可判定通过。Bootloader 运行期间必须记录原始 VDD 测量值、测量点、万用表型号和校准/准确度；项目负责人须在本记录中补充批准的 VDD 数值验收容差，之前 VDD 行及整个电源部分均不得标记通过，状态保持 `未执行`。低 VDDH 断开测试须使用可控的电池侧电源，并记录 2.75 V 上方和下方的 VDDH 读数及对应的已连接/已断开行为；缺少两侧读数不得判定通过。DCDC0/DCDC1 还必须分别保留 `0x40000580` 和 `0x40000578` 的寄存器输出作为证据，并满足前述 disabled/0 与 enabled/1 判据。

## USB 枚举

| 检查项 | 结果 | 必需证据 |
| --- | --- | --- |
| UF2 + CDC：VID/PID `0x303A:0x1011` 且卷标 `K380BOOT` | 未执行 | 未执行 |
| CDC-only：VID/PID `0x303A:0x1012` | 未执行 | 未执行 |
| USB connect/disconnect | 未执行 | 未执行 |

```powershell
Get-PnpDevice -PresentOnly |
  Where-Object InstanceId -Match 'VID_303A&PID_(1011|1012)' |
  Format-List Status, Class, FriendlyName, InstanceId
```

```powershell
Get-Volume |
  Where-Object FileSystemLabel -EQ 'K380BOOT' |
  Format-List DriveLetter, FileSystemLabel, DriveType, HealthStatus
```

UF2 + CDC 行只有在附加或转录上述卷标查询输出和现有 VID/PID PnP 输出后，才可判定通过。

CDC-only 不会由 RESET 或 RESET 双击进入；仅当 Bootloader 发现 `NRF_POWER->GPREGRET` 等于 `0x4e`（`DFU_MAGIC_SERIAL_ONLY_RESET`）时选择该模式。CDC-only 检查暂缓至具有经批准的触发 helper：可为单独记录的测试 image，或经批准的 SWD debugger 操作流程写入该 magic 后复位。本记录不声明任何特定 SWD 写入命令已经测试。执行时必须随 `0x303A:0x1012` 的 PnP 证据记录 helper 的来源/commit 或 debugger 操作流程及其证据；此前该行保持 `未执行`。

USB connect/disconnect 检查从已枚举的 UF2 + CDC 会话开始。连续 10 次拔出并重新插入 USB 线，每次均须观察 `0x303A:0x1011` 先消失再重新出现，期间除有意的拔插事件外不得复位目标。记录为 `x/10`，只有 `10/10` 通过。

## RESET 双击

本检查以单独烧录且独立验证为有效的 K380 application 为前提，因为不存在有效 application 时 Bootloader 会进入 DFU。该 application 仅为测试前提；application 烧录和 ZMK `&bootloader` 均不在本记录范围内。没有可用的有效 application 时，本检查保持 `未执行`。

每个可复现循环均从 application 正常运行且尚未枚举为 UF2 开始：在 500 ms 内双击 RESET，确认 UF2 + CDC `0x303A:0x1011`；随后单击 RESET，并等待超过 500 ms 让有效 application 恢复，再开始下一次尝试。连续执行 10 次，记录成功数 `x/10`；只有 `10/10` 才可判定通过。

| 检查项 | 结果 | 必需证据 |
| --- | --- | --- |
| RESET 双击后进入 UF2 + CDC，VID/PID `0x303A:0x1011` | 未执行 | 未执行 |
| RESET 双击 10 次连续尝试成功数 `x/10`，通过条件 `10/10` | 未执行 | 未执行 |

## 文档结构检查

以下原生 PowerShell 检查只验证本记录包含必要的程序和验收约束，不执行硬件操作：

```powershell
$record = 'docs/k380/bootloader-hardware-validation.md'
$text = Get-Content -Raw $record
$required = @(
  "Get-ChildItem -File -Filter 'k380_bootloader-*_s140_6.1.1.hex'",
  '$hexFile.FullName',
  'NRF_POWER->GPREGRET',
  '0x4e',
  'DFU_MAGIC_SERIAL_ONLY_RESET',
  '独立验证为有效的 K380 application',
  '10/10',
  'VDDHDIV5',
  'VDDH <2.75 V',
  '批准的 VDD 数值验收容差'
)
foreach ($needle in $required) {
  if (-not $text.Contains($needle)) { throw "Missing required record content: $needle" }
}
if ($text -match 'nrfjprog --program\s+\.\\k380_bootloader-\*_s140_6\.1\.1\.hex') {
  throw 'Wildcard must not be passed directly to nrfjprog --program.'
}
if ($text -match '\| (通过|失败) \|') {
  throw 'Physical result cells must remain 未执行.'
}
```

## 异常记录

| 时间 | 步骤 | 现象 | 串口或命令输出 | 处理结果 |
| --- | --- | --- | --- | --- |
| 未执行 | 未执行 | 未执行 | 未执行 | 未执行 |

## 结论

只有具备可复核证据时才可标记通过；必须保留失败症状、artifact SHA256 和测量值，后续成功不得覆盖失败记录。
