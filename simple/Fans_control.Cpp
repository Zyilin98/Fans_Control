#include <Wire.h>  // 包含用于 I2C 通信的 Wire 库
#include <Adafruit_GFX.h>  // 包含 Adafruit GFX 图形库，用于图形显示
#include <Adafruit_SSD1306.h>  // 包含 Adafruit SSD1306 库，用于控制 OLED 显示屏

// === 配置区 ===
#define I2C_SDA_PIN 8  // 定义 I2C 通信的 SDA 引脚为 8 号引脚
#define I2C_SCL_PIN 9  // 定义 I2C 通信的 SCL 引脚为 9 号引脚

#define SCREEN_WIDTH 128  // 定义 OLED 显示屏的宽度为 128 像素
#define SCREEN_HEIGHT 64  // 定义 OLED 显示屏的高度为 64 像素
#define OLED_RESET -1  // 定义 OLED 的复位引脚，-1 表示使用内部复位

#define PIN_BTN_UP 0  // 定义向上按钮连接的引脚为 0 号引脚
#define PIN_BTN_DOWN 1  // 定义向下按钮连接的引脚为 1 号引脚

#define PIN_FAN_TACH 2  // 定义风扇转速检测引脚（TACH）为 2 号引脚
#define PIN_FAN_PWM 3  // 定义风扇 PWM 控制引脚为 3 号引脚

const uint32_t PWM_FREQ = 25000;  // 定义 PWM 频率为 25000Hz
const uint8_t PWM_RES = 8;  // 定义 PWM 分辨率为 8 位
const uint8_t PULSES_PER_REV = 2;  // 定义风扇每转产生的脉冲数为 2 个

// 全局状态变量
volatile uint32_t pulseCount = 0;  // 用于记录风扇转速检测到的脉冲数，volatile 关键字表示该变量可能被中断服务程序修改
int pwmDuty = 0;  // 表示 PWM 占空比，范围为 0 - 100%
uint32_t lastActivity = 0;  // 记录最后一次 “交互”（按键或串口通信）的时间，单位为毫秒
bool displayOn = true;  // 表示 OLED 显示屏是否处于开启状态

// 创建 OLED 对象，指定显示屏的宽度、高度以及使用的 I2C 总线和复位引脚
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Tach 中断服务程序，用于处理风扇转速检测引脚上的脉冲信号
void IRAM_ATTR onTachPulse() {
  pulseCount++;  // 每检测到一个脉冲，脉冲计数加 1
}

// 唤醒 OLED 显示屏并刷新显示内容的函数
void wakeAndRefresh(float rpm) {
  if (!displayOn) {  // 如果显示屏处于关闭状态
    display.ssd1306_command(SSD1306_DISPLAYON);  // 发送命令唤醒 OLED 显示屏
    displayOn = true;  // 更新显示屏状态为开启
  }
  // 清除显示屏内容并设置文本颜色
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  // 显示 RPM（转速）信息
  display.setTextSize(2);  // 设置文本大小为 2
  display.setCursor(0, 0);  // 设置文本起始位置
  display.print("RPM:");  // 显示 “RPM:” 文本
  display.setTextSize(3);  // 设置文本大小为 3
  display.setCursor(0, 18);  // 设置文本起始位置
  display.printf("%4d", (int)rpm);  // 显示转速值，保留 4 位数字

  // 显示 PWM 占空比信息
  display.setTextSize(2);  // 设置文本大小为 2
  display.setCursor(0, 42);  // 设置文本起始位置
  display.print("PWM:");  // 显示 “PWM:” 文本
  display.setTextSize(3);  // 设置文本大小为 3
  display.setCursor(72, 42);  // 设置文本起始位置
  display.printf("%2d", pwmDuty);  // 显示 PWM 占空比值，保留 2 位数字

  display.display();  // 刷新显示屏，使显示内容生效
}

void setup() {  // 程序初始化函数
  Serial.begin(115200);  // 初始化串口通信，波特率为 115200

  // OLED 初始化
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);  // 初始化 I2C 通信，指定 SDA 和 SCL 引脚
  // 检查 OLED 是否初始化成功，若失败则进入死循环
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init failed");
    while (1);
  }
  display.clearDisplay();  // 清除显示屏内容
  display.display();  // 刷新显示屏

  // 设置按键和转速检测引脚模式
  pinMode(PIN_BTN_UP, INPUT_PULLUP);  // 设置向上按钮引脚为输入模式，并启用内部上拉电阻
  pinMode(PIN_BTN_DOWN, INPUT_PULLUP);  // 设置向下按钮引脚为输入模式，并启用内部上拉电阻
  pinMode(PIN_FAN_TACH, INPUT_PULLUP);  // 设置转速检测引脚为输入模式，并启用内部上拉电阻
  // 附中断服务程序到转速检测引脚，检测到下降沿触发中断
  attachInterrupt(digitalPinToInterrupt(PIN_FAN_TACH), onTachPulse, FALLING);

  // PWM 初始化（新版 API）
  // 检查是否成功将 PWM 频率和分辨率设置到指定引脚，若失败则进入死循环
  if (!ledcAttach(PIN_FAN_PWM, PWM_FREQ, PWM_RES)) {
    Serial.println("LEDC Attach Failed");
    while (1);
  }
  ledcWrite(PIN_FAN_PWM, 0);  // 设置 PWM 占空比为 0，即风扇停止转动

  // 初始化最后一次活动时间为当前时间
  lastActivity = millis();
}

