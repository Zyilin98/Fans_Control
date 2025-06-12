#ifndef ENCODER_H
#define ENCODER_H

#include "config.h"
#include <Versatile_RotaryEncoder.h>

// 初始化旋转编码器模块
void encoderInit();

// 检查编码器状态
void checkEncoder();

// 处理编码器事件
void handleRotate(int8_t rotation);
void handleLongPressRelease();

// 检查编码器状态
void checkEncoder();

// 检查旋转状态
void checkRotationStatus(uint32_t now);

// 添加按钮事件处理声明
void handlePress(uint8_t pressType);
void switchChannel();  // 通道切换函数

#endif // ENCODER_H

