#include "adc.h"

// 全局变量定义
float measuredVoltage = 0.0;
float measuredVoltageRaw = 0.0;
uint32_t ReadMilliVolts;
uint32_t ReadRawVolts;
static float calibrationOffset = 0.0;
static bool isCalibrated = false;

// 初始化ADC模块
void adcInit() {
    pinMode(ADC_PIN, INPUT);
    Serial.println("ADC初始化完成");
}

// 更新电压读数

//校准函数
void calibrateADC() {
    ReadMilliVolts = analogReadMilliVolts(ADC_PIN);
    float v_adc = ReadMilliVolts * VOLTAGE_DIVIDER_RATIO;
    measuredVoltage = (v_adc / 1000.0) - calibrationOffset;

    // 确保电压不为负值
    if (measuredVoltage < 0) measuredVoltage = 0;
    if (pwmDuty_B == 0 && !isCalibrated) {
        // 连续采样10次取平均
        const int samples = 10;
        float sum = 0;

        for (int i = 0; i < samples; i++) {
            sum += ReadMilliVolts;
            delay(10);
        }

        calibrationOffset = (sum / samples) / 1000.0;
        isCalibrated = true;
        Serial.printf("[Debug] 零点偏移量: %.2fV\n", calibrationOffset);
    }
}

// 修改现有电压计算函数
void VoltageRead() {
    ReadMilliVolts = analogReadMilliVolts(ADC_PIN);
    float v_adc = ReadMilliVolts * VOLTAGE_DIVIDER_RATIO;
    measuredVoltage = (v_adc / 1000.0) - calibrationOffset;
}
void VoltageRawRead() {
    ReadRawVolts = analogRead(ADC_PIN);
    float v_adc = (ReadRawVolts / ADC_MAX_READING) * ADC_REF_VOLTAGE;
    measuredVoltageRaw = v_adc * VOLTAGE_DIVIDER_RATIO / 1000.0;  // 转为伏特
}


