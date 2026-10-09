# FigureCase 展示柜控制系统

ESP32 展示柜固件与微信小程序，通过蓝牙手柄、手机或编码器操作 OLED 菜单。仅维护 `main` 分支。

## 目录

```text
esp32-firmware/       ESP32 PlatformIO 工程、源代码、逻辑测试
wechat-mini-program/ 微信小程序 BLE 遥控器
资料/
  hardware/          原理图及 PCB 工程文件、PDF
  docs/              软件设计、历史计划、图标预览
  images/            历史学习配图
```

固件目录原名 `2.controller_test`。历史 OLED 测试和 Bad Apple 示例已移除，可从 Git 历史恢复。

## 当前功能

- SH1106 128×64 OLED：Light、Editor、Volt、Setting 菜单，导航、参数窗口、睡眠/唤醒、运行时动画曲线选择。
- 手柄地址过滤：`C0:D6:D5:EE:DB:8A`；保留配对记录，支持首次配对与重连。首次配对仍需手柄进入配对模式。
- A 键按下只确认一次；左摇杆及十字键支持持续滚动：首次立即移动，等待 400ms 后每 150ms 一项。摇杆触发阈值 ±500，双轴均进入 ±200 时回中；十字键优先。
- 微信小程序连接与上/下/左/右/确定五键遥控。
- 手柄与小程序独立连接/断开通知：右侧滑入 300ms、停留 2 秒、向右滑出 300ms。
- 手柄按键日志默认关闭；启动和连接日志保留，串口 115200。调试时在 build_flags 加 `-D ENABLE_GAMEPAD_INPUT_LOG=1`。

灯带目前运行彩虹演示，灯光菜单参数尚未完整接入实际灯效；电机有驱动代码，但完整展示柜业务尚未实现。Editor 部分菜单为占位功能。硬件文件归档不代表完成电气审核或实板验证。

## 当前软件设计

| 模块 | 职责 |
|---|---|
| `src/main.cpp`、`src/App` | Arduino 薄入口、启动生命周期、四个现有任务的创建与失败检查 |
| `src/Bluetooth` | 统一蓝牙生命周期、手柄管理、小程序 ATT 服务、独立连接通知队列 |
| `src/Input` | 纯手柄状态机、统一输入事件队列与输入适配 |
| `src/Diagnostics` | 独立可开关手柄日志，不参与控制逻辑 |
| `src/OledUI` | 薄入口、内部上下文、输入、公共绘制、分组页面、导航与独立连接弹窗 |
| `src/Encoder` | 编码器采集与按钮事件 |
| `src/HAL`、`RGB`、`TB6612` | 电源、灯带、电机等硬件操作 |
| `src/Configs` | 板级引脚、任务规格、输入/蓝牙配置与版本；Config.h 为 HAL 兼容入口 |

手柄、小程序、编码器和串口统一提交带来源与时间戳的语义事件；UI 每轮最多消费一个 FIFO 事件。队列固定容量 16，满时拒绝新事件并计数，不覆盖旧事件，不阻塞生产者。蓝牙栈为 Bluepad32/BTstack，不应混用另一蓝牙栈的 BLEDevice 初始化。

[架构重构设计](资料/docs/superpowers/specs/2026-10-10-firmware-architecture-design.md)分阶段实施。输入、蓝牙与App/配置三个阶段已由用户实机验证；第四阶段OLED模块拆分完成代码验证，等待实机回归。[输入验收记录](资料/docs/firmware-input-validation.md)、[蓝牙验收记录](资料/docs/firmware-bluetooth-validation.md)、[App验收记录](资料/docs/firmware-app-validation.md)、[OLED验收记录](资料/docs/firmware-oled-validation.md)。历史文档路径见[资料说明](资料/README.md)。

## 环境与编译

