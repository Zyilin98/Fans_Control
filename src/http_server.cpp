#include "http_server.h"
#include "config.h"
#include "pwm.h"
#include "encoder.h"

WebServer server(80);

// 简化的JSON构建辅助函数
String buildJson(String keys[], String values[], int count) {
    String json = "{";
    for (int i = 0; i < count; i++) {
        if (i > 0) json += ",";
        json += "\"" + keys[i] + "\":" + values[i];
    }
    json += "}";
    return json;
}

// 简化的HTML页面 - 移除了内联CSS和冗余的JavaScript
void handleRoot() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/css/bootstrap.min.css" rel="stylesheet">
  <title>ESP32C3 Fanscontrol</title>
  <style>body{background:#f8f9fa}.status-badge{position:fixed;top:1rem;right:1rem;padding:.5rem 1rem;border-radius:.25rem;color:#fff}</style>
</head>
<body>
  <div class="container py-4">
    <h1 class="text-center mb-4">ESP32C3 Fanscontrol</h1>
    <span id="status" class="status-badge bg-secondary">连接状态: 未连接</span>
    
    <div class="row gy-4">
      <div class="col-md-6">
        <div class="card shadow-sm h-100">
          <div class="card-header">通道控制</div>
          <div class="card-body">
            <div class="mb-3"><label>通道1: <span id="d1">0</span>%</label><input type="range" class="form-range" id="c1" min="0" max="100"></div>
            <div class="mb-3"><label>通道2: <span id="d2">0</span>%</label><input type="range" class="form-range" id="c2" min="0" max="100"></div>
            <div class="mb-3"><label>通道3: <span id="d3">0</span>%</label><input type="range" class="form-range" id="c3" min="0" max="100"></div>
          </div>
        </div>
      </div>
       
       <div class="col-md-6 d-flex flex-column">
         <div class="card shadow-sm mb-3 flex-grow-1">
            <div class="card-header">实时数据</div>
            <div class="card-body">
              <p>通道1: <strong><span id="r1">0</span> RPM</strong></p>
              <p>通道2: <strong><span id="r2">0</span> RPM</strong></p>
              <p>电压: <strong><span id="v">0.00</span> V</strong></p>
            </div>
          </div>
          
          <div class="row" style="flex: 1;">
            <div class="col-6"><div class="card shadow-sm h-100"><div class="card-header">自然风模式</div><div class="card-body d-flex align-items-center justify-content-center"><button id="nw" class="btn btn-sm btn-outline-secondary">加载中...</button></div></div></div>
            <div class="col-6"><div class="card shadow-sm h-100"><div class="card-header">屏幕状态</div><div class="card-body d-flex align-items-center justify-content-center"><button id="wd" class="btn btn-sm btn-outline-secondary">点亮屏幕</button></div></div></div>
          </div>
      </div>
    </div>
    
    <div class="card shadow-sm mt-4">
      <div class="card-header">WiFi信息</div>
      <div class="card-body d-flex justify-content-between">
        <div>SSID: <span id="w1">-</span></div>
        <div>IP: <span id="w2">-</span></div>
        <div>RSSI: <span id="w3">-</span>dBm</div>
      </div>
    </div>
  </div>
  
  <script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.0/dist/js/bootstrap.bundle.min.js"></script>
  <script>
    const $ = id => document.getElementById(id);
    const channels = [1,2,3];
    let dragging = [false,false,false];
    let naturalOn = false;
    
    // 通用获取函数
    const get = (url, fn) => fetch(url).then(r => r.json()).then(fn);
    const post = (url, params) => fetch(url + '?' + new URLSearchParams(params));
    
    // 状态更新
    setInterval(() => fetch('/heartbeat').then(() => {
      $('status').textContent = '连接状态: 已连接';
      $('status').className = 'status-badge bg-success';
    }).catch(() => {
      $('status').textContent = '连接状态: 未连接';
      $('status').className = 'status-badge bg-danger';
    }), 1000);
    
    // WiFi信息
    setInterval(() => get('/wifi_info', d => {
      $('w1').textContent = d.ssid;
      $('w2').textContent = d.ip;
      $('w3').textContent = d.rssi;
    }), 5000);
    
    // 占空比和实时数据
    setInterval(() => {
      get('/duty_cycle', d => channels.forEach(i => {
        if(!dragging[i-1]) $('c'+i).value = d['ch'+i];
        $('d'+i).textContent = d['ch'+i];
      }));
      get('/speed', d => {
        $('r1').textContent = d.ch1.toFixed(0);
        $('r2').textContent = d.ch2.toFixed(0);
      });
      get('/voltage', d => $('v').textContent = d.voltage.toFixed(2));
    }, 1000);
    
    // 滑块事件
    channels.forEach(i => {
      const s = $('c'+i);
      s.oninput = e => $('d'+i).textContent = e.target.value;
      s.onmousedown = s.ontouchstart = () => dragging[i-1] = true;
      s.onmouseup = s.ontouchend = () => {
        dragging[i-1] = false;
        post('/channel_duty_cycle', {['ch'+i]: s.value});
      };
    });
    
    // 自然风
    const loadNatural = () => fetch('/natural_wind_status').then(r => r.text()).then(t => {
      naturalOn = t.trim() === '1';
      $('nw').textContent = naturalOn ? '已开启' : '已关闭';
      $('nw').className = 'btn btn-sm ' + (naturalOn ? 'btn-success' : 'btn-outline-secondary');
    });
    $('nw').onclick = () => fetch('/natural_wind').then(loadNatural);
    loadNatural();
    
    // 点亮屏幕
    $('wd').onclick = () => fetch('/wake_display');
  </script>
</body>
</html>
)rawliteral";
    server.send(200, "text/html; charset=utf-8", html);
}

