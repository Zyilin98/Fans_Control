#ifndef ADC_H
#define ADC_H

#include <Arduino.h>

// 初始化 ADC，设置采集的 GPIO 针脚。
// 参数 gpio：ADC 采集使用的 GPIO 号（请确保该引脚支持 ADC）。
void adcInit(uint8_t gpio);

// 校准 ADC（软件校准），要求此时采集点短接至 0V。
// 校准时会多次采样取平均，保存偏置值，后续采集结果会扣除该偏置。
void adcCalibrate();

// 读取 ADC 并转换为电池电压（单位：伏）。
// 此处先通过分压公式还原出原始电压 Vee = V_adc * (57/10)
// 返回值为浮点数电压（单位：伏）。
float readVoltage();

#endif  // ADC_H
