#ifndef TACH_H
#define TACH_H

#include "config.h"

// 初始化转速检测模块
void tachInit();

// 计算RPM值
void calculateRPM();

// A通道转速检测中断服务程序
void IRAM_ATTR onTachPulseA();

// B通道转速检测中断服务程序
void IRAM_ATTR onTachPulseB();

#endif // TACH_H

