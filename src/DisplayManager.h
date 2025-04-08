// DisplayManager.h
#pragma once
#include <U8g2lib.h>
#include "SystemState.h"
#define DISPLAY_UPDATE_INTERVAL 200 // 200ms更新间隔

// 显示模式
#define FULL_DISPLAY   0
#define SAVE_DISPLAY   1

// OLED配置
#define OLED_WIDTH     128
#define OLED_HEIGHT    64


void initDisplay();
void updateFullDisplay(SystemState* state, float rpmA, float rpmB, float voltageA, float dutyA, float dutyB);
void updateSimpleDisplay(uint8_t position, float rpmA, float rpmB, float dutyA, float dutyB);
void handleScreenMode(SystemState* state);
void showError(const char* msg);
void clearError();