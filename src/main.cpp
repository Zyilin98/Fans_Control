#include <Arduino.h>
#include "config.h"
#include "tach.h"
#include "display.h"
#include "pwm.h"
#include "adc.h"
#include "encoder.h"
#include "uart.h"

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
    updateVoltageReading();
    
    // OLED屏幕超时关闭
    checkDisplayTimeout(now);
}