void loop() {  // 程序主循环函数
  uint32_t now = millis();  // 获取当前时间（毫秒）

  // 每秒统计一次 RPM 并更新状态
  static uint32_t lastPrint = 0;  // 静态变量，用于记录上次打印时间
  // 如果当前时间与上次打印时间间隔超过 1 秒，则执行以下操作
  if (now - lastPrint >= 1000) {
    lastPrint = now;  // 更新上次打印时间为当前时间

    noInterrupts();  // 禁用中断，防止中断服务程序修改 pulseCount 变量
    uint32_t cnt = pulseCount;  // 获取当前脉冲计数值
    pulseCount = 0;  // 将脉冲计数清零
    interrupts();  // 重新启用中断

    // 根据脉冲数计算转速（单位：转/分钟）
    float rpm = cnt * 60.0f / PULSES_PER_REV;
    // 通过串口打印转速和 PWM 占空比信息
    Serial.printf("RPM: %.0f, PWM: %d%%\n", rpm, pwmDuty);

    if (displayOn) {  // 如果显示屏处于开启状态
      // 调用 wakeAndRefresh 函数更新显示内容
      wakeAndRefresh(rpm);
    }
  }

  // 处理按键和串口交互
  bool interacted = false;  // 用于标记是否发生了交互操作
  // 检测向上按钮是否被按下
  if (digitalRead(PIN_BTN_UP) == LOW) {
    delay(50);  // 短暂延时，用于消抖
    // 再次检查按钮状态，确认按键按下
    if (digitalRead(PIN_BTN_UP) == LOW) {
      pwmDuty += 10;  // 增加 PWM 占空比 10%
      if (pwmDuty > 100) pwmDuty = 0;  // 如果占空比超过 100%，则重置为 0%
      // 根据 PWM 分辨率计算实际占空比值，并设置到 PWM 引脚
      uint32_t duty = pwmDuty * ((1 << PWM_RES) - 1) / 100;
      ledcWrite(PIN_FAN_PWM, duty);
      interacted = true;  // 标记发生了交互操作
      // 等待按钮松开
      while (digitalRead(PIN_BTN_UP) == LOW);
    }
  }
  // 检测向下按钮是否被按下，逻辑与向上按钮类似，但减少 PWM 占空比
  if (digitalRead(PIN_BTN_DOWN) == LOW) {
    delay(50);
    if (digitalRead(PIN_BTN_DOWN) == LOW) {
      pwmDuty -= 10;
      if (pwmDuty < 0) pwmDuty = 100;
      uint32_t duty = pwmDuty * ((1 << PWM_RES) - 1) / 100;
      ledcWrite(PIN_FAN_PWM, duty);
      interacted = true;
      while (digitalRead(PIN_BTN_DOWN) == LOW);
    }
  }
  // 检测串口是否有数据可供读取
  if (Serial.available()) {
    // 从串口读取一个整数值
    int val = Serial.parseInt();
    // 如果读取的值在 0 - 100 范围内，则更新 PWM 占空比
    if (val >= 0 && val <= 100) {
      pwmDuty = val;
      uint32_t duty = pwmDuty * ((1 << PWM_RES) - 1) / 100;
      ledcWrite(PIN_FAN_PWM, duty);
      interacted = true;
    }
  }
  // 如果发生了交互操作，则更新最后一次活动时间，并刷新显示
  if (interacted) {
    lastActivity = now;
    // 禁用中断，获取当前脉冲计数值，重新启用中断
    noInterrupts();
    uint32_t cnt = pulseCount;
    interrupts();
    // 根据脉冲数计算转速并调用 wakeAndRefresh 函数更新显示
    float rpm = cnt * 60.0f / PULSES_PER_REV;
    wakeAndRefresh(rpm);
  }

  // 防止 OLED 烧屏功能
  // 如果显示屏处于开启状态且距离最后一次活动时间超过 30 秒，则关闭显示屏
  if (displayOn && (now - lastActivity >= 30000)) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    displayOn = false;
  }

  delay(10);  // 短暂延时，减少 CPU 占用率
}
