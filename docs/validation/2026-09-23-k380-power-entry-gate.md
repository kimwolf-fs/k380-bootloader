# K380 Bootloader 电池供电进入门禁验证

## 范围

验证电池供电时，Fn+Del 和冷启动按住 Del 都在进入 bootloader 前判断电源状态；
电池状态不得进入 bootloader，USB 供电仍允许正常进入。

## 自动验证

| 项目 | 结果 | 证据 |
| --- | --- | --- |
| 提交 | 已完成 | `7ff08ef`，分支 `k380` |
| GitHub Actions | 通过 | run `35810645164` |
| host power-gate policy | 通过 | 电池门禁及 USB 放行策略测试 |
| status indicator behavior | 通过 | 低功耗错误提示三次红灯契约 |
| source contract | 通过 | 门禁位于 bootloader 入口前 |
| J-Link target info | 通过 | nRF52840_xxAA，SWD 4000 kHz，探针 `851000967`，VTref 约 1.81 V |
| J-Link flash | 通过 | application/bootloader 目标烧写后 `verified: true` |
| reset/info | 通过 | 烧写后复位并重新探测成功 |

自动证据不能替代 LED、按键和 USB 插拔的物理观察。

## 人工测试 H01-H04

每次测试前记录供电方式、按键动作和三颗指示灯状态；每项独立完成，
不要用 J-Link 连接状态判断 LED 或功耗。

| 编号 | 操作 | 预期现象 | 实际结果 | 判定 |
| --- | --- | --- | --- | --- |
| H01 | 仅电池，运行 ZMK，按 `Fn+Del` | 三颗红灯各闪 3 次；不进入 bootloader；应用继续运行 | 三颗灯同步红闪 3 次；无蓝灯；未进入 bootloader；普通按键正常输入 | 通过 |
| H02 | 仅电池，断电冷启动并按住 `Del` | 三颗红灯各闪 3 次；不出现蓝灯 bootloader 等待；进入应用 | 三颗灯红闪 3 次；无蓝灯；未进入 bootloader；随后进入应用 | 通过 |
| H03 | USB 供电，运行 ZMK，按 `Fn+Del` | 按既有协议进入 bootloader，显示正常 bootloader 指示 | 测试通过，正常进入 bootloader | 通过 |
| H04 | USB 供电，断电冷启动并按住 `Del` | 显示蓝灯并进入 bootloader；USB 可枚举 | 待填写 | 待判定 |

### H01 实测结果

- 供电：仅电池
- 操作：ZMK 运行态按 `Fn+Del`
- 实际：三颗灯同步红闪 3 次；无蓝灯；未进入 bootloader；普通按键仍可正常输入
- 判定：通过

### H02 实测结果

- 供电：仅电池
- 操作：断电冷启动并按住 `Del`
- 实际：三颗灯红闪 3 次；无蓝灯；未进入 bootloader；随后进入应用
- 判定：通过

### H03 实测结果

- 供电：USB
- 操作：ZMK 运行态按 `Fn+Del`
- 实际：按既有协议正常进入 bootloader
- 判定：通过

填写规则：你每次只回复一个编号和实际观察，我根据预期逐项判断通过、失败或受阻，
再给出下一项。若 H01/H02 失败，先停止后续人工测试并保留 LED/按键时序。
