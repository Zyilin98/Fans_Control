#include "display.h"
#include <Wire.h>

// 全局变量定义
bool displayOn = true;
uint32_t lastActivity = 0;

// 创建OLED显示对象
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 初始化显示模块
bool displayInit() {
    // OLED初始化
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
        Serial.println("OLED初始化失败");
        return false;
    }
    display.clearDisplay();
    display.display();
    Serial.println("OLED显示屏初始化完成");
    return true;
}

// OLED显示刷新函数（双通道版本）
void wakeAndRefresh(float rpmA, float rpmB) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    
    // 显示通道A RPM
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("A RPM:");
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.printf("%4d", (int)rpmA);
    
    // 显示通道B RPM
    display.setTextSize(1);
    display.setCursor(64, 0);
    display.print("B RPM:");
    display.setTextSize(2);
    display.setCursor(64, 10);
    display.printf("%4d", (int)rpmB);
    
    // 分隔线
    display.drawLine(0, 35, 128, 35, SSD1306_WHITE);
    
    // 显示PWM值
    if (currentChannel == 0) {
        display.setTextSize(2);
        display.setCursor(0, 42);
        display.print("PWMA:");
        display.setTextSize(2);
        display.setCursor(56, 42);
        display.printf("%2d", pwmDuty_A);
    } else {
        display.setTextSize(2);
        display.setCursor(0, 42);
        display.print("PWMB:");
        display.setTextSize(2);
        display.setCursor(56, 42);
        display.printf("%2d", pwmDuty_B);
    }
    //显示模式
    if (Naturewind) {
        display.setTextSize(1);
        display.setCursor(SCREEN_WIDTH - 12, 2);  // 右上角位置
        display.print("N");
    }
    //显示wifi状态
    if (nowifi == 0) {
        display.setTextSize(1);
        display.setCursor(2, 56);  //左下角位置
        display.print("Wifi");
    }
    //显示ADC电压
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(90, 56);
    display.print(measuredVoltage, 2);
    display.print("V");
    
    display.display();
}

// 检查显示超时
void checkDisplayTimeout(uint32_t now) {
    if (displayOn && !isRotating && (now - lastActivity >= DISPLAY_TIMEOUT_MS)) {
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        displayOn = false;
        Serial.println("[DEBUG] 显示屏已关闭（超时）");
    }
}

// 获取显示对象引用
Adafruit_SSD1306& getDisplay() {
    return display;
}

