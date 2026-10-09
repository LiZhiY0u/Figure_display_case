# ESP32 展示柜固件架构整理设计

日期：2026-10-10  
状态：待用户审阅  
范围：`2.controller_test`，在 `main` 分阶段实施，不新建分支。

## 1. 目标与边界

提升代码可读性、模块独立性和可测试性，保留已验证的蓝牙连接、输入和 OLED 行为。此次是应用层重构，不更换 ESP32 平台、Bluepad32/BTstack、FreeRTOS、U8g2，不修改框架、HAL 底层算法、ISR 或构建工具链。不增加开发环境或第三方依赖。

不改微信小程序、不补齐灯光/电机业务、不改变菜单内容与视觉效果。`1.oled_test`、`bad_apple` 不在范围内。现有未提交的手柄输入修复作为迁移基线保留；小程序本地 AppID 和私有配置不提交。

## 2. 当前问题及实施原则

- Controller.cpp 同时承担设备管理、输入转换、日志和示例输出命令。
- gamepad_axis、key_en 是共享输入标记，消费和写入缺少清晰的事件边界。
- BleRemote 的初始化和 Update 间接管理手柄，连接事件接口也归属于它。
- OledUI.cpp 集中输入、公共渲染、导航和所有页面；应拆分职责而非机械按行数切割。
- main.cpp 存在未创建的任务、未使用队列和历史注释；Config.h 混有 STM32/LVGL/GPS 等移植残留。

采取渐进模块化：先锁定行为，再提取纯逻辑，最后迁移调用者。不上通用事件总线、插件框架、动态页面注册或多层继承。代码布局可以变化，底层外设操作必须保持等价。发现引脚或业务缺陷时记录并另行确认，不在重构中悄悄修正。

## 3. 模块与依赖

```text
main.cpp → App（初始化与任务编排）
           ├─ Bluetooth → Input / ConnectionEvents
           ├─ Encoder / Serial adapters → Input
           ├─ UI → Input / ConnectionEvents / OLED driver
           └─ Device services → 既有 RGB / Motor / Power drivers
Config、Diagnostics 为基础设施；驱动不得依赖 UI。
```

### App

`App/App.h/.cpp` 负责系统启动和周期入口；`App/AppTasks.h/.cpp` 负责现有任务创建、周期及创建失败报告。main.cpp 只调用 `App::Init()` 和 `App::Update()`。保持现有任务核心、优先级和周期，使用 `pdMS_TO_TICKS` 表达毫秒，检查任务/队列创建结果。启动失败不得继续使用空句柄。

### Bluetooth

`Bluetooth/BluetoothService` 统一一次性的 Bluepad32/BTstack 初始化和更新，避免重复初始化协议栈。

`Bluetooth/GamepadController` 只管理手柄连接、目标地址过滤及报告采集，将报告交给输入逻辑。`Bluetooth/MiniAppRemote` 只负责 FF10/FF11 ATT 服务、广告、命令解码与手机链路识别。手柄不再经 MiniAppRemote 初始化或轮询。

`Bluetooth/ConnectionEvents` 独立提供有界连接通知队列。只有设备生命周期事件产生连接通知，UI 只消费；不再以小程序模块命名手柄通知。

### Input

`Input/InputEvent.h` 定义 `Action {Previous, Next, Confirm}` 和 `Source {Gamepad, MiniApp, Encoder, Serial}`，事件携带动作、来源、毫秒时间戳。`Input/InputQueue` 提供 `Init/Push/Read`，替代跨任务共享输入标记。固定容量 16，非阻塞发送；满队列拒绝新事件、记录溢出计数，不覆盖旧事件、不阻塞蓝牙回调。ISR 不直接调用此接口。

`Input/GamepadInput` 是不依赖 Arduino 的状态机：输入原始按钮/方向/摇杆状态及 `uint32_t now_ms`，输出最多一个动作。`Update` 接收每个新报告；`Tick` 以现有主循环周期检查重复计时，不依赖手柄是否持续发送报告。每个连接拥有独立状态，断开时清理状态，不允许断开后继续连滚。

编码器继续使用现有驱动 API；在应用适配层转换为事件。key_en 如需移除跨任务竞争，可让既有按钮回调调用任务上下文的输入适配器，不改 ISR。涉及 ISR 读清操作的改动超出本设计，必须另外确认。

UI 每轮最多消费一个输入事件，按入队顺序处理。多个来源同时输入的行为从共享变量覆盖改为明确 FIFO；这是本次允许的并发语义改善，正常单来源操作不变。

### UI

保留现有 UiNavigation、MenuItem、MainPageState、AnimationCurve 纯逻辑及测试。拆出 `UiInputAdapter`（事件到既有 BTN_ID 映射）、`UiRenderer`（公共绘制）、`UiPages`（按主页、设置、灯光、工具分组）及 `ConnectionToast.cpp`（弹窗状态和绘制）。OledUI 留下入口和页面分发；UI 状态的所有权明确为 OLED 任务，不向蓝牙模块暴露可变 UI 全局。

