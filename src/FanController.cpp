// FanController.cpp
#include "FanController.h"
#include "DisplayManager.h"
#include <Arduino.h>

FanStatus fanStatus;  // 实际定义

// FanController.cpp中修改函数名：
void IRAM_ATTR fanISR_A() {
  fanStatus.tachCountA++;
}

void IRAM_ATTR fanISR_B() {
  fanStatus.tachCountB++;
}

void updateFanData() {
  // 合并原main.cpp中的相关操作
  static unsigned long lastCheck = 0;
  if(millis() - lastCheck >= 1000) {
    getVoltageA();
    updateRPM();
    lastCheck = millis();
  }
}


void initFans() {
  // 初始化PWM通道
  ledcSetup(0, PWM_FREQ, PWM_RESOLUTION); // 通道A DC
  ledcSetup(1, PWM_FREQ, PWM_RESOLUTION); // 通道A PWM
  ledcSetup(2, PWM_FREQ, PWM_RESOLUTION); // 通道B PWM

  ledcAttachPin(FAN_A_DC_PIN, 0);
  ledcAttachPin(FAN_A_PWM_PIN, 1);
  ledcAttachPin(FAN_B_PWM_PIN, 2);

  // 初始化测速中断
  pinMode(TACH_A_PIN, INPUT_PULLUP);
  pinMode(TACH_B_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(TACH_A_PIN), fanISR_A, RISING);
  attachInterrupt(digitalPinToInterrupt(TACH_B_PIN), fanISR_B, RISING);

  // 初始化ADC
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
}

void setChannelA_DC(float duty) {
  duty = constrain(duty, 0, 100);
  fanStatus.currentDutyA = duty;
  ledcWrite(0, (duty/100.0) * (1 << PWM_RESOLUTION));
}

void setChannelA_PWM(float duty) {
  duty = constrain(duty, 0, 100);
  fanStatus.currentDutyA = duty;
  ledcWrite(1, (duty/100.0) * (1 << PWM_RESOLUTION));
}

void setChannelB_PWM(float duty) {
  duty = constrain(duty, 0, 100);
  fanStatus.currentDutyB = duty;
  ledcWrite(2, (duty/100.0) * (1 << PWM_RESOLUTION));
}

float getVoltageA() {
  const int samples = 10;
  uint32_t sum = 0;

  for(int i=0; i<samples; i++){
    sum += analogRead(ADC_SENSE_PIN);
    delayMicroseconds(100);
  }
  float adcValue = sum / (float)samples;
  fanStatus.voltageA = 12.0 - (adcValue * 12.0 / 4095.0);
  return fanStatus.voltageA;
}

void updateRPM() {
  static unsigned long lastUpdate = 0;
  unsigned long now = millis();

  if(now - lastUpdate >= RPM_SAMPLE_MS) {
    // 计算转速
    float timeDiff = (now - lastUpdate) / 1000.0;

    noInterrupts();
    uint32_t countA = fanStatus.tachCountA;
    uint32_t countB = fanStatus.tachCountB;
    fanStatus.tachCountA = 0;
    fanStatus.tachCountB = 0;
    interrupts();

    fanStatus.rpmA = (countA * 60.0) / (PULSE_PER_REV * timeDiff);
    fanStatus.rpmB = (countB * 60.0) / (PULSE_PER_REV * timeDiff);
    lastUpdate = now;

    // 更新最后有效转速时间
    if(countA > 0) fanStatus.lastRpmTime = now;
    if(countB > 0) fanStatus.lastRpmTime = now;
  }
}

void pidControl() {
  static unsigned long lastPID = 0;
  static float integral = 0;
  static float lastError = 0;

  unsigned long now = millis();
  if(now - lastPID < PID_UPDATE_MS) return;

  float dt = (now - lastPID) / 1000.0;
  lastPID = now;

  // 获取目标电压（根据目标占空比）
  float targetVoltage = 12.0 * (fanStatus.targetDutyA / 100.0);
  float actualVoltage = getVoltageA();
  float error = targetVoltage - actualVoltage;

  // PID计算
  integral += error * dt;
  float derivative = (error - lastError) / dt;
  lastError = error;

  float output = (PID_KP * error) +
                (PID_KI * integral) +
                (PID_KD * derivative);

  // 限幅处理
  output = constrain(output, 0, 100);
  setChannelA_DC(output);
}

void checkFanStatus() {
  static unsigned long lastCheck = 0;
  if(millis() - lastCheck < 1000) return;

  // 检查堵转
  if((millis() - fanStatus.lastRpmTime) > 2000) {
    if(fanStatus.rpmA == 0 || fanStatus.rpmB == 0) {
      emergencyStop();
      showError("FAN STALL!");
    }
  }

  lastCheck = millis();
}

void emergencyStop() {
  setChannelA_DC(0);
  setChannelA_PWM(0);
  setChannelB_PWM(0);

  detachInterrupt(TACH_A_PIN);
  detachInterrupt(TACH_B_PIN);
}