// 简化的处理函数
void handleHeartbeat() {
    server.send(200, "text/plain", "alive");
}

void handleWifiInfo() {
    String keys[] = {"ssid", "ip", "rssi"};
    String values[] = {"\"" + WiFi.SSID() + "\"", 
                      "\"" + WiFi.localIP().toString() + "\"", 
                      String(WiFi.RSSI())};
    server.send(200, "application/json", buildJson(keys, values, 3));
}

void handleChannelDutyCycle() {
    if (server.hasArg("ch1")) {
        pwmDuty_A = constrain(server.arg("ch1").toInt(), 0, 100);
        updatePWMOutputs();
    }
    if (server.hasArg("ch2")) {
        pwmDuty_B = constrain(server.arg("ch2").toInt(), 0, 100);
        updatePWMOutputs();
    }
    if (server.hasArg("ch3")) {
        pwmDuty_C = constrain(server.arg("ch3").toInt(), 0, 100);
        updatePWMOutputs();
    }
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
}

void handleSpeed() {
    String keys[] = {"ch1", "ch2"};
    String values[] = {String(currentRPM_A), String(currentRPM_B)};
    server.send(200, "application/json", buildJson(keys, values, 2));
}

void handleVoltage() {
    String response = "{\"voltage\":" + String(measuredVoltage, 2) + "}";
    server.send(200, "application/json", response);
}

void handleDutyCycle() {
    String keys[] = {"ch1", "ch2", "ch3"};
    String values[] = {String(pwmDuty_A), String(pwmDuty_B), String(pwmDuty_C)};
    server.send(200, "application/json", buildJson(keys, values, 3));
}

void handleNaturalWind() {
    Naturewind = !Naturewind;
    server.sendHeader("Location", "/", true);
    server.send(302, "text/plain", "");
}

void handleNaturalWindStatus() {
    server.send(200, "text/plain", String(Naturewind));
}

void handleWakeDisplay() {
    handleRotate(1);
    delay(5);
    handleRotate(-1);
    server.send(200, "text/plain", "OK");
}

// 简化的服务器初始化
void setupServer() {
    server.on("/", handleRoot);
    server.on("/duty_cycle", handleDutyCycle);
    server.on("/channel_duty_cycle", handleChannelDutyCycle);
    server.on("/speed", handleSpeed);
    server.on("/voltage", handleVoltage);
    server.on("/natural_wind", handleNaturalWind);
    server.on("/natural_wind_status", handleNaturalWindStatus);
    server.on("/heartbeat", handleHeartbeat);
    server.on("/wifi_info", handleWifiInfo);
    server.on("/wake_display", handleWakeDisplay);
    server.begin();
    Serial.println("HTTP服务器已启动");
}