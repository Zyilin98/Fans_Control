#include "EncoderHandler.h"
#include "FanController.h"
#include "SystemState.h"

extern FanStatus fanStatus;

#define DEBOUNCE_TIME     5    // ms
#define LONG_PRESS_TIME 800   // ms

volatile int encoderDelta = 0;
EncoderData encoder;

// 中断安全锁
portMUX_TYPE encoderMux = portMUX_INITIALIZER_UNLOCKED;

// 旋转检测中断处理
void IRAM_ATTR handleEncoderInterruptA() {
  portENTER_CRITICAL_ISR(&encoderMux);
  static uint8_t lastState = 0;
  uint8_t a = digitalRead(PIN_EC11_A);
  uint8_t b = digitalRead(PIN_EC11_B);
  uint8_t currentState = (a << 1) | b;
  uint8_t combined = (lastState << 2) | currentState;

  switch(combined) {
    case 0b0001: case 0b0111: case 0b1110: case 0b1000:
      encoderDelta++;
      break;
    case 0b0010: case 0b0100: case 0b1101: case 0b1011:
      encoderDelta--;
      break;
  }
  lastState = currentState;
  portEXIT_CRITICAL_ISR(&encoderMux);
}

// 初始化编码器
void initEncoder() {
  pinMode(PIN_EC11_A, INPUT_PULLUP);
  pinMode(PIN_EC11_B, INPUT_PULLUP);
  pinMode(PIN_EC11_SW, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PIN_EC11_A), handleEncoderInterruptA, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_EC11_B), handleEncoderInterruptA, CHANGE);

  encoder.btnState = BTN_IDLE;
  encoder.pressStart = 0;
  encoder.longPressSent = false;
}

// 获取旋转事件
static EncoderEvent checkRotation() {
  portENTER_CRITICAL(&encoderMux);
  int delta = encoderDelta;
  encoderDelta = 0;
  portEXIT_CRITICAL(&encoderMux);

  if (delta > 0) return EVT_CW;
  if (delta < 0) return EVT_CCW;
  return EVT_NONE;
}

// 按钮检测状态机
static EncoderEvent checkButton() {
  static uint32_t lastTick = 0;
  bool currentState = gpio_get_level((gpio_num_t)PIN_EC11_SW);  // 使用 gpio_get_level 提升响应

  switch (encoder.btnState) {
  case BTN_IDLE:
    if (!currentState) {
      encoder.btnState = BTN_DEBOUNCE;
      lastTick = millis();
    }
    break;

  case BTN_DEBOUNCE:
    if (millis() - lastTick > DEBOUNCE_TIME) {
      if (!gpio_get_level((gpio_num_t)PIN_EC11_SW)) {
        encoder.btnState = BTN_PRESSED;
        encoder.pressStart = millis();
        encoder.longPressSent = false;
      } else {
        encoder.btnState = BTN_IDLE;
      }
    }
    break;

  case BTN_PRESSED:
    if (currentState) {
      encoder.btnState = BTN_RELEASE;
    } else if (!encoder.longPressSent &&
               (millis() - encoder.pressStart > LONG_PRESS_TIME)) {
      encoder.longPressSent = true;
      return EVT_LONG_PRESS;
               }
    break;

  case BTN_RELEASE:
    encoder.btnState = BTN_IDLE;
    return encoder.longPressSent ? EVT_NONE : EVT_SHORT_PRESS;
  }
  return EVT_NONE;
}

// 获取综合事件
EncoderEvent getEncoderEvent() {
  EncoderEvent rotation = checkRotation();
  if (rotation != EVT_NONE) return rotation;
  return checkButton();
}

// 事件处理
void handleEncoder(SystemState *state) {
  const EncoderEvent evt = getEncoderEvent();
  if (evt != EVT_NONE) {
    Serial.printf("[DEBUG] Encoder Event: %d\n", evt);
  }
  switch(evt) {
    case EVT_CW:
      Serial.println("[ENCODER] Rotation: Clockwise");
      if (state->active_channel == 0) {
        if (state->mode_a == 0) {
          fanStatus.targetDutyA = constrain(fanStatus.targetDutyA + 5.0, 0, 100);
        } else {
          fanStatus.currentDutyA = constrain(fanStatus.currentDutyA + 5.0, 0, 100);
        }
      } else {
        fanStatus.currentDutyB = constrain(fanStatus.currentDutyB + 5.0, 0, 100);
      }
      state->active_changed = true;
      break;

    case EVT_CCW:
      Serial.println("[ENCODER] Rotation: Counter-clockwise");
      if (state->active_channel == 0) {
        if (state->mode_a == 0) {
          fanStatus.targetDutyA = constrain(fanStatus.targetDutyA - 5.0, 0, 100);
        } else {
          fanStatus.currentDutyA = constrain(fanStatus.currentDutyA - 5.0, 0, 100);
        }
      } else {
        fanStatus.currentDutyB = constrain(fanStatus.currentDutyB - 5.0, 0, 100);
      }
      state->active_changed = true;
      break;

    case EVT_SHORT_PRESS:
      Serial.println("[BUTTON] Short press detected");
      state->mode_a = !state->mode_a;
      state->mode_changed = true;
      break;

    case EVT_LONG_PRESS:
      Serial.println("[BUTTON] Long press detected");
      state->active_channel = !state->active_channel;
      state->active_changed = true;
      break;

    default:
      break;
  }
}