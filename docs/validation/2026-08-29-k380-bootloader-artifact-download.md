# K380 Bootloader 验证记录

- 日期：2026-08-29
- 记录类型：artifact 下载与离线检查
- Bootloader repo commit：`6d99212`
- GitHub Actions run ID：`33231812234`
- workflow：`K380 Bootloader`
- artifact 名称：`k380-bootloader`
- artifact 本地目录：`.artifacts/k380-bootloader/33231812234`

## 已确认文件

- `k380_bootloader-upstream-baseline-c67f0bcf-34-g6d99212_s140_6.1.1.hex`
- `update-k380_bootloader-upstream-baseline-c67f0bcf-34-g6d99212_nosd.uf2`
- `k380_bootloader-upstream-baseline-c67f0bcf-34-g6d99212_s140_6.1.1.zip`
- `k380_bootloader-upstream-baseline-c67f0bcf-34-g6d99212.out.map`
- `k380-memory-layout.txt`

## 检查结果

| Test item | Expected | Observed | Result |
| --- | --- | --- | --- |
| Artifact 下载 | `k380-bootloader` artifact 可下载 | 已下载到 `.artifacts/k380-bootloader/33231812234` | PASS |
| HEX/UF2/ZIP/MAP | 至少包含要求的 4 个文件 | 4 个目标文件均存在，额外包含 `k380_bootloader-*.hex` 与 `k380_bootloader-*_nosd.hex` | PASS |
| Memory layout | 包含 `k380-memory-layout.txt` | 文件存在，布局值与 workflow 约定一致 | PASS |
| J-Link/SWD 烧写 | 可用 J-Link/SWD 首刷/救砖验证 | `JLinkExe` / `nrfjprog` 不在 PATH，当前环境无实板工具 | BLOCKED |
| USB B1-B6 观察 | 记录 B1-B6 实板显示 | 当前环境无硬件接入，无法执行 | BLOCKED |

## 备注

- `gh run watch --repo kimwolf-fs/k380-bootloader 33231812234 --exit-status` 成功。
- `gh run download --name k380-bootloader` 成功。
- 由于缺少 J-Link / nrfjprog / 实板接入，本次只完成 artifact 归档前半段。
- temporary directory cleanup result：blocked by command policy on delete operations; `.artifacts/k380-bootloader/33231812234` remains on disk.
