# 固件架构优化第一阶段实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans or superpowers:subagent-driven-development to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 提取可测试的手柄状态机，统一输入事件通道，消除手柄输入共享标记覆盖问题。

**Architecture:** 输入算法独立于 Arduino/Bluepad32，硬件适配器只提交报告。统一的有界 FreeRTOS 队列传递语义事件，UI 保持单任务消费和原有按键映射。蓝牙服务拆分、App 层和 UI 大文件拆分属于后续独立阶段，不混入本阶段。

**Tech Stack:** 现有 ESP32 Arduino、Bluepad32/BTstack、FreeRTOS、U8g2、C++11；主机 assert 测试。

**Spec:** `资料/docs/superpowers/specs/2026-10-10-firmware-architecture-design.md`

## Global Constraints

- 在 main 上执行，不新建分支，不推送除非用户要求。
- 不修改小程序本地配置、HAL 底层算法、ISR、任务核心或优先级、BLE 协议。
- 固定容量 16 的输入队列，满时拒绝新事件并计数；非阻塞，持续输入路径不动态分配。
- A 键边沿触发；方向初次立即触发，400ms 首次重复、150ms 后续重复。
- 摇杆阈值 >500/<-500；双轴绝对值 <=200 回中。正方向为 Previous，负方向为 Next（保留现有 axis2/axis1 到 UI 的映射）。
- 十字键上/左 Previous，下/右 Next；冲突方向不移动；十字键优先、Confirm 优先。
- 延迟 Tick 不补发积压次数；时间回绕正确；断开清理状态。
- UI 每轮最多一个 FIFO 事件；现有菜单、动画和连接通知不变。
- 基线 ad6bf9f，RAM 98432 bytes，Flash 791769 bytes；增长超过 5% 需说明并确认。

## Review Focus

1. 无新手柄报告仍能长按连滚，但断开后立即停止（任务 1/3）。
2. 日志限频不能影响输入边沿，A+其他键仍正常确认（任务 1/3）。
3. 队列满不会覆盖确认或阻塞蓝牙，错误可观测（任务 2）。
4. 摇杆和十字键同时操作不会叠加滚动，改变方向重新延迟（任务 1）。
5. 编码器 ISR 不得调用普通队列；串口与手机按原语义工作（任务 3）。

## 接口及文件布局

- `Input/InputEvent.h`：`enum class Action:uint8_t {None,Previous,Next,Confirm}`；`Source {Gamepad,MiniApp,Encoder,Serial}`；`struct Event {Action action; Source source; uint32_t time_ms;}`。
- `Input/GamepadInput.h/.cpp`：`Report {uint16_t buttons; uint8_t dpad; int32_t x,y;}`；`class State {Action Update(const Report&,uint32_t); Action Tick(uint32_t); void Reset();}`。Update 处理新报告和首次/边沿输出，Tick 处理重复；同一轮 Confirm 抑制方向输出。
- `Input/InputQueue.h/.cpp`：`bool Init(); bool Push(const Event&); bool Read(Event&); uint32_t DroppedCount();`。队列只允许任务/蓝牙任务上下文调用。
- `Diagnostics/GamepadLogger.h/.cpp`：`void Reset(uint8_t slot); void Log(uint8_t slot,const Report&,uint32_t);`，只读取报告，默认关闭；其他模拟量由日志专用快照承载，不扩大输入状态机职责。
- `Configs/InputConfig.h`：触发/回中阈值及重复计时常量；此阶段不整理其他硬件配置。
- 修改 `Controller.cpp/.h`、`BleRemote.cpp/.h`、`OledUI.cpp`、`HAL_Encoder.cpp/.h`、`main.cpp/.h`，仅迁移输入通道。OledUI 输入映射 Previous→BTN_ID_CC，Next→BTN_ID_CW，Confirm→BTN_ID_SP。

## Task 1：锁定输入行为并提取状态机

**Files:** 新增 `Input/InputEvent.h`、`Input/GamepadInput.h/.cpp`、`Configs/InputConfig.h`、`test/gamepad_input/test_gamepad_input.cpp`、`tools/run-host-tests.ps1`。

**Interfaces:** 产生上述 Action、Report、State，供任务 3 使用。

