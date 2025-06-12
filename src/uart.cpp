#include "uart.h"
#include "pwm.h"
#include "display.h"

// 初始化串口模块
void uartInit() {
    Serial.begin(115200);
    Serial.println("系统启动...");
    Serial.println("双通道风扇转速监控系统");
    Serial.println("----------------------");
}

// 处理串口输入
void handleSerialInput() {
    if (Serial.available()) {
        int val = Serial.parseInt();
        Serial.printf("[DEBUG] 收到串口值: %d\n", val);
        
        if (val >= 0 && val <= 100) {
            setPWMDuty(val);
            lastActivity = millis();
            Serial.printf("[DEBUG] PWM已更新为: %d%%\n", pwmDuty_A);
            
            if (!displayOn) {
                getDisplay().ssd1306_command(SSD1306_DISPLAYON);
                displayOn = true;
            }
            wakeAndRefresh(currentRPM_A, currentRPM_B);
        }
        // 清空串口缓冲区
        while (Serial.available()) Serial.read();
    }
}

