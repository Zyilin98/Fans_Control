// DisplayManager.cpp
#include "DisplayManager.h"
#include <Arduino.h>

U8G2_SSD1306_128X64_NONAME_F_4W_SW_SPI u8g2(U8G2_R0,
                                              /* clock=*/ 4,
                                              /* data=*/ 3,
                                              /* cs=*/ U8X8_PIN_NONE,
                                              /* dc=*/ U8X8_PIN_NONE,
                                              /* reset=*/ U8X8_PIN_NONE);

static unsigned long lastActiveTime = 0;
static bool errorState = false;
static char errorMsg[32] = {0};

void initDisplay() {
  u8g2.begin();
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
                     float voltageA, float dutyA, float dutyB) {
  u8g2.clearBuffer();

  // 绘制头部
  drawHeader(state);

  // 通道A数据
  u8g2.setFont(FONT_NORMAL);
  u8g2.drawStr(0, 28, "A:");
  drawFanData(rpmA, dutyA, 28);

  // 电压显示（仅通道A DC模式）
  if(state->mode_a == 0 && !state->active_channel) {
    char voltStr[16];
    snprintf(voltStr, sizeof(voltStr), "VOLT:%.2fV", voltageA);
    u8g2.drawStr(0, 40, voltStr);
  }

  // 通道B数据
  u8g2.drawStr(0, 52, "B:");
  drawFanData(rpmB, dutyB, 52);

  // 错误状态显示
  if(errorState) {
    u8g2.setDrawColor(0);
    u8g2.drawBox(0, 0, OLED_WIDTH, 13);
    u8g2.setDrawColor(1);
    u8g2.drawStr(0, 10, "ERROR:");
    u8g2.drawStr(40, 10, errorMsg);
  }

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