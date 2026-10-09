# App 入口与配置整理第三阶段实施计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将启动、任务编排与硬件配置从 main 中分离，保持已验收功能。

**Architecture:** main 只调用 App::Init/Update。App 编排 HAL、输入、蓝牙与任务；AppTasks 管理原有四个任务。BoardConfig 和 TaskConfig 集中实际配置，不改驱动算法。

**Tech Stack:** 现有 ESP32 Arduino、FreeRTOS、PlatformIO、MinGW 主机测试。

**Spec:** `资料/docs/superpowers/specs/2026-10-10-firmware-architecture-design.md`

## Global Constraints

- 基线 633b8de，第二阶段由用户反馈实机通过；只在 main 本地提交，不推送、不提交微信 AppID。
- 不改 ISR、HAL 算法、协议栈、菜单和设备业务，不增加环境或依赖。
- 四任务创建顺序保持采样→灯带→编码器→OLED；核心全部 1，优先级 2/4/4/4，栈 4096/2048/2048/10240。
- 延时原值 1000/10/1ms 和主循环 10ms，使用 pdMS_TO_TICKS；先核对实际 configTICK_RATE_HZ，非 1000Hz 则暂停说明，不偷偷改变周期。
- RAM98440、Flash789033 为基线，增长超过5%需说明并确认。

## Review Focus

1. 任意任务创建失败：停止后续创建，删除已创建任务，不标记 App ready。
2. 重复成功 Init：不重复 HAL、蓝牙或任务初始化。
3. 队列或蓝牙失败：不创建任务、不更新蓝牙；失败启动不自动重试部分初始化。
4. 配置迁移不得改变引脚或通过传递包含破坏 RGB/编码器/UI。
5. 未启用的电机/HAL周期任务不能在整理时被意外启用。

## Task 1：App 生命周期

**Files:** 新建 `src/App/App.h/.cpp`、`test/app_lifecycle/test_app_lifecycle.cpp`，扩展主机测试脚本。

**Interfaces:** `bool App::Init(); void App::Update(); bool AppTasks::Start();`。App 状态为 NotStarted/Failed/Ready；Init 失败后返回 false，不自动重试。

- [ ] 写测试链接真实 App.cpp，替身只模拟 HAL/InputQueue/BluetoothService/AppTasks：首次成功调用顺序 HAL→input→bluetooth→tasks；任意失败后不调用下一项；Update仅Ready调用蓝牙；重复Init与失败后再次Init无副作用。
- [ ] 运行确认缺失实现失败，再实现生命周期；保留现有初始化日志与失败诊断。
- [ ] 测试通过并提交。

## Task 2：任务与配置迁移

**Files:** 新建 `src/App/AppTasks.h/.cpp`、`src/Configs/BoardConfig.h`、`TaskConfig.h`、`test/app_tasks/test_app_tasks.cpp`；修改 main.cpp、main.h、Configs/Config.h 及其实际消费者。

**Interfaces:** AppTasks::Start 返回全部四任务创建成功；任务句柄仅模块内持有。BoardConfig集中现有使用引脚，TaskConfig定义上述任务规格与延时；保留必要宏兼容驱动，不改ISR函数体。

- [ ] 用FreeRTOS/设备替身链接真实AppTasks.cpp，测试四任务名称/顺序/栈/优先级/核心，逐个注入创建失败，断言不继续创建且清理此前句柄；重复成功Start不重建。
- [ ] RED后迁移四任务原函数体；保留RGB3全局链接供现有RGB代码使用，不改变灯带对象构造和演示。创建失败报告具体任务名，清理已启动任务；不宣称回滚已发生硬件动作。
- [ ] 检索每个旧宏与全局引用；移走实际引脚，删除确认无引用的GPS/STM32/LVGL残留、未创建任务、无消费者adc和无用队列；保留采样任务周期及analogRead操作。
- [ ] main.cpp仅Arduino setup/loop薄入口，loop保持10ms延时；消费者改为明确配置或驱动头文件，main.h仅必要兼容或无消费者时删除。
- [ ] 测试配置数值与旧值一致，全部主机测试及esp32dev构建通过；rg确认旧任务/队列无引用；独立只读审查并处理阻断问题，提交。

## Task 3：验证交接

- [ ] 更新README和第三阶段验收文档：依赖、资源、命令、失败策略、尚未覆盖真实多核与设备动作的边界；记录第二阶段用户实机通过。
- [ ] 实机检查冷启动、手柄/手机重连及共同连接、输入长按、全部菜单、睡眠唤醒和原灯带行为。
- [ ] 自动化结果与实机验收分开，实机通过后进入OLED拆分。

自审：沿用已批准App设计，不扩展业务；任务失败清理是显式错误路径，不改变正常路径调度。实际GPIO已逐引用核对，硬件网表审核仍不在本阶段范围。状态：任务1–2代码与自动化验证完成，任务3文档完成、实机待验收。14组主机测试及esp32dev构建通过，独立审查无阻断问题。继续采用当前会话直接执行、末尾独立审查。
