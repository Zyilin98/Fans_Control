#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// 初始化显示模块
bool displayInit();

// 刷新显示内容
void wakeAndRefresh(float rpmA, float rpmB);

// 检查显示超时
void checkDisplayTimeout(uint32_t now);

// 获取显示对象引用
Adafruit_SSD1306& getDisplay();

#endif // DISPLAY_H

