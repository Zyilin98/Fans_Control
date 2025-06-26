#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// === 引脚定义 ===
// ADC引脚
#define ADC_PIN 0          // GPIO0 用作 ADC
#define ADC_MAX_READING 4095.0
#define ADC_REF_VOLTAGE 2500.0  // ESP32-C3 ADC 满量程电压

// 电压分压比例
constexpr float VOLTAGE_DIVIDER_RATIO = 5.70;  // (47k + 10k) / 10k

// I2C引脚
#define I2C_SDA_PIN 3
#define I2C_SCL_PIN 4

// OLED显示屏配置
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

// 旋转编码器引脚
#define PIN_ENC_CLK 9
#define PIN_ENC_DT  8
#define PIN_ENC_SW  10

// 风扇控制引脚
#define PIN_FAN_TACH_A 2   // A通道转速检测
#define PIN_FAN_TACH_B 6   // B通道转速检测
#define PIN_FAN_PWM1 1     // 主PWM输出
#define PIN_FAN_PWM2 7     // 同步PWM输出

// PWM配置
constexpr uint32_t PWM_FREQ_1 = 25000;
constexpr uint32_t PWM_FREQ_2 = 125000;
constexpr uint8_t PULSES_PER_REV = 2;

// 超时配置
constexpr uint32_t DISPLAY_TIMEOUT_MS = 30000;  // 显示超时时间（毫秒）
constexpr uint32_t ROTATION_TIMEOUT_MS = 300;   // 旋转超时时间（毫秒）

// 全局状态变量声明（在各模块中定义）
extern volatile uint32_t pulseCountA;  // A通道脉冲计数
extern volatile uint32_t pulseCountB;  // B通道脉冲计数
extern volatile int pwmDuty_A;           // PWM占空比A
extern volatile int pwmDuty_B;             // PWM占空比B
extern int Naturewind; //自然风状态
extern uint32_t natureWindStartTime;
extern float natureWindPhase;  // 正弦波相位（0-2π）

extern uint32_t lastActivity;
extern int currentChannel;
extern bool displayOn;
extern float currentRPM_A;    // A通道转速
extern float currentRPM_B;    // B通道转速
extern bool isRotating;       // 旋转状态标志
extern uint32_t rotationStartTime;  // 旋转开始时间
extern float measuredVoltage;
extern float measuredVoltageRaw;
extern uint32_t ReadMilliVolts;
extern uint32_t ReadRawVolts;


#endif // CONFIG_H

