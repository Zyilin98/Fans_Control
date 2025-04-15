#include "pwm.h"
#include <Arduino.h>

// 通道配置：LEDC通道号 + GPIO引脚
const PWM::ChannelConfig PWM::CHANNEL_CONFIG[3] = {
    {0, 1},  // 通道0使用LEDC通道0，GPIO1
    {1, 5},  // 通道1使用LEDC通道1，GPIO5
    {2, 7}   // 通道2使用LEDC通道2，GPIO7
};

float PWM::dutyCycles[3] = {0};
bool PWM::enabled[3] = {false};

void PWM::init() {
    // 配置PWM定时器参数
    ledcSetup(TIMER_NUM, PWM_FREQ, PWM_RESOLUTION);

    // 初始化所有通道
    for(uint8_t i=0; i<3; i++) {
        const auto& config = CHANNEL_CONFIG[i];
        ledcAttachPin(config.pin, config.channel);
        ledcWrite(config.channel, 0);  // 初始禁用输出
    }
}

void PWM::setDutyCycle(uint8_t channel, float dutyPercent) {
    if(channel >= 3) return;

    // 参数约束
    dutyPercent = constrain(dutyPercent, 0.0f, 100.0f);
    dutyCycles[channel] = dutyPercent;

    // 立即生效（如果通道已启用）
    if(enabled[channel]) {
        uint32_t duty = static_cast<uint32_t>((dutyPercent / 100.0f) * (1 << PWM_RESOLUTION));
        ledcWrite(CHANNEL_CONFIG[channel].channel, duty);
    }
}

void PWM::enableChannel(uint8_t channel) {
    if(channel >= 3) return;

    // 应用保存的占空比设置
    uint32_t duty = static_cast<uint32_t>((dutyCycles[channel] / 100.0f) * (1 << PWM_RESOLUTION));
    ledcWrite(CHANNEL_CONFIG[channel].channel, duty);
    enabled[channel] = true;
}

void PWM::disableChannel(uint8_t channel) {
    if(channel >= 3) return;

    // 关闭PWM输出（设置占空比为0）
    ledcWrite(CHANNEL_CONFIG[channel].channel, 0);
    enabled[channel] = false;
}