- [ ] 检测现有主机 C++ 编译器；只使用已安装工具。无可用主机编译器时暂停测试门，不安装新工具、不声称通过。
- [ ] 添加测试和可复用运行脚本。以仓库根为工作目录运行 `& .\esp32-firmware\tools\run-host-tests.ps1`，脚本寻找已装 g++/clang++，用 C++11、src include 编译每个 test/*/test_*.cpp；涉及 GamepadInput 的测试链接对应 cpp。输出每组 PASS，任一失败非零退出，临时产物留在被忽略构建目录。
- [ ] 运行新增测试，确认因 State 尚不存在编译失败；记录旧六组测试基线。
- [ ] 测试断言：A 的 down/down/up/down 输出 Confirm/None/None/Confirm；buttons=0x0003 仍识别 A；dpad 0x01/0x08→Previous、0x02/0x04→Next、0x03→None。
- [ ] 测试从 t=100 按住方向：Update(100) 输出一次；Tick(499)=None、Tick(500)=方向、Tick(649)=None、Tick(650)=方向。迟到 Tick(2000) 只输出一次，下一次最早 2150。
- [ ] 测试摇杆 x=501→Previous、y=-501→Next；±500 不首次触发；维持 450 保持方向；双轴 <=200 后停止；方向变化重置 400ms。
- [ ] 测试十字键优先、Confirm 与方向同时变化不双输出、Reset 后 Tick 无输出；以接近 UINT32_MAX 的起点重复上述时序验证回绕。
- [ ] 实现 State，运行全部主机测试通过；提交本任务文件。

## Task 2：统一非阻塞事件队列

**Files:** 新增 `Input/InputQueue.h/.cpp`、`test/input_queue/test_input_queue.cpp` 及 `test/support/` FreeRTOS 队列测试替身。

**Interfaces:** 消费任务 1 Event；产生 Init/Push/Read/DroppedCount。

- [ ] 用测试替身编译真实 InputQueue.cpp，断言 Init 失败时 Push/Read 返回 false；禁止测试只覆盖另一套未集成的队列实现。
- [ ] 断言连续 16 次 Push 成功，第 17 次失败且 dropped+1；Read 保持动作/来源/时间 FIFO，不覆盖已有事件；空读 false。断言每次发送/接收等待参数为 0。
- [ ] 先运行测试确认失败，再实现固定容量 FreeRTOS 队列。溢出计数使用适合当前环境的原子计数，跨任务读取无数据竞争；Init 在产生输入前完成一次。
- [ ] 运行全部主机测试及 `& 'C:\Users\Cainiao\.platformio\penv\Scripts\pio.exe' run -d .\esp32-firmware`，确认 SUCCESS；提交。

## Task 3：迁移生产者与 UI 消费者

**Files:** 修改 Controller、BleRemote、OledUI、main、Encoder 文件；新增 GamepadLogger 及适配器测试 `test/input_adapter/test_input_adapter.cpp`。

**Interfaces:** 消费任务 1 State、任务 2 InputQueue；不改变蓝牙初始化职责，本阶段 BleRemote 仍管理蓝牙启动。

- [ ] 添加映射断言 Previous→BTN_ID_CC、Next→BTN_ID_CW、Confirm→BTN_ID_SP；手机 1/3、2/4、5 和串口 1/2/3 保持原动作，编码器正负与既有方向等价。
- [ ] 每个手柄槽保存 State；新数据 Update 后 Push，主循环无新数据仍 Tick；连接/断开 Reset。清除旧 axis、a_down、scroll_direction、repeat 数组，只保留单一状态来源。
- [ ] 提取日志，默认关闭、数字变化即时、模拟量至多每 100ms；不使用日志快照判断控制边沿。删除 A/B/X 示例灯效和震动副作用。
- [ ] BleRemote::OnWrite 解码后直接 Push 来源 MiniApp；保留 ATT/广告/连接事件行为。移除重复输入队列，不动连接队列。
- [ ] 编码器按钮回调在既有任务上下文 Push Confirm；UI 原扫描位置读取 Encoder_GetDiff 并转成事件，串口同样在原扫描位置转事件；不修改 IRQHandler 或底层脉冲读取算法。
- [ ] UI 每轮 Read 一个 Event 并设置既有 btn；删除 gamepad_axis/key_en 及其 extern。main 启动时 InputQueue::Init 先于任何生产者，失败时报告并禁止继续启动输入消费者。
- [ ] 全测试和固件编译通过，检查仍存在的历史输入变量引用为零；记录 RAM/Flash。提交迁移文件，不提交微信配置。

## Task 4：阶段交付与实机验收

**Files:** 更新根 README、新增 `资料/docs/firmware-input-validation.md`。

- [ ] 记录实际命令、测试结果、资源差异、队列溢出观察方式和剩余架构工作；不将计划写成已完成。
- [ ] 用户实机检查冷启动连接、断开重连、A 单击/长按/组合键、十字键/摇杆短按与长按、回中、手机/编码器/串口、页面导航和弹窗。
- [ ] 标注未实测项，不以编译代替验收；存在回归暂停后续蓝牙拆分。
- [ ] 提交文档，给出第一阶段完成边界与下一阶段建议。只有用户要求时推送。

## 后续阶段（另行计划）

第一阶段验收后，按原设计依次安排蓝牙职责与连接事件独立、App/任务/配置整理、UI 公共绘制与页面拆分。每阶段先具体计划再实施，避免所有模块同时变动导致问题无法定位。

## 自审

已覆盖原设计第一阶段输入要求及五项高风险条件。其余蓝牙/App/UI 架构变更明确留给后续阶段；不改变构建平台与硬件算法。队列测试必须编译生产实现，状态机测试必须覆盖精确边界、回绕和无报告计时。主机工具不可用即报告阻塞，不安装依赖。当前尚未实施。
