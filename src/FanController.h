// FanController.h
#ifndef FAN_CONTROLLER_H
#define FAN_CONTROLLER_H

#pragma once
#include <Arduino.h>
#include "SystemState.h"

// 风扇状态结构
struct FanStatus {
    volatile uint32_t tachCountA = 0;
    volatile uint32_t tachCountB = 0;
    uint16_t rpmA = 0;
    uint16_t rpmB = 0;
    float voltageA = 0;
    float targetDutyA = 0;
    float currentDutyA = 0;
    float currentDutyB = 0;
    unsigned long lastRpmTime = 0;
};

extern FanStatus fanStatus;  // 只声明

void IRAM_ATTR fanISR_A();
void IRAM_ATTR fanISR_B();
void updateFanData();

// 硬件配置
#define FAN_A_DC_PIN     5
#define FAN_A_PWM_PIN    7
#define FAN_B_PWM_PIN    1
#define TACH_A_PIN       6
#define TACH_B_PIN       2
#define ADC_SENSE_PIN    0

// PWM参数
#define PWM_FREQ         25000
#define PWM_RESOLUTION   8

// 测速参数
#define PULSE_PER_REV    2       // 每转脉冲数
#define RPM_SAMPLE_MS    1000    // RPM采样周期

// PID参数
#define PID_UPDATE_MS    100
#define PID_KP           0.8
#define PID_KI           0.2
#define PID_KD           0.05

void initFans();
void setChannelA_DC(float duty);
void setChannelA_PWM(float duty);
void setChannelB_PWM(float duty);
void updateRPM();
float getVoltageA();
uint16_t getRPM_A();
uint16_t getRPM_B();
void pidControl();
void checkFanStatus();
void emergencyStop();


// 正确声明中断服务程序
extern "C" {
    void IRAM_ATTR fanISR_A();
    void IRAM_ATTR fanISR_B();
}

// 添加函数声明
void updateFanData();


// 在main.cpp中只需attach中断，不需要重新定义

extern FanStatus fanStatus;
inline FanStatus fanStatus;
#endif