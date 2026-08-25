# K380 Cold Boot Del Recovery Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make the K380 bootloader enter UF2+CDC when `Del` is held during cold boot, while keeping runtime `Fn+Del` through ZMK `&bootloader` and removing RESET double-click as a user recovery path.

**Architecture:** Keep the generic Adafruit nRF52 bootloader flow intact and add only a K380 board-specific matrix-pair recovery probe. The probe drives R4 (`P0.04`) high, reads C7 (`P0.31`) with pulldown, restores both pins to defaults, and contributes to `dfu_start` before USB DFU selection.

**Tech Stack:** C, Nordic nRF GPIO HAL, Adafruit nRF52 Bootloader, PowerShell/Python static checks, GitHub Actions for full firmware build.

---

## File Structure

| File | Responsibility |
| --- | --- |
| `src/boards/k380/board.h` | Declare the K380 `Del` recovery row/column pins and document that this is a matrix-pair recovery input, not a user DFU button. |
| `src/main.c` | Remove generic double-reset state, add the K380 cold-boot recovery probe, and include it in DFU entry selection. |
| `docs/k380/bootloader-hardware-validation.md` | Move `Del` recovery validation into bootloader scope and keep hardware evidence as `未执行`. |
| `docs/superpowers/specs/2026-08-19-k380-bootloader-bringup-design.md` | Align the original bootloader design with the root recovery contract. |
| `docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md` | Align the original implementation plan with the root recovery contract. |
| `docs/superpowers/specs/2026-08-21-k380-cdc-only-swd-helper-design.md` | Keep CDC-only helper recovery notes consistent with SWD and cold-boot `Del`. |
| `docs/superpowers/plans/2026-08-21-k380-bootloader-hardware-validation-preparation.md` | Align the validation-preparation plan with bootloader-owned `Del` recovery. |

### Task 1: Add Static Contract Test

**Files:**
- Test: inline Python command against `src/main.c`, `src/boards/k380/board.h`, and docs.

- [x] **Step 1: Run the failing contract test before implementation**

Run:

```powershell
python -c "from pathlib import Path; main=Path('src/main.c').read_text(encoding='utf-8'); board=Path('src/boards/k380/board.h').read_text(encoding='utf-8'); doc=Path('docs/k380/bootloader-hardware-validation.md').read_text(encoding='utf-8'); assert 'K380_RECOVERY_ROW_PIN' in board; assert 'K380_RECOVERY_COL_PIN' in board; assert 'k380_del_recovery_pressed' in main; assert 'k380_del_recovery_pressed()' in main; assert 'DFU_DBL_RESET' not in main; assert 'RESET 双击' not in doc; assert 'Bootloader 在冷启动检测窗口' in doc"
```

Expected: FAIL before implementation because the K380 recovery pin macros and probe are missing, or because docs still assign cold-boot `Del` to ZMK.

Actual: after fixing Windows shell quoting and UTF-8 reads, the test failed with `AssertionError` because the K380 recovery macros and probe were not present.

### Task 2: Implement K380 Cold-Boot Probe

**Files:**
- Modify: `src/boards/k380/board.h`
- Modify: `src/main.c`
- Test: inline Python command from Task 1.

- [x] **Step 1: Add K380 recovery pins to `board.h`**

Add these macros and comments after `LEDS_NUMBER`:

```c
/*
 * Cold-boot recovery only probes the Del matrix coordinate: RC(4,7).
 * R4 is driven high briefly and C7 is read with pulldown.
 */
#define K380_RECOVERY_ROW_PIN PINNUM(0, 4)
#define K380_RECOVERY_COL_PIN PINNUM(0, 31)
```

- [x] **Step 2: Add the probe to `src/main.c`**

Add a `static bool k380_del_recovery_pressed(void)` helper guarded by `K380_RECOVERY_ROW_PIN` and `K380_RECOVERY_COL_PIN`.

The helper must:

