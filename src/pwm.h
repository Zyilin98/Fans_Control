#ifndef PWM_H
#define PWM_H

#include <Arduino.h>

class PWM {
public:
    // 前置声明通道配置结构体
    struct ChannelConfig;

    static void init();
    static void setDutyCycle(uint8_t channel, float dutyPercent);
    static void enableChannel(uint8_t channel);
    static void disableChannel(uint8_t channel);

private:
    static const uint16_t PWM_FREQ = 20000;   // 20kHz固定频率
    static const uint8_t PWM_RESOLUTION = 10; // 10位分辨率
    static const uint8_t TIMER_NUM = 0;       // 统一使用定时器0

    // 通道配置数组声明
    static const ChannelConfig CHANNEL_CONFIG[3];

    static float dutyCycles[3];  // 各通道占空比存储
    static bool enabled[3];      // 通道启用状态
};

// 通道配置结构体定义
struct PWM::ChannelConfig {
    uint8_t channel;
    uint8_t pin;
};

#endif
