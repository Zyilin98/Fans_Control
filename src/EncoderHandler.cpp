// EncoderHandler.cpp
#include "EncoderHandler.h"
#include "FanController.h"   // 提供fanStatus对象
#include "SystemState.h"     // 提供SystemState结构体


// 消抖参数
#define DEBOUNCE_TIME   50    // 毫秒
#define LONG_PRESS_TIME 1000  // 长按判定时间

// 旋转状态机
static volatile uint8_t encoderPos = 0;
static const int8_t encoderStates[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};

// 按键状态机
enum ButtonState {
  BTN_IDLE,
  BTN_DEBOUNCE,
  BTN_PRESSED,
  BTN_RELEASE
};

struct EncoderData {
  uint8_t lastABState : 2;
  ButtonState btnState;
  unsigned long pressStart;
  bool longPressSent;
} encoder;

// 初始化编码器
void initEncoder() {
  pinMode(PIN_EC11_A, INPUT_PULLUP);
  pinMode(PIN_EC11_B, INPUT_PULLUP);
  pinMode(PIN_EC11_SW, INPUT_PULLUP);

  encoder.lastABState = (digitalRead(PIN_EC11_A) << 1) | digitalRead(PIN_EC11_B);
  encoder.btnState = BTN_IDLE;
  encoder.pressStart = 0;
  encoder.longPressSent = false;
}

// 获取旋转事件（带消抖）
static EncoderEvent checkRotation() {
  static unsigned long lastCheck = 0;
  static uint8_t lastStableState = 0;

  uint8_t currentState = (digitalRead(PIN_EC11_A) << 1) | digitalRead(PIN_EC11_B);

  if (currentState != encoder.lastABState) {
    lastCheck = millis();
    encoder.lastABState = currentState;
    return EVT_NONE;
  }

  if ((millis() - lastCheck) < DEBOUNCE_TIME)
    return EVT_NONE;

  if (currentState == lastStableState)
    return EVT_NONE;

  uint8_t stateChange = (lastStableState << 2) | currentState;
  lastStableState = currentState;

  int8_t direction = encoderStates[stateChange];
  return (direction > 0) ? EVT_CW : (direction < 0) ? EVT_CCW : EVT_NONE;
}

// 检测按键事件
static EncoderEvent checkButton() {
  static unsigned long lastDebounce = 0;
  bool currentState = digitalRead(PIN_EC11_SW);

  switch (encoder.btnState) {
    case BTN_IDLE:
      if (!currentState) {
        encoder.btnState = BTN_DEBOUNCE;
        lastDebounce = millis();
      }
      break;

    case BTN_DEBOUNCE:
      if (millis() - lastDebounce > DEBOUNCE_TIME) {
        if (!digitalRead(PIN_EC11_SW)) {
          encoder.btnState = BTN_PRESSED;
          encoder.pressStart = millis();
          encoder.longPressSent = false;
        } else {
          encoder.btnState = BTN_IDLE;
        }
      }
      break;

    case BTN_PRESSED:
      if (digitalRead(PIN_EC11_SW)) {
        encoder.btnState = BTN_RELEASE;
      } else if (!encoder.longPressSent &&
                (millis() - encoder.pressStart > LONG_PRESS_TIME)) {
        encoder.longPressSent = true;
        Serial.println("[BUTTON] Long Press");
        return EVT_LONG_PRESS;
      }
      break;

    case BTN_RELEASE:
      encoder.btnState = BTN_IDLE;
      if (!encoder.longPressSent) {
        Serial.println("[BUTTON] Short Press");
        return EVT_SHORT_PRESS;
      }
      break;
  }
  return EVT_NONE;
}

// 获取最新事件
EncoderEvent getEncoderEvent() {
  EncoderEvent rotationEvent = checkRotation();
  if (rotationEvent != EVT_NONE) return rotationEvent;

  return checkButton();
}

// 处理编码器事件
void handleEncoder(SystemState *state) {
  switch(getEncoderEvent()) {
  case EVT_CW:
    Serial.println("[ROTATION] Clockwise");
    if (state->active_channel == 0) { // 通道A
      if (state->mode_a == 0) { // DC模式
        fanStatus.targetDutyA += 5.0; // 增加5%占空比
        fanStatus.targetDutyA = constrain(fanStatus.targetDutyA, 0, 100);
      } else { // PWM模式
        fanStatus.currentDutyA += 5.0;
        fanStatus.currentDutyA = constrain(fanStatus.currentDutyA, 0, 100);
        setChannelA_PWM(fanStatus.currentDutyA);
      }
    } else { // 通道B
      fanStatus.currentDutyB += 5.0;
      fanStatus.currentDutyB = constrain(fanStatus.currentDutyB, 0, 100);
      setChannelB_PWM(fanStatus.currentDutyB);
    }
    state->active_changed = true;
    break;

  case EVT_CCW:
    // 类似CW处理，但减少值
      Serial.println("[ROTATION] Counter-Clockwise");
      if (state->active_channel == 0) {
        if (state->mode_a == 0) {
          fanStatus.targetDutyA -= 5.0;
          fanStatus.targetDutyA = constrain(fanStatus.targetDutyA, 0, 100);
        } else {
          fanStatus.currentDutyA -= 5.0;
          fanStatus.currentDutyA = constrain(fanStatus.currentDutyA, 0, 100);
          setChannelA_PWM(fanStatus.currentDutyA);
        }
      } else {
        fanStatus.currentDutyB -= 5.0;
        fanStatus.currentDutyB = constrain(fanStatus.currentDutyB, 0, 100);
        setChannelB_PWM(fanStatus.currentDutyB);
      }
    state->active_changed = true;
    break;
    // ... 其他case保持不变
  }
}
