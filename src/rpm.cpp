#include "rpm.h"

// 用 volatile 修饰保证中断与主循环间的数据一致
volatile unsigned long pulseCount1 = 0;
volatile unsigned long pulseCount2 = 0;
volatile unsigned long lastRPM1 = 0;
volatile unsigned long lastRPM2 = 0;

// 时间控制变量
unsigned long lastMeasureTime = 0;
const unsigned long period = 1000;   // 总周期：1秒
const unsigned long window = 1000;    // 测量窗口：100ms
bool measuring = false;

// 外部中断服务函数：通道1
void IRAM_ATTR channel1_ISR() {
    pulseCount1++;
}

// 外部中断服务函数：通道2
void IRAM_ATTR channel2_ISR() {
    pulseCount2++;
}

// 初始化 RPM 计算，配置输入引脚和中断
void initRPM(){
    // 此处假设通道1使用 GPIO2，通道2 使用 GPIO3，请根据实际硬件修改引脚编号
    pinMode(2, INPUT);
    pinMode(6, INPUT);

    attachInterrupt(digitalPinToInterrupt(2), channel1_ISR, RISING);
    attachInterrupt(digitalPinToInterrupt(3), channel2_ISR, RISING);

    lastMeasureTime = millis();
}

// 更新 RPM 的采集与计算
void updateRPM(){
    unsigned long now = millis();

    // 当不处于测量状态，并且到了下一个周期时，清零计数并启动测量窗口
    if (!measuring && (now - lastMeasureTime >= period)) {
        noInterrupts();
        pulseCount1 = 0;
        pulseCount2 = 0;
        interrupts();

        lastMeasureTime = now;   // 记录本周期起始时间
        measuring = true;
    }

    // 测量状态下，当 100ms 窗口结束后，计算 RPM
    if (measuring && (now - lastMeasureTime >= window)) {
        // 为防止中断数据变化，进入临界区复制计数变量
        noInterrupts();
        unsigned long count1 = pulseCount1;
        unsigned long count2 = pulseCount2;
        interrupts();

        // 计算公式说明：
        // 100ms内脉冲数 count1 → 1秒脉冲数 = count1 * 10 → RPM = count1 * 10 * 60 = count1 * 600
        lastRPM1 = count1 * 30;
        lastRPM2 = count2 * 30;

        measuring = false;
    }
}

// 获取通道1的最新 RPM 值
unsigned long getRPMCh1(){
    return lastRPM1;
}

// 获取通道2的最新 RPM 值
unsigned long getRPMCh2(){
    return lastRPM2;
}