- 原版 ESP32 / `esp32dev`，Arduino，PlatformIO，C++。
- 平台 `espressif32@6.10.0`。
- 自定义 Bluepad32 框架由 `platformio.ini` 的 `platform_packages` 指定；最近本机解析为 4.0.2。源地址引用 main 分支，未锁定提交，其他机器解析版本可能变化。
- 依赖：Adafruit NeoPixel `^1.15.1`、U8g2 `^2.36.12`；最近本机解析为 1.15.5、2.36.18。
- 推荐 VS Code + PlatformIO IDE，优先使用已有环境，不安装重复工具链。

仓库根目录，Windows PowerShell：

```powershell
# 编译
& 'C:\Users\Cainiao\.platformio\penv\Scripts\pio.exe' run -d .\esp32-firmware
# 烧录，COM3 替换为实际端口
& 'C:\Users\Cainiao\.platformio\penv\Scripts\pio.exe' run -d .\esp32-firmware -t upload --upload-port COM3
# 串口
& 'C:\Users\Cainiao\.platformio\penv\Scripts\pio.exe' device monitor -p COM3 -b 115200
```

若环境未初始化，用 VS Code 打开 `esp32-firmware` 并等待 PlatformIO 扩展初始化后重试。`.pio/`、生成的 `.vscode/` 不入库。重命名后重新打开新固件目录。

`test/` 包含二十组独立主机 C++ 测试，覆盖 UI 纯逻辑、输入、连接与应用生命周期、任务参数、绘制记录、弹窗时序、页面分发与真实跨文件页面生命周期。已安装 g++/clang++ 时，在仓库根目录运行 `& .\esp32-firmware\tools\run-host-tests.ps1`。串口输入测试同时编译调试开/关两种配置，并执行迁移脚本安全回归。尚未配置 PlatformIO native 环境；编译及逻辑测试不能替代实机验收。

## 代码中的引脚配置

| 用途 | GPIO |
|---|---|
| OLED SDA / SCL | 21 / 22 |
| 编码器 A / B / 按键 | 14 / 33 / 32 |
| RGB3 灯带 | 19 |
| 电机 IN1 / IN2 / PWM | 16 / 4 / 17 |
| 电池采样 / 充电检测 / 电源保持 | 27 / 25 / 26 |

上述是源码配置，未与此次 PCB 网表逐项核对。接线前核对板型、原理图及实际模块。未接 OLED 时，现有 I²C 初始化/通信可能影响 UI 任务。

## 微信小程序与协议

微信开发者工具导入 `wechat-mini-program`。仓库使用 `touristappid`，真机调试和发布使用自己的 AppID；手机需开启蓝牙。私有配置不上传。详见[小程序 README](wechat-mini-program/README.md)。

- 广播名：`FigureCase`
- Service UUID：`0000FF10-0000-1000-8000-00805F9B34FB`
- Write UUID：`0000FF11-0000-1000-8000-00805F9B34FB`
- 写入单字节：上/左 `0x01/0x03` 为上一项；下/右 `0x02/0x04` 为下一项；确定 `0x05` 为短按确认。

## 硬件资料（2026-10-10）

- [原理图 PDF](资料/hardware/SCH_Schematic1_2_2026-10-10.pdf)
- [PCB PDF](资料/hardware/PCB_PCB2_2026-10-10.pdf)
- [原理图工程](资料/hardware/ProDoc_Schematic1_2_2026-10-10.epro2)
- [PCB 工程](资料/hardware/ProDoc_PCB2_2026-10-10.epro2)

原始内容和文件名保留；`.epro2` 用兼容的嘉立创 EDA 专业版打开。

## 参考项目

[Bluepad32](https://github.com/ricardoquesada/bluepad32)、[WouoUI](https://github.com/RQNG/WouoUI)、[ESP32_connect_XboxController](https://github.com/xiaocainiao11111/ESP32_connect_XboxController)、[super_knob](https://gitee.com/wenzhengclub/super_knob)。保留源代码原作者署名。
