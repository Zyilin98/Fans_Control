#include "http_server.h"
#include "config.h"
#include "pwm.h"
WebServer server(80);

/*
int getChannelSpeed(int channel) {
    return (channel == 1) ? currentRPM_A : currentRPM_B;
}

float getCurrentVoltage() {
    return measuredVoltage;
}
*/

unsigned long lastHeartbeat = 0;
bool clientConnected = false;


void handleRoot() {
    // 使用 C++11 原始字符串字面量，方便编写多行 HTML
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <!-- 引入 Bootstrap 5 -->
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
  <title>ESP32C3 FansController</title>
  <style>
    body { background-color: #f8f9fa; }
    .status-badge { position: fixed; top: 1rem; right: 1rem; padding: .5rem 1rem; border-radius: .25rem; color: #fff; }
    .slider-label { font-weight: 500; }
  </style>
</head>
<body>
  <div class="container py-4">
    <h1 class="text-center mb-4">ESP32C3 FansController</h1>

    <!-- 状态指示条 -->
    <span id="connection-status" class="status-badge bg-secondary">连接状态: 未连接</span>

    <div class="row gy-4">
      <!-- 占空比控制卡片 -->
      <div class="col-md-6">
        <div class="card shadow-sm">
          <div class="card-header">通道占空比控制</div>
          <div class="card-body">
            <p>拖动滑块后点击“应用”设置占空比，或观察当前占空比状态：</p>
            <div class="mb-3">
              <label for="ch1Range" class="slider-label">通道 1: <span id="duty1">0</span>%</label>
              <input type="range" class="form-range" id="ch1Range" min="0" max="100">
            </div>
            <div class="mb-3">
              <label for="ch2Range" class="slider-label">通道 2: <span id="duty2">0</span>%</label>
              <input type="range" class="form-range" id="ch2Range" min="0" max="100">
            </div>
            <div class="mb-3">
              <label for="ch3Range" class="slider-label">通道 3: <span id="duty3">0</span>%</label>
              <input type="range" class="form-range" id="ch3Range" min="0" max="100">
            </div>
            <button id="setDutyBtn" class="btn btn-primary">应用</button>
          </div>
        </div>
      </div>

      <!-- 转速及电压信息卡片 -->
      <div class="col-md-6">
        <div class="card shadow-sm mb-4">
          <div class="card-header">实时数据</div>
          <div class="card-body">
            <p>通道 1 转速: <strong><span id="rpm1">0</span> RPM</strong></p>
            <p>通道 2 转速: <strong><span id="rpm2">0</span> RPM</strong></p>
            <p>电压: <strong><span id="voltage">0.00</span> V</strong></p>
          </div>
        </div>

        <!-- 运行模式卡片 -->
        <div class="card shadow-sm">
          <div class="card-header">运行模式</div>
          <div class="card-body text-center">
            <button id="naturalWindBtn" class="btn btn-outline-secondary">自然风: 加载中...</button>
          </div>
        </div>
      </div>
    </div>

    <!-- WiFi 信息卡片 -->
    <div class="card shadow-sm mt-4">
      <div class="card-header">WiFi 信息</div>
      <div class="card-body d-flex justify-content-between">
        <div>SSID: <span id="wifi-ssid">获取中...</span></div>
        <div>IP: <span id="wifi-ip">获取中...</span></div>
        <div>RSSI: <span id="wifi-rssi">获取中...</span> dBm</div>
      </div>
    </div>
  </div>

  <!-- 引入 Bootstrap JS 和主逻辑脚本 -->
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script>
  <script>
    let missedBeats = 0;
    let naturalWindOn = false;

    // 心跳检测
    function checkHeartbeat() {
      fetch('/heartbeat')
        .then(() => { missedBeats = 0; updateStatus(true); })
        .catch(() => { if (++missedBeats >= 5) updateStatus(false); });
    }
    function updateStatus(connected) {
      const badge = document.getElementById('connection-status');
      badge.textContent = connected ? '连接状态: 已连接' : '连接状态: 未连接';
      badge.className = 'status-badge ' + (connected ? 'bg-success' : 'bg-danger');
    }
    setInterval(checkHeartbeat, 1000);

    // 更新 WiFi 信息
    function updateWifiInfo() {
      fetch('/wifi_info').then(r => r.json()).then(data => {
        document.getElementById('wifi-ssid').textContent = data.ssid;
        document.getElementById('wifi-ip').textContent = data.ip;
        document.getElementById('wifi-rssi').textContent = data.rssi;
      });
    }
    setInterval(updateWifiInfo, 5000);
    updateWifiInfo();

    // 实时更新占空比
    function updateDuty() {
      fetch('/duty_cycle').then(r => r.json()).then(data => {
        document.getElementById('duty1').textContent = data.ch1;
        document.getElementById('duty2').textContent = data.ch2;
        document.getElementById('duty3').textContent = data.ch3;
      });
    }
    setInterval(updateDuty, 1000);
    updateDuty();

    // 实时更新转速、电压
    function updateData() {
      fetch('/speed').then(r => r.json()).then(data => {
        document.getElementById('rpm1').textContent = data.ch1.toFixed(0);
        document.getElementById('rpm2').textContent = data.ch2.toFixed(0);
      });
      fetch('/voltage').then(r => r.json()).then(data => {
        document.getElementById('voltage').textContent = data.voltage.toFixed(2);
      });
    }
    setInterval(updateData, 1000);
    updateData();

    // 初始化滑块与文本
    function initDuty() {
      fetch('/duty_cycle').then(r => r.json()).then(data => {
        ['1','2','3'].forEach(i => {
          document.getElementById(`ch${i}Range`).value = data[`ch${i}`];
          document.getElementById(`duty${i}`).textContent = data[`ch${i}`];
        });
      });
    }
    initDuty();

    // 滑块移动更新文本
    ['ch1Range','ch2Range','ch3Range'].forEach(id => {
      document.getElementById(id).addEventListener('input', e => {
        const ch = id.charAt(2);
        document.getElementById(`duty${ch}`).textContent = e.target.value;
      });
    });

    // 应用占空比
    document.getElementById('setDutyBtn').onclick = () => {
      const params = new URLSearchParams();
      ['1','2','3'].forEach(i => params.append(`ch${i}`, document.getElementById(`ch${i}Range`).value));
      fetch('/channel_duty_cycle?' + params);
    };

    // 加载自然风状态，使用纯文本接口，0/1
    function loadNaturalStatus() {
      fetch('/natural_wind_status')
        .then(r => r.text())
        .then(t => {
          naturalWindOn = t.trim() === '1';
          refreshNaturalBtn();
        });
    }
    loadNaturalStatus();

    // 刷新按钮显示
    const naturalBtn = document.getElementById('naturalWindBtn');
    function refreshNaturalBtn() {
      naturalBtn.textContent = `自然风: ${naturalWindOn ? '已开启' : '已关闭'}`;
      naturalBtn.className = `btn ${naturalWindOn ? 'btn-success' : 'btn-outline-secondary'}`;
    }

    // 切换自然风，保持原后端逻辑
    naturalBtn.onclick = () => {
      fetch('/natural_wind').then(() => loadNaturalStatus());
    };
  </script>
</body>
</html>
)rawliteral";
    server.send(200, "text/html; charset=utf-8", html);
}


// 处理客户端断开连接
void handleHeartbeat() {
  lastHeartbeat = millis();
  clientConnected = true;
  server.send(200, "text/plain", "alive");
}

void handleWifiInfo() {
  String json = "{";
  json += "\"ssid\":\"" + WiFi.SSID() + "\",";
  json += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  json += "\"rssi\":" + String(WiFi.RSSI());
  json += "}";
  server.send(200, "application/json", json);
}


// 处理通道占空比设置请求
void handleChannelDutyCycle() {
    if (server.hasArg("ch1")) {
        int ch1Duty = server.arg("ch1").toInt();
        pwmDuty_A = constrain(ch1Duty, 0, 100);
        updatePWMOutputs(); // 使用统一的更新函数
    }
    if (server.hasArg("ch2")) {
        int ch2Duty = server.arg("ch2").toInt();
        pwmDuty_B = constrain(ch2Duty, 0, 100);
        updatePWMOutputs(); // 使用统一的更新函数
    }
    if (server.hasArg("ch3")) {
        int ch3Duty = server.arg("ch3").toInt();
        pwmDuty_C = constrain(ch3Duty, 0, 100);
        updatePWMOutputs(); // 使用统一的更新函数
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
    String response = "{\"ch1\": " + String(pwmDuty_A) + ", \"ch2\": " + String(pwmDuty_B) + ", \"ch3\": " + String(pwmDuty_C) + "}";
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
    server.on("/natural_wind_status", handleNaturalWindStatus); // 状态查询
    server.on("/heartbeat", handleHeartbeat);
    server.on("/wifi_info", handleWifiInfo);
    server.begin();
    Serial.println("HTTP 服务器已启动");
}