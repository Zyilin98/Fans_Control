// main.cpp
#include <Arduino.h>
#include "EncoderHandler.h"
#include "DisplayManager.h"
#include "FanController.h"

SystemState sysState;
bool isSystemReady = false; // 系统是否准备好的标志


void setup() {
    Serial.begin(115200);
    Serial.println("System started successfully");
    // 显示初始开机界面
    initDisplay();
    showBootScreen();
    status = INIT_START;
    // 初始化编码器
    initEncoder();
    showBootScreen(); // 更新显示
    status = ENCODER_OK; // 设置状态为编码器初始化完成

    // 初始化风扇控制
    initFans();
    showBootScreen(); // 更新显示
    status = FAN_CTRL_OK; // 设置状态为风扇控制初始化完成

    // I2C初始化（如果需要）
    // Wire.begin();
    showBootScreen(); // 更新显示
    status = I2C_OK; // 设置状态为I2C初始化完成

    // 其他初始化...
    status = SYSTEM_READY; // 最终系统就绪状态
    showBootScreen();

    isSystemReady = true;

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
        if (!isSystemReady) {
            // 如果系统未准备好，持续显示开机界面
            showBootScreen();
            delay(100); // 防止频繁刷新
            return;
        }
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