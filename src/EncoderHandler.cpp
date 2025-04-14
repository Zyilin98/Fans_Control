// EncoderHandler.cpp
#include "EncoderHandler.h"
#include "FanController.h"
#include <Arduino.h>

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
void EncoderHandler::handleRotate(int8_t rotation) {
    Serial.print("Rotated: ");
    Serial.println(rotation > 0 ? "Right" : "Left");
}

void EncoderHandler::handlePress() {
    Serial.println("Button Pressed");
}

void EncoderHandler::handleLongPress() {
    Serial.println("Long Press Detected");
}
