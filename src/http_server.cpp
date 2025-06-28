#include "http_server.h"
#include "config.h"
#include "pwm.h"
WebServer server(8080);

int getChannelSpeed(int channel) {
    return (channel == 1) ? currentRPM_A : currentRPM_B;
}

float getCurrentVoltage() {
    return measuredVoltage;
}

// 处理根路径请求，返回 HTML 页面
void handleRoot() {
    String html = R"(<html><head>
        <meta charset='UTF-8'>
        <script>
            function updateData() {
                // 更新通道占空比
                fetch('/duty_cycle').then(r => r.json()).then(data => {
                    document.getElementById('duty1').textContent = data.ch1;
                    document.getElementById('duty2').textContent = data.ch2;
                });
                // 更新转速数据
                fetch('/speed').then(r => r.json()).then(data => {
                    document.getElementById('rpm1').textContent = data.ch1.toFixed(0);
                    document.getElementById('rpm2').textContent = data.ch2.toFixed(0);
                });

                // 更新电压数据
                fetch('/voltage').then(r => r.json()).then(data => {
                    document.getElementById('voltage').textContent = data.voltage.toFixed(2);
                });

                // 更新自然风状态
                fetch('/natural_wind_status').then(r => r.text()).then(state => {
                    document.getElementById('natural-wind-btn').textContent =
                        (state === '1') ? '关闭' : '开启';
                });
            }
            setInterval(updateData, 1000);
            updateData(); // 初始加载立即更新
        </script>
        </head><body>)";

    html += "<h1>ESP32C3 FansController</h1>";
    html += "<h2>通道占空比控制</h2>";
    html += "<form onsubmit=\"event.preventDefault(); fetch('/channel_duty_cycle?' + new URLSearchParams(new FormData(this))).then(() => updateData())\">";
    html += "通道 1 占空比: <span id='duty1'>" + String(pwmDuty_A) + "</span>% ";
    html += "<input type='number' name='ch1' min='0' max='100'><br>";
    html += "通道 2 占空比: <span id='duty2'>" + String(pwmDuty_B) + "</span>% ";
    html += "<input type='number' name='ch2' min='0' max='100'><br>";
    html += "<input type='submit' value='设置'>";
    html += "</form>";

    html += "<h2>通道转速</h2>";
    html += "通道 1 转速: <span id='rpm1'>" + String(currentRPM_A) + "</span> RPM<br>";
    html += "通道 2 转速: <span id='rpm2'>" + String(currentRPM_B) + "</span> RPM<br>";

    html += "<h2>当前电压</h2>";
    html += "电压: <span id='voltage'>" + String(measuredVoltage, 2) + "</span> V<br>";

    html += "<h2>运行模式</h2>";
    html += "<p>自然风模式：<button id='natural-wind-btn' onclick=\"location.href='/natural_wind'\">";
    html += (Naturewind ? "关闭" : "开启");
    html += "</button></p>";

    html += "</body></html>";
    server.send(200, "text/html; charset=utf-8", html);
}

// 处理通道占空比设置请求
void handleChannelDutyCycle() {
    if (server.hasArg("ch1")) {
        int ch1Duty = server.arg("ch1").toInt();
        pwmDuty_A = constrain(ch1Duty, 0, 100);
        setPWMDuty(pwmDuty_A);
    }
    if (server.hasArg("ch2")) {
        int ch2Duty = server.arg("ch2").toInt();
        pwmDuty_B = constrain(ch2Duty, 0, 100);
        ledcWrite(2, pwmDuty_B * 255 / 100); // 直接控制通道2
    }
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
}

// 处理获取通道转速请求（示例）
void handleSpeed() {
        String response = "{\"ch1\": " + String(currentRPM_A) + ", \"ch2\": " + String(currentRPM_B) + "}";
        server.send(200, "application/json; charset=utf-8", response); // 添加charset
}

// 处理获取当前电压请求（示例）
void handleVoltage() {
    String response = "{\"voltage\": " + String(measuredVoltage, 2) + "}";
    server.send(200, "application/json; charset=utf-8", response); // 添加charset
}

// 处理自然风功能控制请求
void handleNaturalWind() {
    Naturewind = !Naturewind;
    server.sendHeader("location", "/", true);
    server.send(302, "text/plain", "");
}

void handleNaturalWindStatus() {
    server.send(200, "text/plain", String(Naturewind));
}

void handleDutyCycle() {
    String response = "{\"ch1\": " + String(pwmDuty_A) + ", \"ch2\": " + String(pwmDuty_B) + "}";
    server.send(200, "application/json", response);
}

// 初始化 HTTP 服务器
void setupServer() {
    server.on("/", handleRoot);
    server.on("/duty_cycle", handleDutyCycle);
    server.on("/channel_duty_cycle", handleChannelDutyCycle);
    server.on("/speed", handleSpeed);
    server.on("/voltage", handleVoltage);
    server.on("/natural_wind", handleNaturalWind);
    server.on("/natural_wind_status", handleNaturalWindStatus); // 新增状态查询
    server.begin();
    Serial.println("HTTP 服务器已启动");
}