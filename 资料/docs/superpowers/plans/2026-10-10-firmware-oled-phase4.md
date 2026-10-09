# OLED 模块拆分第四阶段实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 拆分 OLED 输入、公共绘制、页面和弹窗，让入口只负责初始化及状态分发。

**Architecture:** 保留现有函数签名以控制迁移风险，内部状态声明移入 UiContext.h，定义保持单一。按职责提取 UiInput.cpp、UiRenderer.cpp、UiPages/*.cpp 和 ConnectionToast.cpp；不引入页面继承或动态注册。

**Tech Stack:** 现有 U8g2、ESP32 Arduino、C++11、PlatformIO、MinGW。

**Spec:** `资料/docs/superpowers/specs/2026-10-10-firmware-architecture-design.md`

## Global Constraints

- 基线 caab895，第三阶段用户反馈实机测试通过；main 本地提交、不推送、不提交微信 AppID。
- 不改任务、驱动、ISR、蓝牙、菜单、字体、图标、灯光占位业务、输入算法与动画曲线。
- OLED128×64；输入每轮最多消费一个FIFO事件；已有btn.pressed不能覆盖。
- 帧顺序保持Key_Scan→SendBuffer→状态/页面处理→连接弹窗；不在重构时“修正”先发送上一帧的现有行为。
- 弹窗300ms滑入、2000ms停留、300ms滑出，按无符号时间差处理回绕，一条显示完才取下一条。
- RAM98448、Flash789337为基线；增长超过5%需说明并确认。无新增运行时动态分配、环境或依赖。

## Review Focus

1. 待处理输入遇到页面过渡不得被新事件覆盖：任务1测试。
2. 弹窗时间边界0/299/300/2299/2300/2599/2600及millis回绕：任务1测试。
3. 连续通知不能覆盖当前弹窗；结束后下一条立即开始：任务1测试。
4. 页面切换与静态状态不能因多编译单元产生副本或复位：任务3检查及主机状态回归。
5. 公共绘制与弹窗必须恢复原绘图状态，帧调用顺序不能变化：任务2记录式绘制测试、任务3分发测试。

## Task 1：输入及连接弹窗

**Files:** 新建 `src/OledUI/UiInput.cpp`、`ConnectionToast.cpp`、`ToastTimeline.h`、`test/ui_input/test_ui_input.cpp`、`test/toast_timeline/test_toast_timeline.cpp`；修改ConnectionToast.h与主机测试脚本。

**Interfaces:** 保留`void OledUI::Key_Scan()`、`void ConnectionToast::Draw()`。纯`ToastTimeline::State`提供`bool Active(uint32_t now) const; void Start(uint32_t now); int X(uint32_t now) const;`，内部started与active默认零；X保持原二次曲线公式，不使用浮点。

- [ ] 先测试真实UiInput.cpp：编码器事件提交后按FIFO消费；btn.pressed=true不读取队列；None无动作；串口调试开/关两个构建，无效字节跳过、每轮最多一条有效命令。替身只覆盖编码器/串口/队列边界。
- [ ] 先测试时间状态：未启动不active；Start(100)后100/399/400/2399/2400/2699 active，2700 inactive；x分别128/18/18/18/18/127；回绕Start(0xfffffff0)后300ms仍在停留，2600ms结束。观察RED再实现。
- [ ] 将Key_Scan原逻辑迁移，弹窗绘制移到cpp、状态仅一份；绘制字节、标题、布局保持原样。测试队列连续两事件在2600ms才取第二条，绘制保存/恢复font及drawColor；链接生产ConnectionToast.cpp与记录式U8g2替身。
- [ ] 全主机测试、固件构建通过，记录资源并提交。

## Task 2：公共绘制

**Files:** 新建`src/OledUI/UiRenderer.cpp`、`test/ui_renderer/test_ui_renderer.cpp`及绘制替身。

**Interfaces:** 保留OledUI::Animation、SetAnimationCurve、GetAnimationCurve、Fade、Tile_Show、List_Show、Volt_Show、Window_Show现有签名。List_Draw_*私有辅助和其调用者一起移动，不暴露新公共API。

- [ ] 对基线绘制建立记录式测试：Tile_Show初始/已初始化、List_Show文本/值/复选框、Window_Show上下限，记录绘制调用与关键坐标；Fade测试缓冲区逐步变化。期望从基线手工核对，不由生产实现生成。
- [ ] 新模块链接缺失RED后迁移函数原体；保持ui.param、AnimationTrack引用、字体设置、绘制顺序和取样行为。不得顺带修改Volt模拟引脚或越界等历史问题，发现后另行记录。
- [ ] 全测试、构建、资源检查通过并提交。

## Task 3：页面与状态所有权

**Files:** 新建`src/OledUI/UiContext.h/.cpp`、`UiPages/UiPages.h`、`HomePages.cpp`、`ToolPages.cpp`、`SettingPages.cpp`、`LightPages.cpp`、`PageNavigation.cpp`，修改OledUI.h/.cpp；新增`test/ui_dispatch/test_ui_dispatch.cpp`。

**Interfaces:** 内部`void UiPages::InitPage(UI_PAGE page); void UiPages::Process(UI_PAGE page); void UiPages::RotateTile(); void UiPages::RotateList();`。保留OledUI::Init/Task及导航公共接口；上下文仅OLED模块依赖，rg核对外部消费者后收窄头文件。

- [ ] 写分发测试：全部现有UI_PAGE映射到唯一原处理函数，未知页无动作；S_FADE/S_WINDOW/S_LAYER_IN/S_LAYER_OUT不得误执行普通页；Task调用顺序及弹窗调用位置保持基线。RED后实现生产分发。
- [ ] UiContext集中原ui/tile/list/volt/check_box/win/knob/btn类型及单一定义，初始值与存储期不变；analog_pin移至ToolPages私有区域（确认无外部消费者后）。
- [ ] HomePages迁移Sleep/Main/Editor，ToolPages迁移Knob/Krf/Kpf/Volt，SettingPages迁移Setting/AnimationCurve/About，LightPages迁移全部Light初始化/处理/显示及其私有状态。灯光参数若有跨文件消费者保留原外部链接，否则收窄为模块内。
- [ ] PageNavigation集中导航、层切换、页面初始化和Tile/List旋转；复选框及Window初始化/处理共用代码留在该模块。先保持函数体原样，不改变页面初始化时机或选中位置。
- [ ] OledUI.cpp仅系统参数初始化、Init、Task与状态分发；OledUI.h保留外部实际使用接口，内部声明移入UiContext/UiPages。检索确认非OLED模块不依赖可变UI上下文。
- [ ] 全测试、固件构建、独立只读审查、处理阻断问题并提交；替身测试不能宣称真实OLED像素验收。

## Task 4：交接

- [ ] 更新README与`资料/docs/firmware-oled-validation.md`，记录模块、命令、资源、审查、自动化覆盖及未覆盖项，记录第三阶段用户验收通过。
- [ ] 实机逐页检查Light/Editor/Volt/Setting、参数窗口、动画曲线、睡眠唤醒、各种输入、连续连接通知与弹窗观感。
- [ ] 未完成实机验收不得宣称整体架构优化完全验收；本地提交不主动推送。

自审：接口保持兼容、私有辅助随调用者迁移；先建立可观察行为测试再移动实现。页面所有权与绘制顺序是核心风险，资源阈值沿用原设计。执行方式沿用当前会话直接实现、末尾独立审查。状态：用户批准并实施，任务1–3代码完成、任务4文档完成、实机待验收。20组主机测试与迁移脚本安全回归通过，目标构建及独立审查完成；Active到期锁存采用非const查询，analog_pin无实际消费者故删除。
