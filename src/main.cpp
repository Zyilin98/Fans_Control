// main.cpp
#include <Arduino.h>
#include "SystemState.h"
#include "EncoderHandler.h"
#include "DisplayManager.h"
#include "FanController.h"
#include "rpm.h"
#include "pwm.h"
#include "adc.h"

// 修改main.cpp中的初始化方式（兼容C++11标准）
SystemState sysState{};


bool isSystemReady = false; // 系统是否准备好的标志
const uint8_t pwmPins[] = {1, 5, 7};

void setup() {
    Serial.begin(115200);
    Serial.println("System started successfully");
    // 显示初始化
    initDisplay();
    showBootScreen();
    status = INIT_START;

    // 初始化各模块
    EncoderHandler::initEncoder();
    status = ENCODER_OK;
    showBootScreen();

    initRPM();
    PWM::init();
    adcInit(0);
    status = FAN_CTRL_OK;
    showBootScreen();

    status = I2C_OK;
    showBootScreen();

    status = SYSTEM_READY;
    showBootScreen();

    adcCalibrate();

    // 初始化PWM通道
    PWM::enableChannel(0);  // B通道（GP1）
    PWM::enableChannel(1);  // A通道DC（GP5）
    PWM::enableChannel(2);  // A通道PWM（GP7）

    // 初始占空比设置
    PWM::setDutyCycle(1, 0.0f);    // DC模式初始0%
    PWM::setDutyCycle(2, 0.0f);  // PWM模式GP7初始100%

    isSystemReady = true;

}

void loop() {
        static unsigned long lastDisplayUpdate = 0;
        unsigned long currentMillis = millis();

        EncoderHandler::processEncoder();
        updateRPM();

        // 读取所有传感器数据
        float voltageA = readVoltage();
        unsigned long rpmA = getRPMCh1();
        unsigned long rpmB = getRPMCh2();

        // 获取当前占空比
        float dutyA = sysState.active_channel == 0 ?
                     (sysState.mode_a ? sysState.duty_a_pwm : sysState.duty_a_dc) :
                      sysState.duty_b;
        float dutyB = sysState.duty_b;
        // 定期更新显示
        if (currentMillis - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
            if(isSystemReady) {
                // 全功能显示模式
                updateFullDisplay(&sysState,
                                (float)rpmA,
                                (float)rpmB,
                                voltageA,
                                sysState.mode_a ? sysState.duty_a_pwm : sysState.duty_a_dc,
                                sysState.duty_b);
            } else {
                // 系统未就绪时显示启动界面
                showBootScreen();
            }
            lastDisplayUpdate = currentMillis;
        }
        // 处理屏幕模式切换
        handleScreenMode(&sysState);

        // 调试输出
        static unsigned long lastSerialOutput = 0;
        if (currentMillis - lastSerialOutput >= 1000) {
            Serial.printf("[A] Mode:%s DC:%.1f%% PWM:%.1f%% [B] PWM:%.1f%%\n",
                         sysState.mode_a ? "PWM" : "DC",
                         sysState.duty_a_dc,
                         sysState.duty_a_pwm,
                         sysState.duty_b);
            lastSerialOutput = currentMillis;
        }
}