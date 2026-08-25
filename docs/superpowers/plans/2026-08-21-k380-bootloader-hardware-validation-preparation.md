# K380 Bootloader Hardware Validation Preparation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a durable K380 Bootloader hardware-validation record that preserves the CI artifact identity, exact SWD and USB observations, and deferred status until a physical board is available.

**Architecture:** Create one K380-specific Markdown record in the Bootloader repository. It is the source of truth for physical Bootloader evidence and links back to the immutable CI and memory-layout contract; it must not change firmware, linker, board definitions, or CI. Update the original bring-up plan with a link to this record so the remaining hardware gate is discoverable from the implementation history.

**Tech Stack:** Markdown, GitHub Actions artifacts, `nrfjprog`, USB enumeration tools, multimeter, PowerShell.

---

## File Structure

| File | Responsibility |
| --- | --- |
| `docs/k380/bootloader-hardware-validation.md` | Records the exact artifact, board identity, SWD/UICR/VDD checks, USB enumeration, CDC-only path, bootloader cold-start Del recovery, failures, and completion state. |
| `docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md` | Links the original bring-up plan to the physical validation record without marking hardware checks complete. |

### Task 1: Add the K380 Physical Validation Record

**Files:**
- Create: `docs/k380/bootloader-hardware-validation.md`
- Test: `docs/k380/bootloader-hardware-validation.md`

- [ ] **Step 1: Run the structural check before the record exists**

Run:

```powershell
Test-Path docs/k380/bootloader-hardware-validation.md
```

Expected: `False`.

- [ ] **Step 2: Create the physical validation record**

Create `docs/k380/bootloader-hardware-validation.md` with this content:

```markdown
# K380 Bootloader 实板验证记录

**状态：** 未执行。实板、SWD 调试器和测量设备到位前，任何检查不得改为通过。

**范围：** 本记录只覆盖 K380 Adafruit nRF52 Bootloader 的 SWD 首刷、电源配置、USB 枚举和
ZMK `Fn+Del -> &bootloader` 运行态入口、应用 UF2、矩阵、电池、蓝牙和 WS2812B 属于各自的
后续实板验证。

## 不变的构建契约

| 项目 | 已确认值 |
| --- | --- |
| Bootloader 合并提交 | `476577baf9134af8373f420d88a46e3ca2d4d5d9` |
| SoC | nRF52840-QIAA |
| Bootloader/S140 首刷工件 | `k380_bootloader-*_s140_6.1.1.hex` |
| USB UF2+CDC | `0x303A:0x1011` |
| USB CDC-only | `0x303A:0x1012` |
| UF2 卷标 | `K380BOOT` |
| REGOUT0 | 2.7 V |
| DCDC0 | 禁用 |
| DCDC1 | 启用 |
| 应用窗口 | `0x00026000..0x000EA000` |
| Bootloader 区域起点 | `0x000F4000` |

构建工件必须来自通过的 `K380 Bootloader` 工作流。`update-k380_bootloader-*_nosd.uf2` 仅用于
更新 Bootloader 本身，不能代替 SWD 首刷工件，也不能作为 ZMK 应用 UF2 使用。

## 测试会话

| 字段 | 记录 |
| --- | --- |
| 测试日期 | 未执行 |
| 操作者 | 未执行 |
| PCB revision | 未执行 |
| Bootloader Git 提交 | 未执行 |
| GitHub Actions run URL | 未执行 |
| Artifact 名称 | 未执行 |
| merged HEX 文件名 | 未执行 |
| merged HEX SHA-256 | 未执行 |
| SWD 调试器与版本 | 未执行 |
| USB 主机与操作系统 | 未执行 |
| 万用表型号 | 未执行 |

## 预检

- [ ] 已确认 SWDIO、SWCLK、RESET、GND 和 VTref 测试点连接可靠，且 VTref 为 nRF VDD。
- [ ] 已下载通过的 `k380-bootloader` artifact，并记录 merged HEX 的 SHA-256。
- [ ] 已确认 USB-C 线可传输数据。
- [ ] 已断开或隔离任何不应在首刷过程中供电的外部设备。

使用 PowerShell 记录首刷工件的 SHA-256：

```powershell
Get-FileHash .\k380_bootloader-*_s140_6.1.1.hex -Algorithm SHA256
```

## SWD 首刷与电源配置

`nrfjprog --recover` 会擦除设备，包括 Flash 和 UICR。仅在确认该板可以被完全擦除时执行。

1. 先执行恢复：

   ```powershell
   nrfjprog --recover -f NRF52
   ```

2. 再编程、校验并复位：

   ```powershell
   nrfjprog --program .\k380_bootloader-*_s140_6.1.1.hex --verify -f NRF52 --reset
   ```

3. 读取 UICR `REGOUT0`：

   ```powershell
   nrfjprog --memrd 0x10001304 --w 32 -f NRF52
   ```

4. 在 USB 供电和接近 2.75 V 的电池供电条件下测量 VDDH 与 VDD。

| 检查 | 通过判据 | 结果 |
| --- | --- | --- |
| SWD recover | 命令退出码为 0 | 未执行 |
| merged HEX 编程 | `--program` 和 `--verify` 均退出码为 0 | 未执行 |
| `REGOUT0` | 读取值的 `VOUT` 字段为 2.7 V 配置 | 未执行 |
| VDD | 实测为 2.7 V，记录测量值和容差 | 未执行 |
| DCDC0 | 运行时未启用 | 未执行 |
| DCDC1 | 运行时启用 | 未执行 |
| 电池低 VDDH 断开 | VDDH 低于 2.75 V 时断开 | 未执行 |

## USB UF2+CDC 与 CDC-only

连接 USB-C 后，记录操作系统可见的 USB 设备和卷标。不得仅依据设备管理器的缓存名称判定 VID/PID。

| 检查 | 通过判据 | 结果 |
| --- | --- | --- |
| UF2+CDC 枚举 | VID/PID 为 `0x303A:0x1011`，且出现 `K380BOOT` 卷 | 未执行 |
| CDC-only 路径 | VID/PID 为 `0x303A:0x1012` | 未执行 |
| USB 断开/接入 | 接入和拔出均不产生异常复位 | 未执行 |

在 Windows 上可用以下命令记录当前已连接的 USB PnP 实例：

```powershell
Get-PnpDevice -PresentOnly |
  Where-Object InstanceId -Match 'VID_303A&PID_(1011|1012)' |
  Format-List Status, Class, FriendlyName, InstanceId