```c
nrf_gpio_cfg_input(K380_RECOVERY_COL_PIN, NRF_GPIO_PIN_PULLDOWN);
nrf_gpio_cfg_output(K380_RECOVERY_ROW_PIN);
nrf_gpio_pin_set(K380_RECOVERY_ROW_PIN);
NRFX_DELAY_US(100);
bool const pressed = nrf_gpio_pin_read(K380_RECOVERY_COL_PIN) ? true : false;
nrf_gpio_cfg_default(K380_RECOVERY_ROW_PIN);
nrf_gpio_cfg_default(K380_RECOVERY_COL_PIN);
return pressed;
```

- [x] **Step 3: Include the probe in DFU start selection**

In `check_dfu_mode()`, keep GPREGRET handling first and only scan the `Del` matrix pair after `DFU_MAGIC_SKIP` has been handled:

```c
bool dfu_start = _ota_dfu || serial_only_dfu || uf2_dfu;
...
if (dfu_skip) {
  return;
}

dfu_start = dfu_start || k380_del_recovery_pressed();
```

Leave `serial_only_dfu` unchanged so CDC-only still selects PID `0x1012`, while `Del` enters normal UF2+CDC.

- [x] **Step 4: Run the contract test**

Run the same Python command from Task 1.

Expected: PASS.

Actual: PASS.

### Task 3: Align Documentation

**Files:**
- Modify: `docs/k380/bootloader-hardware-validation.md`
- Modify: `docs/superpowers/specs/2026-08-19-k380-bootloader-bringup-design.md`
- Modify: `docs/superpowers/plans/2026-08-19-k380-bootloader-bringup.md`
- Modify: `docs/superpowers/specs/2026-08-21-k380-cdc-only-swd-helper-design.md`
- Modify: `docs/superpowers/plans/2026-08-21-k380-bootloader-hardware-validation-preparation.md`

- [x] **Step 1: Replace ZMK-owned cold-boot wording**

Use wording that says cold-boot `Del` belongs to bootloader and runtime `Fn+Del` belongs to ZMK.

- [x] **Step 2: Keep all physical results deferred**

Verify no validation result is changed from `未执行` to `通过` or `失败`.

Run:

```powershell
rg -n "\| (通过|失败) \|" docs/k380/bootloader-hardware-validation.md
```

Expected: no output and exit code 1.

Actual: no output and exit code 1.

### Task 4: Verify

**Files:**
- Test: repository static checks and available local build.

- [x] **Step 1: Check whitespace**

Run:

```powershell
git diff --check
```

Expected: no output and exit code 0.

Actual: exit code 0. Git warned that LF will be replaced by CRLF when Git touches the files; no whitespace errors were reported.

- [x] **Step 2: Check removed double-reset and added K380 recovery symbols**

Run:

```powershell
rg -n "DFU_DBL_RESET|DFU_SERIAL_STARTUP_INTERVAL" src/main.c
rg -n "K380_RECOVERY_(ROW|COL)_PIN|k380_del_recovery_pressed|DFU_MAGIC_(UF2|SERIAL_ONLY)_RESET" src/main.c src/boards/k380/board.h
```

Expected: the first command has no output and exit code 1; the second command shows the K380 recovery symbols and existing GPREGRET magic handling.

Actual: the first command had no output and exit code 1; the second command showed `K380_RECOVERY_ROW_PIN`, `K380_RECOVERY_COL_PIN`, `k380_del_recovery_pressed`, `DFU_MAGIC_UF2_RESET`, and `DFU_MAGIC_SERIAL_ONLY_RESET`.

- [x] **Step 3: Build if the local toolchain is available**

Run:

```powershell
make BOARD=k380 all
```

Expected: success if `arm-none-eabi-gcc` and Python dependencies are installed. If the toolchain is unavailable, record the exact failure and rely on static checks plus GitHub Actions for final build.

Actual: Windows `make BOARD=k380 all` failed because `make` is not in PATH. WSL `Ubuntu-24.04` has `/usr/bin/make`, but `arm-none-eabi-gcc` is not available there. Final firmware build is intentionally deferred to GitHub Actions.
