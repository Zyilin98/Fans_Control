#include "pwm.h"

// 全局变量定义
volatile int pwmDuty_A = 0;  // PWM占空比
volatile int pwmDuty_B = 0;  // PWM占空比
// 初始化PWM模块
bool pwmInit() {
    // PWM初始化 - 适配PlatformIO环境
    // 使用ledcSetup和ledcAttachPin替代ledcAttach
    
    // 设置PWM通道1
    ledcSetup(1, PWM_FREQ_1, 8);
    ledcAttachPin(PIN_FAN_PWM1, 1);
    
    // 设置PWM通道2
    ledcSetup(2, PWM_FREQ_2, 8);
    ledcAttachPin(PIN_FAN_PWM2, 2);
    
    updatePWMOutputs();
    Serial.println("[DEBUG] PWM输出已初始化");
    return true;
}

// 更新PWM输出（同步两个引脚）
void updatePWMOutputs() {
    uint32_t dutyA = pwmDuty_A * ((1 << 8) - 1) / 100;
    ledcWrite(1, dutyA);
    uint32_t dutyB = pwmDuty_B * ((1 << 8) - 1) / 100;
    ledcWrite(2, dutyB);
}


// 设置PWM占空比
void setPWMDuty(int dutyA) {
    pwmDuty_A = constrain(dutyA, 0, 100);
    updatePWMOutputs();
}