```

## Bootloader Del 冷启动恢复入口

常规恢复入口由 Bootloader 冷启动检测单键 `Del` 触发：断电后按住 `Del`，再上电并保持到
USB 枚举完成。该路径在本 Bootloader 记录中验证；ZMK 仓库只单独验证运行态
`Fn+Del -> &bootloader`。

| 检查 | 通过判据 | 结果 |
| --- | --- | --- |
| Del 恢复入口 | 进入 UF2+CDC，VID/PID 为 `0x303A:0x1011` | 未执行 |

## 异常与结论

| 时间 | 步骤 | 现象 | 串口或命令输出 | 处理结果 |
| --- | --- | --- | --- | --- |
| 未执行 | 未执行 | 未执行 | 未执行 | 未执行 |

本记录仅在上述每个检查都有可复核证据时改为“通过”。任一失败项必须保留失败现象、使用的
工件 SHA-256 和测量值；不得用后续成功结果覆盖失败记录。
```

- [ ] **Step 3: Run the record structural check**

Run:

```powershell
$path = 'docs/k380/bootloader-hardware-validation.md'
$patterns = @(
  '^# K380 Bootloader 实板验证记录$',
  '476577baf9134af8373f420d88a46e3ca2d4d5d9',
  '0x303A:0x1011',
  '0x303A:0x1012',
  '0x00026000..0x000EA000',
  'nrfjprog --recover -f NRF52',
  'nrfjprog --memrd 0x10001304 --w 32 -f NRF52',
  'Get-PnpDevice -PresentOnly'
)

foreach ($pattern in $patterns) {
  if (-not (Select-String -Path $path -Pattern $pattern -Quiet)) {
    throw "missing required record content: $pattern"
  }
}
```

Expected: every assertion passes.

### Task 2: Link the Original Bring-up Plan to the Record

**Files:**
- Modify: `docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md`
- Test: `docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md`

- [ ] **Step 1: Add the physical-record reference**

Under the `## 实板验证边界` heading in
`docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md`, insert this paragraph before the
numbered validation sequence:

```markdown
每次实板验证必须使用
[`docs/k380/bootloader-hardware-validation.md`](../../k380/bootloader-hardware-validation.md) 记录
GitHub Actions run、artifact、merged HEX SHA-256、命令输出、测量值和失败现象。该记录初始状态为
“未执行”；完成某项实板操作后才能更新对应结果。
```

- [ ] **Step 2: Verify the link and deferred state**

Run:

```powershell
$plan = 'docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md'
$record = 'docs/k380/bootloader-hardware-validation.md'
if (-not (Test-Path $record)) {
  throw "missing physical validation record"
}
if (-not (Select-String -Path $plan -Pattern '\]\(\.\./\.\./k380/bootloader-hardware-validation\.md\)' -Quiet)) {
  throw "missing physical validation record link"
}
if (-not (Select-String -Path $record -Pattern '^\*\*状态：\*\* 未执行。' -Quiet)) {
  throw "physical validation record is not deferred"
}
```

Expected: every assertion passes.

### Task 3: Review Documentation Scope and Commit

**Files:**
- Modify: `docs/k380/bootloader-hardware-validation.md`
- Modify: `docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md`

- [ ] **Step 1: Confirm that no firmware or CI files changed**

Run:

```powershell
$expected = @(
  'docs/k380/bootloader-hardware-validation.md',
  'docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md'
)
$changed = git diff --name-only -- $expected
if ((Compare-Object $expected $changed)) {
  throw "unexpected documentation scope: $($changed -join ', ')"
}
```

Expected: only these paths appear:

```text
docs/k380/bootloader-hardware-validation.md
docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md
```

- [ ] **Step 2: Check whitespace and unresolved planning markers**

Run:

```powershell
git diff --check
if (rg -n ('TO' + 'DO|TB' + 'D') docs/k380/bootloader-hardware-validation.md docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md) {
  exit 1
}
```

Expected: `git diff --check` has no output, the `rg` command has no output, and the command exits with
code 0.

- [ ] **Step 3: Commit the hardware validation preparation**

Run:

```powershell
git add docs/k380/bootloader-hardware-validation.md docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md
git commit -m "docs(k380): add bootloader hardware validation record"
```

Expected: one documentation-only commit. Do not push, merge, delete a branch, or mark any hardware check
as passed without explicit user approval and physical evidence.
