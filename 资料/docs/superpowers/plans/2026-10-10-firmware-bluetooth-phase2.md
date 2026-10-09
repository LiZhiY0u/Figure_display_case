# 蓝牙职责分离第二阶段实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans or superpowers:subagent-driven-development to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将手柄、小程序 ATT 服务、连接通知队列与蓝牙生命周期编排分离。

**Architecture:** BluetoothService 是唯一启动/更新入口，先初始化连接队列，再初始化 Bluepad32 手柄，最后启动 ATT 与广告。GamepadController 采集并提交输入，MiniAppRemote 处理手机服务，两者只通过独立 ConnectionEvents 通知 UI，不互相初始化。

**Tech Stack:** 现有 ESP32 Arduino、Bluepad32/BTstack、FreeRTOS、C++11、MinGW 主机测试。

**Spec:** `资料/docs/superpowers/specs/2026-10-10-firmware-architecture-design.md`

## Global Constraints

- main 上执行，不新建分支、不推送、不改微信配置；第一阶段已由用户反馈实机通过。
- 不修改协议栈、框架、ISR、任务核心/优先级、输入状态机或菜单行为。
- 保留配对信息、目标地址 C0:D6:D5:EE:DB:8A、广播 FigureCase、FF10/FF11、1–5 输入命令。
- 保持现有初始化顺序和一次 BP32.update；禁止重复 BP32.setup/att_server_init。
- 连接队列容量 8、非阻塞、满时拒绝新事件并计数，不改输入队列容量 16。
- 冷启动首次手柄延后识别只通知一次；手机只识别 LE peripheral 链路，按句柄对应断开。
- 弹窗图标和 300/2000/300ms 时序保持不变。
- 基线第一阶段 RAM 98432、Flash 788757；增长超过 5% 需说明并确认。

## Review Focus

1. 第一条连接发生在消费者启动前也不丢初始化通知（任务 1/3）。
2. 手柄 ATT 事件不得误触发 Mini App 弹窗（任务 2）。
3. 不匹配句柄的断开不能清空手机连接或重启其广告（任务 2）。
4. 重复 Init 不重复注册回调或重置现有事件；队列失败不轮询未初始化栈（任务 1/3）。
5. 蓝牙每轮只更新一次，手柄长按在没有新报告时仍能重复（任务 3）。

## 文件与接口

所有生产文件位于 `esp32-firmware/src/`。

- `Bluetooth/ConnectionEvents.h/.cpp`：`enum class Event:uint8_t {GamepadConnected,GamepadDisconnected,MiniAppConnected,MiniAppDisconnected}`；`bool Init(); bool Push(Event); bool Read(Event&); uint32_t DroppedCount();`。
- `Bluetooth/MiniAppConnection.h`：纯 `class State {bool Connect(uint16_t handle,bool is_le,bool peripheral); bool Disconnect(uint16_t handle);}`；无句柄值使用 0xffff；重复/无效连接拒绝，只有已记录句柄可断开。
- `Bluetooth/MiniAppRemote.h/.cpp`：`void Init();`，只注册 ATT、写回调并启动广告；使用 ConnectionEvents 和 InputQueue。
- `Bluetooth/GamepadController.h/.cpp`：`void Init(); void Update();`，迁移 Controller 原逻辑；Update 内保留 BP32.update，BluetoothService 不重复调用。
- `Bluetooth/BluetoothService.h/.cpp`：`bool Init(); void Update();`，初始化幂等，成功后才能轮询。
- `Configs/BluetoothConfig.h`：目标地址、广播字节及 BLE UUID/特征句柄关联说明。`BleRemoteProfile.h` 移到 `Bluetooth/MiniAppProfile.h`，ATT 二进制内容不改。
- 修改 main.cpp 和 ConnectionToast.h，只改入口与事件类型；删除旧 Controller、BleRemote 接口及不再使用的包含。

## Task 1：独立连接事件队列

