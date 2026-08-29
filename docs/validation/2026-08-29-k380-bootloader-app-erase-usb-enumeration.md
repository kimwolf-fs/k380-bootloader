# K380 Bootloader 验证记录

- 日期：2026-08-29
- 记录类型：J-Link 应用区擦除 + USB 枚举复核
- Bootloader repo commit：`6d99212`
- J-Link：`C:\Program Files\SEGGER\JLink_V970\JLink.exe`
- J-Link serial：`851000967`
- target：`NRF52840_XXAA`
- interface：`SWD`
- speed：`4000 kHz`
- 参考内存布局：应用区 `0x26000-0xEA000`，bootloader 区从 `0xF4000` 起

## 过程

- 使用 J-Link Commander 仅擦除应用区 `0x26000-0xEA000`。
- 未执行整片擦除，避免一并清空 bootloader。
- 擦除后执行 `reset` 和 `go`。
- 重新检查本机 USB 枚举与卷标。

## 结果

- `JLink.exe` 成功连接探针并完成擦除，退出码 `0`。
- 擦除后本机枚举到 `USB\VID_303A&PID_1011`。
- 看到 `USB Composite Device`、`USB 大容量存储设备`、`USB 串行设备 (COM16)`。
- 看到 `K380BOOT` 卷标，盘符为 `F:`。

## 结论

- 通过。
- 该现象支持“ZMK 应用存在时 bootloader 会直接跳转/不保持 USB 枚举”的判断。
- 当前不建议用整片擦除做这个诊断，因为那会同时清掉 bootloader，本质上变成恢复操作。

## 限制

- B1-B6 目视验证仍未完成。
- 当前仅证明 bootloader 在应用区被清空后可重新以 USB 方式枚举。
