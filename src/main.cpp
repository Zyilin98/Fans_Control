// main.cpp
#include <Arduino.h>
#include "SystemState.h"
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
    EncoderHandler::initEncoder();
    showBootScreen(); // 更新显示
    status = ENCODER_OK; // 设置状态为编码器初始化完成
    // 初始化风扇控制
    initFans();
    showBootScreen(); // 更新显示
    status = FAN_CTRL_OK; // 设置状态为风扇控制初始化完成
    Serial.println("Fan Ctrl Ready!");
    // I2C初始化（如果需要）
    // Wire.begin();
    showBootScreen(); // 更新显示
    status = I2C_OK; // 设置状态为I2C初始化完成
    Serial.println("I2C Ready!");
    // 其他初始化...
    status = SYSTEM_READY; // 最终系统就绪状态
    showBootScreen();
    Serial.println("SystemReady Ready!");
    isSystemReady = true;


    // 初始显示
    updateFullDisplay(&sysState, fanStatus.rpmA, fanStatus.rpmB,
                      fanStatus.voltageA, fanStatus.currentDutyA, fanStatus.currentDutyB);
}

void loop() {
/*        if (!isSystemReady) {
            // 如果系统未准备好，持续显示开机界面
            showBootScreen();
            delay(100); // 防止频繁刷新
            return;
        }
*/

    unsigned long currentMillis = millis();
    static unsigned long lastUpdate = 0; // 声明 lastUpdate 变量
    const unsigned long interval = 1; // 声明 interval 变量，假设间隔为 1 毫秒

    EncoderHandler::processEncoder();

    static unsigned long lastDisplayUpdate = 0;
    if(millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        // 更新显示代码
        lastDisplayUpdate = millis();
    }
}