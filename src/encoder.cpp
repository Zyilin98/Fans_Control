#include "encoder.h"
#include "pwm.h"
#include "display.h"
#include "config.h"

// 全局变量定义
bool isRotating = false;     // 旋转状态标志
int currentChannel = 0;
int Naturewind = 0;
uint32_t rotationStartTime = 0;  // 旋转开始时间
volatile int* pwmDuties[3] = {&pwmDuty_A, &pwmDuty_B, &pwmDuty_C};// 指向PWM值的指针数组


// 创建旋转编码器对象
Versatile_RotaryEncoder *encoder;

// 旋转编码器事件处理函数
void handleRotate(int8_t rotation) {
    if (!isRotating) {
        isRotating = true;
        rotationStartTime = millis();
    }

    // 根据当前通道调整对应的PWM值
    volatile int* currentDuty = pwmDuties[currentChannel];
    int newDuty = *currentDuty + ((rotation > 0) ? 5 : -5);
    *currentDuty = constrain(newDuty, 0, 99);
    
    lastActivity = millis();
    
    // 唤醒显示屏
    if (!displayOn) {
        getDisplay().ssd1306_command(SSD1306_DISPLAYON);
        displayOn = true;
    }
    
    // 刷新显示当前值
    wakeAndRefresh(currentRPM_A, currentRPM_B);
}

void handlePressRelease() {
    Serial.println("[Debug] 切换通道");
    currentChannel += 1;
    if (currentChannel > 2) {
        currentChannel = 0;
    }

}
void handleLongPressRelease() {
    Serial.println("[Debug] 切换自然风");
    Naturewind ^= 1;
}
// 初始化旋转编码器模块
void encoderInit() {
    // 初始化旋转编码器
    encoder = new Versatile_RotaryEncoder(PIN_ENC_CLK, PIN_ENC_DT, PIN_ENC_SW);
    encoder->setHandleRotate(handleRotate);
    encoder->setHandleLongPressRelease(handleLongPressRelease);
    encoder->setHandlePressRelease(handlePressRelease);
    Serial.println("编码器初始化完成");
}

// 检查编码器状态
void checkEncoder() {
    encoder->ReadEncoder();
}

// 检查旋转状态
void checkRotationStatus(uint32_t now) {
    if (isRotating && (now - rotationStartTime > ROTATION_TIMEOUT_MS)) {
        isRotating = false;
        lastActivity = now;
    }
}

