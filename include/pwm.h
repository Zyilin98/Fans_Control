#ifndef PWM_H
#define PWM_H

#include "config.h"

// 初始化PWM模块
bool pwmInit();

// 更新PWM输出
void updatePWMOutputs();

// 设置PWM占空比
void setPWMDuty(int dutyA);

#endif // PWM_H

