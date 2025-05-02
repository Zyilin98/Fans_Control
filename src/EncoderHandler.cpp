// EncoderHandler.cpp
#include "EncoderHandler.h"
#include "FanController.h"
#include <Arduino.h>
#include "SystemState.h"
// 在EncoderHandler.cpp开头添加以下包含
#include "pwm.h"          // 解决PWM未声明
extern SystemState sysState; // 声明全局系统状态


Versatile_RotaryEncoder* EncoderHandler::versatile_encoder = nullptr;

void EncoderHandler::initEncoder() {
    const uint8_t clk = 9;
    const uint8_t dt = 8;
    const uint8_t sw = 10;

    versatile_encoder = new Versatile_RotaryEncoder(clk, dt, sw);

    // 绑定有效回调
    versatile_encoder->setHandleRotate(handleRotate);
    versatile_encoder->setHandlePress(handlePress);
    versatile_encoder->setHandleLongPress(handleLongPress);

    // 显式禁用不需要的功能
//    versatile_encoder->setHandleHeldRotate(nullptr);
//    versatile_encoder->setHandleDoublePress(nullptr);
//   versatile_encoder->setHandlePressRelease(nullptr);
//    versatile_encoder->setHandleLongPressRelease(nullptr);
//    versatile_encoder->setHandlePressRotateRelease(nullptr);
//    versatile_encoder->setHandleHeldRotateRelease(nullptr);

    Serial.println("EC11 Encoder Ready!");
}

void EncoderHandler::processEncoder() {
    if (versatile_encoder && versatile_encoder->ReadEncoder()) {
        // 可扩展其他处理逻辑
    }
}

// 实现私有回调函数
// EncoderHandler.cpp
void EncoderHandler::handleRotate(int8_t rotation) {
    float step = 5.0f * rotation;
    Serial.printf("[ENCODER] Rotation:%d Channel:%d Mode:%d\n",
                 rotation, sysState.active_channel, sysState.mode_a);
    if(sysState.active_channel == 0) { // A通道
        if(sysState.mode_a == 0) {     // DC模式
            sysState.duty_a_dc = constrain(sysState.duty_a_dc + step, 0, 100);
            PWM::setDutyCycle(1, sysState.duty_a_dc);
        } else {                      // PWM模式
            sysState.duty_a_pwm = constrain(sysState.duty_a_pwm + step, 0, 100);
            PWM::setDutyCycle(2, sysState.duty_a_pwm);
        }
    } else {                          // B通道
        sysState.duty_b = constrain(sysState.duty_b + step, 0, 100);
        PWM::setDutyCycle(0, sysState.duty_b);
    }
}
void EncoderHandler::handlePress() {
    if(sysState.active_channel == 0) { // 仅在A通道时切换模式
        sysState.mode_a = !sysState.mode_a;
        sysState.mode_changed = true;

        // 模式切换时自动设置占空比
        if(sysState.mode_a) { // 进入PWM模式
            PWM::setDutyCycle(1, 100.0f); // GP5固定100%
        } else {              // 进入DC模式
            PWM::setDutyCycle(2, 100.0f); // GP7固定100%
        }
    }
}
void EncoderHandler::handleLongPress()
{
    sysState.active_channel = !sysState.active_channel;
    sysState.active_changed = true;
}
