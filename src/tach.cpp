#include "tach.h"

// 全局变量定义
volatile uint32_t pulseCountA = 0;  // A通道脉冲计数
volatile uint32_t pulseCountB = 0;  // B通道脉冲计数
float currentRPM_A = 0.0;    // A通道转速
float currentRPM_B = 0.0;    // B通道转速

// A通道转速检测中断服务程序
void IRAM_ATTR onTachPulseA() {
    pulseCountA++;
}

// B通道转速检测中断服务程序
void IRAM_ATTR onTachPulseB() {
    pulseCountB++;
}

// 初始化转速检测模块
void tachInit() {
    // 转速检测引脚设置
    pinMode(PIN_FAN_TACH_A, INPUT_PULLUP);
    pinMode(PIN_FAN_TACH_B, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_FAN_TACH_A), onTachPulseA, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_FAN_TACH_B), onTachPulseB, FALLING);
    Serial.println("[DEBUG] 转速检测中断已启用");
}

// 计算RPM值
void calculateRPM() {
    // 读取脉冲计数并重置
    noInterrupts();
    uint32_t cntA = pulseCountA;
    uint32_t cntB = pulseCountB;
    pulseCountA = 0;
    pulseCountB = 0;
    interrupts();
    
    // 计算双通道RPM
    currentRPM_A = cntA * 60.0f / PULSES_PER_REV;
    currentRPM_B = cntB * 60.0f / PULSES_PER_REV;

}

