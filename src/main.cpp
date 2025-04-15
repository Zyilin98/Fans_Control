// main.cpp
#include <Arduino.h>
#include "SystemState.h"
#include "EncoderHandler.h"
//#include "DisplayManager.h"
#include "FanController.h"
#include "rpm.h"
#include "pwm.h"
#include "adc.h"

SystemState sysState;
bool isSystemReady = false; // 系统是否准备好的标志
const uint8_t pwmPins[] = {1, 5, 7};

void setup() {
    Serial.begin(115200);
    Serial.println("System started successfully");
    // 显示初始开机界面
//    initDisplay();
//    showBootScreen();
//    status = INIT_START;
    // 初始化编码器
    EncoderHandler::initEncoder();
//    showBootScreen(); // 更新显示
//    status = ENCODER_OK; // 设置状态为编码器初始化完成
    // 初始化风扇控制
    initRPM();
    PWM::init();
    adcInit(0);
//    showBootScreen(); // 更新显示
//    status = FAN_CTRL_OK; // 设置状态为风扇控制初始化完成
    Serial.println("Fan Ctrl Ready!");
    // I2C初始化（如果需要）
    // Wire.begin();
 //   showBootScreen(); // 更新显示
//    status = I2C_OK; // 设置状态为I2C初始化完成
    Serial.println("I2C Ready!");
    // 其他初始化...
 //   status = SYSTEM_READY; // 最终系统就绪状态
//    showBootScreen();
    Serial.println("SystemReady Ready!");
    isSystemReady = true;
    adcCalibrate(); //校准ADC
    Serial.println("ADC 校准完成！");
//    float voltage = readVoltage(); //获得电压
    PWM::enableChannel(1);    // 启用通道0
    PWM::setDutyCycle(1, 100.0f);    // 设置通道0占空比为75%
//    PWM::disableChannel(1);    // 禁用通道1
    // 初始显示

}

void loop() {

    unsigned long currentMillis = millis();
    static unsigned long lastUpdate = 0; // 声明 lastUpdate 变量
    const unsigned long interval = 1; // 声明 interval 变量，假设间隔为 1 毫秒

    EncoderHandler::processEncoder();
    // 更新 RPM 数据
    updateRPM();

    // 读取当前通道的 RPM 结果（此处简化处理为周期性获取）
    unsigned long rpm1 = getRPMCh1();
    unsigned long rpm2 = getRPMCh2();

    Serial.print("通道1 RPM: ");
    Serial.println(rpm1);

    Serial.print("通道2 RPM: ");
    Serial.println(rpm2);
    float voltage = readVoltage();
    Serial.print("电池电压 (V) = ");
    Serial.println(voltage, 2);
/*    static unsigned long lastDisplayUpdate = 0;
    if(millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL) {
        // 更新显示代码
        lastDisplayUpdate = millis();
    }
 */
}