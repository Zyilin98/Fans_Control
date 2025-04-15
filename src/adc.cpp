#include "adc.h"

// 使用的 ADC 引脚号（由 adcInit() 设置）
static uint8_t adc_gpio = 0;  // 默认可使用的 ADC 针脚，可根据实际情况修改

// 保存校准得到的零点偏置（单位：毫伏）
static uint32_t adc_offset = 0;

// 初始化 ADC: 设置分辨率和衰减
void adcInit(uint8_t gpio)
{
    adc_gpio = gpio;
    // 设定 12 位分辨率（0~4095）
    analogReadResolution(12);
    // 对于 ESP32-C3 选择 11dB 衰减，推荐测量范围大约 0~3100mV（针对 ESP32-C3 默认）
    analogSetPinAttenuation(adc_gpio, ADC_ATTENDB_MAX);
}

// 软件校准 ADC，当输入接 0V 时调用，取多次采样的平均值作为偏置
void adcCalibrate()
{
    const int numSamples = 10;
    uint32_t sum = 0;
    for (int i = 0; i < numSamples; i++) {
        sum += analogReadMilliVolts(adc_gpio);
        delay(10);
    }
    adc_offset = sum / numSamples;
    Serial.print("ADC 校准偏置 = ");
    Serial.print(adc_offset);
    Serial.println(" mV");
}

// 内部实现：利用已采集值计算电池电压
// 电路采用分压电阻：10K（上）与 47K（下），公式为：V_adc = Vee * (10K/(10K+47K))，
// 故 Vee = V_adc * ((10K+47K)/10K)= V_adc * 5.7
static float computeBatteryVoltage(uint32_t adc_mV)
{
    // 除去校准偏置
    if (adc_mV > adc_offset)
        adc_mV -= adc_offset;
    else
        adc_mV = 0;
    // 还原电压（单位：毫伏）
    float battery_mV = adc_mV * 5.7;
    return battery_mV / 1000.0; // 转换为伏
}

// readVoltage() 内部控制 500ms 采样间隔
float readVoltage()
{
    // 用静态变量保存上次采样时间和最新结果
    static unsigned long lastSampleTime = 0;
    static float latestVoltage = 0.0;

    unsigned long now = millis();
    // 每 500ms 重新采样一次（实际采样时间仅占用几毫秒，不会阻塞）
    if (now - lastSampleTime >= 500) {
        lastSampleTime = now;
        // 直接利用 Arduino 提供的 analogReadMilliVolts() 获取经过校准的 ADC 电压（单位：mV）
        uint32_t adc_mV = analogReadMilliVolts(adc_gpio);
        latestVoltage = computeBatteryVoltage(adc_mV);
    }
    return latestVoltage;
}
