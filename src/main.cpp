// main.cpp
#include <Arduino.h>
#include "EncoderHandler.h"
#include "DisplayManager.h"
#include "FanController.h"

SystemState sysState;

void setup() {
    Serial.begin(115200);

    // 初始化各模块
    initEncoder();
    initDisplay();
    initFans();

    // 配置PWM参数
    ledcSetup(0, 25000, 8); // 通道0，25kHz，8位分辨率（通道A DC）
    ledcSetup(1, 25000, 8); // 通道1（通道A PWM）
    ledcSetup(2, 25000, 8); // 通道2（通道B PWM）

    // 绑定PWM通道
    ledcAttachPin(5, 0);
    ledcAttachPin(7, 1);
    ledcAttachPin(1, 2);
    // ADC校准（实验用）
    const int calibrationReads = 100;
    uint32_t calibSum = 0;
    for(int i=0; i<calibrationReads; i++) {
        calibSum += analogRead(ADC_SENSE_PIN);
    }
    float baseVoltage = (calibSum / (float)calibrationReads) * (2.5 / 4095.0);
    // Serial.printf("ADC Base Voltage: %.2fmV\n", baseVoltage*1000);

    // 确保attachInterrupt使用正确名称
    attachInterrupt(digitalPinToInterrupt(6), fanISR_A, RISING);
    attachInterrupt(digitalPinToInterrupt(2), fanISR_B, RISING);


    // 初始显示
    updateFullDisplay(&sysState, fanStatus.rpmA, fanStatus.rpmB,
                      fanStatus.voltageA, fanStatus.currentDutyA, fanStatus.currentDutyB);
}

void loop() {
    unsigned long currentMillis = millis();
    static unsigned long lastUpdate = 0; // 声明 lastUpdate 变量
    const unsigned long interval = 1000; // 声明 interval 变量，假设间隔为 1000 毫秒

    updateRPM();  // 需要确保在FanController.h中声明
    handleEncoder(&sysState);
    updateRPM();  // 需要确保在FanController.h中声明

    if(sysState.screen_mode) {
        updateSimpleDisplay(0, fanStatus.rpmA, fanStatus.rpmB,
                           fanStatus.currentDutyA, fanStatus.currentDutyB);
    } else {
        updateFullDisplay(&sysState, fanStatus.rpmA, fanStatus.rpmB,
                        fanStatus.voltageA, fanStatus.currentDutyA, fanStatus.currentDutyB);
    }

    extern void updateFanData();  // 在FanController.h中添加声明
    updateFanData();

    if (currentMillis - lastUpdate >= interval) {
        updateFanData();
        updateRPM();
        checkFanStatus();
        handleScreenMode(&sysState);
        lastUpdate = currentMillis;
    }

    if (!sysState.active_channel && !sysState.mode_a) {
        pidControl();
    }
    static unsigned long lastDisplayUpdate = 0;
    if(millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        // 更新显示代码
        lastDisplayUpdate = millis();
    }
    checkFanStatus();
}