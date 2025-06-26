#include "adc.h"

// 全局变量定义
float measuredVoltage = 0.0;

// 初始化ADC模块
void adcInit() {
    pinMode(ADC_PIN, INPUT);
    Serial.println("ADC初始化完成");
}

// 更新电压读数
void updateVoltageReading() {
    int raw = analogRead(ADC_PIN);
    float v_adc = (raw / ADC_MAX_READING) * ADC_REF_VOLTAGE;
    measuredVoltage = v_adc * VOLTAGE_DIVIDER_RATIO / 1000.0;  // 转为伏特
}

