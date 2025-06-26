#ifndef ADC_H
#define ADC_H

#include "config.h"

// 初始化ADC模块
void adcInit();

// 更新电压读数
void VoltageRead();

void VoltageRawRead();

// ADC校准
void calibrateADC();

#endif // ADC_H