必要的共享页面状态集中于显式 UI 上下文，仅在 UI 内部使用；先保留现有页面接口，再逐步收窄头文件，避免一次性改写所有动画。

### Config / Diagnostics / Drivers

配置按 `BoardConfig`、`InputConfig`、`BluetoothConfig`、`TaskConfig` 分开。先检索每个旧宏的引用再移除移植残留，保留所有实际使用引脚。`Diagnostics/GamepadLogger` 只处理限频日志，不参与控制状态机；按键变化即时记录，模拟量至多每 100ms 输出，保留 115200 波特率。

整理驱动命名和头文件依赖，不更改 RGB、电机、电源及编码器底层实现。去掉手柄示例中 A/B/X 自动发灯效/震动命令：这些是遗留示例副作用，不属于展示柜控制需求。

## 4. 必须保留的行为

- 手柄地址为 `C0:D6:D5:EE:DB:8A`；保存配对信息，允许首次配对及后续自动重连。
- 小程序名称 FigureCase，FF10 服务、FF11 写特征不变，单字节 1/3=Previous、2/4=Next、5=Confirm；无效命令不产生事件。
- 手机通知只识别 ESP32 作为 LE peripheral 的链路；手柄连接不得产生小程序弹窗。首次手柄类型延后识别时补发一次连接事件。
- A 键只按下边沿触发，松开不触发，长按不连发；使用位判断，其他按钮同时按住不影响 A 键识别。
- 十字键位为上 0x01、下 0x02、右 0x04、左 0x08，上/左 Previous，下/右 Next；同时含两个相反动作组时不移动。
- 摇杆保留现有触发阈值：任一轴 >500 或 <-500；两轴绝对值均 <=200 回中。保留当前正方向和负方向对应的 UI 动作，不能通过命名重构悄悄反转。以当前 UiInputAdapter 的 BTN_ID 映射为回归基准。
- 摇杆/十字键首次立即移动，初始延迟 400ms，重复间隔 150ms；方向变化重新计时。十字键优先于摇杆，二者共享一次滚动输出，不叠加速度；Confirm 优先于本轮方向事件。
- 回中/松开后不再生成重复事件；已有排队事件正常消费。不补发积压的重复次数，不在一个 Tick 内连续生成多项。时间比较使用无符号差值，正确处理 millis 回绕。
- OLED 128×64，连接弹窗右侧 300ms 滑入、停留 2000ms、300ms 滑出，图标和信息保留；通知逐条显示。
- 菜单、动画曲线、串口 1/2/3 调试指令与编码器操作不变。

## 5. 分阶段迁移

1. 记录当前基线并补输入状态机回归测试，保留已有手柄修复。
2. 提取 GamepadInput、日志和统一输入队列，迁移手柄、小程序及 UI 输入消费。
3. 分离 BluetoothService、GamepadController、MiniAppRemote 和 ConnectionEvents。
4. 整理 App 入口、任务编排与配置，清除确认无引用的历史代码。
5. 拆分 OLED 输入、公共绘制、页面和弹窗；完善模块说明及调试文档。

每阶段独立验证和提交，始终在 main 上操作。不能顺带提交微信本地配置；只有用户要求时推送。若阶段需要变更 ISR、HAL、任务核心/优先级或协议栈配置，暂停并另行确认。

## 6. 验收与风险

自动化：运行已有 6 组主机测试，新增输入测试覆盖 A 键按住/组合键、方向单次/长按、400/150ms 边界、无新报告仍连滚、摇杆阈值抖动、冲突方向、双来源优先级、松开/断开、计时回绕。队列测试覆盖 FIFO、容量及溢出策略；连接分类与通知去重尽量提取纯逻辑测试。

构建：使用现有 `C:\Users\Cainiao\.platformio\penv\Scripts\pio.exe` 编译 esp32dev；不用临时安装环境。记录各阶段 RAM/Flash，若 RAM 或 Flash 相比迁移基线增长超过 5%，说明原因并征求确认。队列与状态采用固定容量，不在持续输入处理路径动态分配。

实机：冷启动首连、断开重连、手柄/手机同时连接、A 短按/长按、四向短按/长按、摇杆回中、所有菜单与设置页面、睡眠/唤醒、右侧弹窗及灯带原有行为逐项检查。编译和主机测试不能替代实机确认。

风险：任务输入 FIFO 会改变极端并发时的顺序；页面拆分可能改变静态状态生命周期；协议栈初始化顺序可能影响首连。通过小步迁移、明确状态所有权和阶段回归控制风险。若实机出现差异，只撤销本阶段变更或向前修正，不使用破坏性 Git 重置，不覆盖用户工作。

交付：模块化固件、自动化测试、架构/输入/调试说明、阶段编译和资源记录，以及明确标注仍需实机验收的项目。
