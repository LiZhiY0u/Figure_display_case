# 蓝牙职责分离验证记录

日期：2026-10-10。基线：6d1814e。第一阶段输入重构由用户反馈实机通过；本阶段实机尚未验收。

## 模块边界

main 先创建 InputQueue，再调用 BluetoothService::Init。服务按 ConnectionEvents → GamepadController → MiniAppRemote 顺序启动；成功后才允许 Update。每轮只有 GamepadController 调用一次 BP32.update。

GamepadController 负责地址过滤、报告采集和输入状态机；MiniAppRemote 负责 ATT 写入、广告与手机连接归属。两者通过 ConnectionEvents 通知 UI，不互相初始化。连接队列固定 8 项，输入队列固定 16 项，零等待、满时拒绝新事件，DroppedCount 可供调试器观察。未增加常态日志。

保持目标地址、FigureCase 广播、FF10/FF11 服务、单字节 1–5 命令、配对记录、输入重复时序及弹窗 300/2000/300ms 不变。手机只接受 LE peripheral 链路，只有记录的句柄能触发手机断开通知及重开广告。

## 自动化证据

使用已有 MinGW g++，运行 `& .\esp32-firmware\tools\run-host-tests.ps1`：12 组通过。新增测试均先观察缺失实现导致的编译失败，再完成实现。连接队列和服务编排链接生产 cpp；FreeRTOS 与库边界使用替身。

使用已有 PlatformIO：

```powershell
& 'C:\Users\Cainiao\.platformio\penv\Scripts\pio.exe' run -d .\esp32-firmware
```

esp32dev 构建通过：RAM 98440 字节、Flash 789033 字节。相对基线增加 RAM 8 字节、Flash 276 字节，均小于 5%。

主机测试不覆盖真实 RTOS 调度、蓝牙射频、协议栈回调时序或 OLED 实际显示；编译通过不代表硬件验收。未烧录、未推送。

## 实机回归清单

- [ ] 冷启动打开手柄，首次连接只出现一次手柄弹窗。
- [ ] 手柄断开及重连，各出现一次对应通知，不出现手机通知。
- [ ] 小程序连接、断开、重连通知正常，断开后可重新扫描到 FigureCase。
- [ ] 两者同时连接，分别断开时不会误报另一设备。
- [ ] A 键一次确认；十字键和左摇杆长按仍为 400ms 后每 150ms 滚动。
- [ ] 小程序、编码器、串口输入与弹窗期间的菜单控制正常。

用户反馈通过后进入 App/配置整理阶段。