**Files:** 新增 ConnectionEvents、`test/connection_events/test_connection_events.cpp`；扩展 `tools/run-host-tests.ps1` 链接生产 ConnectionEvents.cpp 与现有 FreeRTOS 替身。

- [ ] 先写测试：Init 失败后读写 false；初始化成功后前 8 次 Push true，第 9 次 false 且 DroppedCount+1；FIFO 保留事件顺序；重复 Init 不清队列；空读 false，等待参数为 0。
- [ ] 运行测试确认缺失实现失败，随后实现固定容量队列与原子计数。
- [ ] 全主机测试和 esp32dev 构建通过，记录资源，提交本任务。

## Task 2：手机连接分类纯逻辑

**Files:** 新增 MiniAppConnection.h、`test/mini_app_connection/test_mini_app_connection.cpp`。

- [ ] 测试经典蓝牙/LE central 不通知；LE peripheral 句柄 42 首次 true，重复 false；另一个句柄 false；Disconnect(99)=false、Disconnect(42)=true、再次 false；连接新句柄可再次 true；invalid=0xffff 无效。
- [ ] 先运行红阶段，再实现 State；所有测试通过，提交。
- [ ] 迁移时 BTstack 的连接类型与角色转换成上述布尔参数，不允许未经分类直接通知。

## Task 3：迁移生命周期与消费者

**Files:** 新建/迁移 Bluetooth 文件与配置，修改 main.cpp、ConnectionToast.h；新增 `test/bluetooth_service/test_bluetooth_service.cpp` 和服务依赖替身。

- [ ] 针对真实 BluetoothService.cpp 写启动测试：队列失败 Init=false，不调用手柄/ATT；成功顺序 queue→gamepad→miniapp；重复 Init 不重复调用；未启动 Update 无动作，启动后每轮只调用一次 GamepadController::Update。先观察测试失败。
- [ ] 移动原 Controller 实现并命名空间化；地址移入配置，输入 State、Tick、日志、通知去重不变；只改通知到 ConnectionEvents。
- [ ] 将 BleRemote ATT 与广告部分移入 MiniAppRemote，使用 MiniAppConnection::State；断开只有匹配手机句柄时通知及重开广告。服务写回调仍直接提交 UserInput 事件。
- [ ] BluetoothService::Init 按指定顺序初始化并在成功后设置 ready；main 先 InputQueue::Init，再 BluetoothService::Init，两者成功后创建原任务；loop 调用服务入口。
- [ ] ConnectionToast 改为读取 ConnectionEvents，画面和动画不改。删除过渡旧接口，rg 检查旧引用为零。
- [ ] 运行全部主机测试及固件编译，独立只读审查，处理阻断问题，记录资源；提交迁移，不上传微信配置。

## Task 4：文档与实机交接

**Files:** 更新 README，新增 `资料/docs/firmware-bluetooth-validation.md`。

- [ ] 记录新模块依赖、验证命令/结果、资源、队列溢出观察方式及保留的外部边界。
- [ ] 用户实机测试冷启动首次连接、手柄/手机分别断开重连、双设备同时连接、手柄不误弹手机通知、输入长按与连接弹窗。
- [ ] 自动化完成与硬件未验收分开报告；用户反馈通过后才进入 App/配置整理阶段。只本地提交，不主动推送。

## 自审与交接

输入算法保持第一阶段原样，本阶段测试集中于队列、连接归属与生命周期。服务测试链接真实编排实现，替身只模拟库边界，不能代替蓝牙射频实测。初始化配置失败需要失败结果，不能用无条件 ready 掩盖；所有迁移后连接通知仍由设备事件触发。实施方式延续第一阶段：当前会话逐项执行并在末尾独立审查。

状态：用户已批准实施；任务 1–3 代码与自动化验证完成，任务 4 文档完成、实机回归待用户验收。各任务合并为一个本地迁移提交，避免暂存重命名造成中间提交接口不完整；不推送。
