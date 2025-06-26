#include <Arduino.h>
#include "config.h"
#include "tach.h"
#include "display.h"
#include "pwm.h"
#include "adc.h"
#include "encoder.h"
#include "uart.h"
#include "display.h"

void natureWindCycle(unsigned long period = 15000, int maxVal = 80, int minVal = 20);
uint32_t natureWindStartTime = 0;
float natureWindPhase = 0.0;


void setup() {
    // 初始化串口
    uartInit();
    
    // 初始化ADC
    adcInit();

    // 初始化显示模块
    if (!displayInit()) {
        Serial.println("显示模块初始化失败");
        while(1);
    }
    
    // 初始化旋转编码器
    encoderInit();
    
    // 初始化转速检测
    tachInit();
    
    // 初始化PWM
    if (!pwmInit()) {
        Serial.println("PWM初始化失败");
        while(1);
    }
    // ADC校准
    //calibrateADC();

    // 显示欢迎信息
    lastActivity = millis();
    Serial.println("系统就绪，等待操作...");
    Serial.println("======================");
}

void loop() {
    uint32_t now = millis();
    
    // 每秒计算一次RPM
    static uint32_t lastPrint = 0;
    if (now - lastPrint >= 1000) {
        lastPrint = now;
        
        // 计算RPM
        calculateRPM();
        
        if (displayOn) {
            wakeAndRefresh(currentRPM_A, currentRPM_B);
        }
    }
    
    // 处理编码器事件
    checkEncoder();
    
    // 处理串口输入
    handleSerialInput();
    
    // 检测旋转结束
    checkRotationStatus(now);

    // 更新PWM输出
    updatePWMOutputs();

    // 更新电压读数

    VoltageRead();

    VoltageRawRead();
  
    updateVoltageReading();

    //自然风模式
    natureWindCycle();
  
    // 新增1500ms定时器
    static uint32_t lastPrintAA = 0;
    if (now - lastPrintAA >= 2000) {
        lastPrintAA = now;
        // 串口输出双通道数据
        Serial.printf("[DEBUG] 脉冲计数 - A: %d, B: %d\n");
        Serial.printf(
            "[DEBUG] A_RPM: %.0f, B_RPM: %.0f, PWMA: %d%%\nPWMB: %d%%,校准电压: %.0f,原始电压: %.0f,ADC校准电压: %0.lu,ADC原始数据: %0.lu\n",
                     currentRPM_A, currentRPM_B, pwmDuty_A, pwmDuty_B, measuredVoltage, measuredVoltageRaw,ReadMilliVolts, ReadRawVolts);
    }
  
    // OLED屏幕超时关闭
    checkDisplayTimeout(now);

}

void natureWindCycle(unsigned long period,int maxVal,int minVal) {
    if (!Naturewind) {
        return;
    }

    // 首次进入时初始化
    if (natureWindStartTime == 0) {
        natureWindStartTime = millis();
        natureWindPhase = 0.0;
        Serial.println("[Debug] 自然风模式");
    }

    // 计算经过的时间比例 (0.0 - 1.0)
    float elapsedRatio = (millis() - natureWindStartTime) / (float)period;

    // 更新相位（确保在0-2π范围内）
    natureWindPhase = TWO_PI * fmod(elapsedRatio, 1.0);

    // 计算正弦波值并更新PWM
    float sinValue = sin(natureWindPhase);
    pwmDuty_A = minVal + (maxVal - minVal) * (0.5 + 0.5 * sinValue);  // 映射到minVal-maxVal范围

    // 周期结束后重置计时
    if (elapsedRatio >= 1.0) {
        natureWindStartTime = millis();
    }
}
