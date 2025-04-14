// EncoderHandler.h
#pragma once
#include <Versatile_RotaryEncoder.h>
#include "SystemState.h"
class EncoderHandler {
public:
    static void initEncoder();
    static void processEncoder();
private:
    static Versatile_RotaryEncoder* versatile_encoder;

    // 声明私有回调函数
    static void handleRotate(int8_t rotation);
    static void handlePress();
    static void handleLongPress();

    // 其他需要保留的回调函数声明...
};




