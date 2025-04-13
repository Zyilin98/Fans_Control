// EncoderHandler.h
#pragma once
#include <Arduino.h>
struct SystemState;
// 引脚定义必须与platformio.ini一致
#define PIN_EC11_A  9   // CLK引脚
#define PIN_EC11_B  8   // DT引脚
#define PIN_EC11_SW 10  // 开关引脚

// 编码器事件类型
enum EncoderEvent {
    EVT_NONE,
    EVT_CW,          // 顺时针
    EVT_CCW,         // 逆时针
    EVT_SHORT_PRESS, // 短按
    EVT_LONG_PRESS   // 长按（1秒）
};

// 按钮状态机声明
enum ButtonState {
  BTN_IDLE,
  BTN_DEBOUNCE,
  BTN_PRESSED,
  BTN_RELEASE
};

// 编码器数据结构声明
struct EncoderData {
  ButtonState btnState;
  unsigned long pressStart;
  bool longPressSent;
};

// 全局变量声明
extern volatile int encoderDelta;
extern portMUX_TYPE encoderMux;
extern EncoderData encoder;

// 函数原型声明
void initEncoder();
EncoderEvent getEncoderEvent();
void handleEncoder(SystemState *state);

// 中断服务程序声明（必须包含IRAM_ATTR）
extern "C" {
    void IRAM_ATTR handleEncoderInterruptA();
}
