#ifndef RPM_H
#define RPM_H

#include <Arduino.h>

// 初始化两个通道的 RPM 计数（中断和变量初始化）
void initRPM();

// 更新 RPM 计算（在主循环中周期性调用，用于采集窗口数据并计算 RPM）
void updateRPM();

// 获取通道1的最新 RPM 值
unsigned long getRPMCh1();

// 获取通道2的最新 RPM 值
unsigned long getRPMCh2();

#endif // RPM_H
