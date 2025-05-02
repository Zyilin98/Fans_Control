// 删除main.cpp中的结构体定义，保留SystemState.h中的唯一定义
// SystemState.h
#pragma once
// SystemState.h
struct SystemState {
    uint8_t active_channel = 0;  // 0:A 1:B
    uint8_t mode_a = 0;          // 0:DC 1:PWM
    float duty_a_dc = 0.0f;      // DC模式占空比（GP5）
    float duty_a_pwm = 0.0f;     // PWM模式占空比（GP7）
    float duty_b = 0.0f;         // B通道占空比（GP1）
    bool screen_mode = false;
    bool active_changed = false;
    bool mode_changed = false;
};
// SystemState.h 末尾添加
extern SystemState sysState; // 全局系统状态实例
