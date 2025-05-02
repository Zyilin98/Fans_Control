// DisplayManager.cpp
#include "DisplayManager.h"
#include <Arduino.h>
#include <Wire.h> // 添加I2C库支持
#include "FanController.h" // 确保fanStatus定义可见
InitStatus status = INIT_START;
// 将原有的TF格式字体改为嵌入式字体
#define FONT_NORMAL    u8g2_font_6x12_tf
#define FONT_BOLD      u8g2_font_7x13B_tf

U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 3, /* data=*/ 4, /* reset=*/ U8X8_PIN_NONE);    //Low spped I2C
static unsigned long lastActiveTime = 0;
static bool errorState = false;
static char errorMsg[32] = {0};
// 添加开机界面显示函数
void showBootScreen() {
  u8g2.clearBuffer();
  u8g2.setFont(FONT_BOLD);
  u8g2.drawStr(20, 20, "FAN CONTROLLER");
  switch(status) {
  case INIT_START:
    u8g2.drawStr(20, 35, "INITIALIZING...");
    break;
  case ENCODER_OK:
    u8g2.drawStr(20, 35, "ENCODER OK");
    break;
  case FAN_CTRL_OK:
    u8g2.drawStr(20, 35, "FAN CTRL OK");
    break;
  case I2C_OK:
    u8g2.drawStr(20, 35, "I2C BUS OK");
    break;
  case SYSTEM_READY:
    u8g2.drawStr(20, 35, "SYSTEM READY");
    break;
  }
  u8g2.sendBuffer();
}
void initDisplay() {
//  Wire.begin(); // 初始化I2C总线（SDA-pin3, SCL-pin4）
  u8g2.begin();
  u8g2.setContrast(128);
  u8g2.setContrast(128);
  u8g2.setFont(FONT_NORMAL);
  u8g2.clearBuffer();
  u8g2.sendBuffer();
  lastActiveTime = millis();
}

void drawHeader(SystemState* state) {
  u8g2.setFont(FONT_BOLD);
  u8g2.drawStr(0, 12, state->active_channel == 0 ? "Channel A" : "Channel B");

  // 显示模式指示
  if(state->active_channel == 0) {
    u8g2.setFont(FONT_NORMAL);
    u8g2.drawStr(80, 12, state->mode_a == 0 ? "[DC]" : "[PWM]");
  }
}

void drawFanData(float rpm, float duty, uint8_t yPos) {
  char buf[20];
  snprintf(buf, sizeof(buf), "RPM:%.0f", rpm);
  u8g2.drawStr(0, yPos, buf);

  snprintf(buf, sizeof(buf), "DUTY:%.1f%%", duty);
  u8g2.drawStr(64, yPos, buf);
}

void updateFullDisplay(SystemState* state, float rpmA, float rpmB,
                     float voltageA, float dutyA, float dutyB)
{
  u8g2.clearBuffer();

  // 头部状态
  u8g2.setFont(FONT_BOLD);
  u8g2.drawStr(0, 12, state->active_channel ? "Channel B" : "Channel A");

  // 模式指示器
  if(!state->active_channel) {
    u8g2.setFont(FONT_NORMAL);
    u8g2.drawStr(100, 12, state->mode_a ? "PWM" : "DC");
  }

  // 通道A数据
  u8g2.setFont(FONT_NORMAL);
  char buf[32];
  snprintf(buf, sizeof(buf), "A: %04dRPM %05.1f%%",
          (int)rpmA, dutyA);
  u8g2.drawStr(0, 28, buf);

  // 电压显示（仅A通道DC模式）
  if(state->active_channel == 0 && state->mode_a == 0) {
    snprintf(buf, sizeof(buf), "Voltage: %5.2fV", voltageA);
    u8g2.drawStr(0, 40, buf);
  }

  // 通道B数据
  snprintf(buf, sizeof(buf), "B: %04dRPM %05.1f%%",
          (int)rpmB, state->duty_b);
  u8g2.drawStr(0, 52, buf);

  u8g2.sendBuffer();
}



void updateSimpleDisplay(uint8_t position, float rpmA, float rpmB,
                        float dutyA, float dutyB) {
  static const uint8_t yPositions[4][2] = {
    {10, 30}, {10, 50}, {30, 50}, {10, 40}
  };

  u8g2.clearBuffer();

  // 通道A
  char buf[20];
  snprintf(buf, sizeof(buf), "A:%.0f/%.0f%%", rpmA, dutyA);
  u8g2.drawStr(0, yPositions[position][0], buf);

  // 通道B
  snprintf(buf, sizeof(buf), "B:%.0f/%.0f%%", rpmB, dutyB);
  u8g2.drawStr(0, yPositions[position][1], buf);

  u8g2.sendBuffer();
}

void handleScreenMode(SystemState* state) {
  static unsigned long screenTimer = millis();
  static uint8_t screenPos = 0;

  if(state->screen_mode) {
    // 防烧屏模式
    if(millis() - screenTimer > 30000) {
      screenPos = (screenPos + 1) % 4;
      screenTimer = millis();
    }
  } else {
    // 正常模式
    if(millis() - lastActiveTime > 30000) {
      state->screen_mode = true;
      screenTimer = millis();
    }
  }

  // 检测到输入活动时重置计时器
  if(state->screen_mode && (state->active_changed || state->mode_changed)) {
    state->screen_mode = false;
    lastActiveTime = millis();
  }
}

void showError(const char* msg) {
  strncpy(errorMsg, msg, sizeof(errorMsg)-1);
  errorState = true;
  // 临时禁用错误显示更新，或传递必要参数
  // updateFullDisplay(nullptr, 0, 0, 0, 0, 0); // 临时方案
}

void clearError() {
  errorState = false;
  errorMsg[0] = '\0';
}