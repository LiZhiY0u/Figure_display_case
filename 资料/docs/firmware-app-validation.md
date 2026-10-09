# App 与配置整理验证记录

日期：2026-10-10；基线633b8de。第二阶段用户反馈“测试无误”，第三阶段用户反馈“测试通过”；未逐项提供日志。

main → App → HAL / InputQueue / BluetoothService / AppTasks。main仅保留Arduino入口与10ms周期；App只在启动成功后轮询蓝牙，成功/失败初始化均幂等，失败后需重启，不隐式重试部分初始化。

AppTasks按原顺序创建采样、灯带、编码器、OLED任务，核心均1，优先级2/4/4/4，栈4096/2048/2048/10240。采样1000ms、编码器10ms、OLED1ms。pdMS_TO_TICKS替代裸tick，目标编译静态断言确认1000Hz。失败时打印任务名、停止后续创建、删除此前已创建任务。任务可能已运行，删除不回滚GPIO/灯带/编码器中断等硬件操作；此策略不是完整安全停机。

BoardConfig集中实际引脚；Config.h仅HAL兼容入口。GPS/STM32/LVGL等无引用配置已移除。未启用电机或HAL周期任务。Motor静态对象构造会配置GPIO/PWM，因此保留；RGB3保留现有全局链接，灯带演示不变。无消费者adc删除，但周期analogRead仍保留。编码器显式包含自己的声明，ISR函数体不改。

## 自动化证据

`& .\esp32-firmware\tools\run-host-tests.ps1`：14组通过。新增测试先观察缺失实现失败，随后通过真实App.cpp/AppTasks.cpp验证编排；外设/FreeRTOS边界替身不代表真实多核验证。

`& 'C:\Users\Cainiao\.platformio\penv\Scripts\pio.exe' run -d .\esp32-firmware`：esp32dev SUCCESS。

RAM98448字节、Flash789337字节，相对基线+8/+304，均小于5%。未烧录、未推送；微信本地AppID不提交。

独立只读审查无Critical/Important问题。一个轻微项暂缓：ButtonEvent.h重复包含Arduino.h，不影响行为。Init/Start仅适用于当前setup启动调用，未增加并发调用保障；实际任务栈余量仍需实机确认。

## 实机清单

- [ ] 冷启动UI与原灯带演示正常。
- [ ] 编码器、手柄A键、四向及摇杆长按正常。
- [ ] 手柄与手机首连/重连/共同连接及通知正常。
- [ ] 全部菜单、设置、睡眠和唤醒正常。

通过后继续OLED模块拆分。
