// EncoderHandler.h
#pragma once
#include <Arduino.h>
struct SystemState;
// 编码器事件类型
enum EncoderEvent {
    EVT_NONE,
    EVT_CW,
    EVT_CCW,
    EVT_SHORT_PRESS,
    EVT_LONG_PRESS
  };

void initEncoder();
EncoderEvent getEncoderEvent();
void handleEncoder(struct SystemState *state);

// 引脚定义
#define PIN_EC11_A  9
#define PIN_EC11_B  8
#define PIN_EC11_SW 10