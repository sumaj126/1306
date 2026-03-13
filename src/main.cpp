/**
 * ESP32 + DHT20 + OLED 0.96寸(SSD1306) + HC-SR501 人体感应 温湿度时间显示项目 + Web服务器
 * 使用U8g2字体库，支持更多字体和语言
 * 功能：
 * 1. 从NTP服务器获取网络时间
 * 2. 从DHT20读取温度和湿度
 * 3. HC-SR501人体感应：人来亮屏，人走1分钟后熄屏
 * 4. 在OLED屏幕上居中显示时间、温度和湿度（使用U8g2精美字体）
 * 5. 启动Web服务器，手机可通过浏览器访问ESP32查看温度、湿度和时间
 *
 * 硬件连接：
 * - OLED (I2C): VCC->3.3V, GND->GND, SCL->GPIO22, SDA->GPIO21
 * - DHT20 (I2C): VCC->3.3V, GND->GND, SCL->GPIO22, SDA->GPIO21 (与OLED共用I2C总线)
 * - HC-SR501 PIR传感器: VCC->5V, GND->GND, OUT->GPIO13
 *
 * 使用方法：
 * 1. 连接WiFi后，OLED会显示ESP32的IP地址
 * 2. 在手机浏览器输入该IP地址即可查看温度和湿度
 * 3. 访问 http://IP地址/temperature 可获取纯文本温度数据
 * 4. 访问 http://IP地址/humidity 可获取纯文本湿度数据
 * 5. 访问 http://IP地址/json 可获取JSON格式数据
 * 6. 人体靠近时OLED自动亮屏，离开1分钟后自动熄屏
 */

// ==================== 头文件包含 ====================
#include <U8g2lib.h>                   // U8g2字体库,提供丰富的字体支持
#include <Wire.h>                      // I2C通信库,用于OLED和DHT20显示屏
#include <Adafruit_AHTX0.h>            // AHT20温湿度传感器库（支持DHT20）
#include <WiFi.h>                      // ESP32 WiFi功能库
#include <WebServer.h>                 // ESP32 Web服务器库,用于创建HTTP服务器
#include <PubSubClient.h>               // MQTT客户端库
#include <time.h>                      // C标准时间库,用于时间处理
#include <esp_task_wdt.h>              // ESP32看门狗库
#include <esp_system.h>                // ESP32系统信息库

// ==================== 核心安全配置 ====================
#define SERIAL_TIMEOUT_MS 50          // 串口写入超时
#define I2C_TIMEOUT_MS 2000          // I2C操作超时
#define WIFI_TIMEOUT_MS 30000         // WiFi操作超时
#define HTTP_TIMEOUT_MS 5000          // HTTP响应超时
#define MAX_SERIAL_WAIT 1000          // 最大串口等待时间

// ==================== OLED显示屏配置 ====================
// 使用SSD1306驱动，I2C协议，完整帧缓冲模式
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ==================== AHT20传感器配置 ====================
// AHT20使用独立的I2C引脚
#define AHT20_SDA 4     // AHT20 SDA引脚
#define AHT20_SCL 5     // AHT20 SCL引脚
Adafruit_AHTX0 aht;    // 创建AHT20对象
TwoWire ahtWire = TwoWire(1);  // 创建第二个I2C实例用于AHT20

// 创建Web服务器对象，监听80端口（HTTP默认端口）
WebServer server(80);

// ==================== MQTT配置 ====================
// 树莓派MQTT代理配置
const char* mqtt_server = "192.168.1.10";  // 树莓派IP地址
const int mqtt_port = 1883;                  // MQTT默认端口
const char* mqtt_username = "homeassistant"; // MQTT用户名
const char* mqtt_password = "homeassistant"; // MQTT密码
const char* mqtt_client_id = "esp32-1306-monitor"; // MQTT客户端ID

// 创建WiFi客户端和MQTT客户端
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// ==================== 预分配缓冲区（避免内存碎片）====================
#define HTML_BUFFER_SIZE 4096        // HTML响应缓冲区大小
#define JSON_BUFFER_SIZE 512         // JSON响应缓冲区大小
char htmlBuffer[HTML_BUFFER_SIZE];   // 预分配HTML缓冲区
char jsonBuffer[JSON_BUFFER_SIZE];   // 预分配JSON缓冲区

// ==================== WiFi配置 ====================
// 注意：请修改为您的WiFi网络名称和密码
const char* ssid = "jiajia";        // WiFi名称（SSID）
const char* password = "9812061104"; // WiFi密码

// ==================== ESP32静态IP配置（已启用）====================
// ESP32使用固定IP地址192.168.1.200，路由器已配置端口映射
// 外网访问地址：http://sumaj.synology.me:7788
IPAddress local_IP(192, 168, 1, 200);      // ESP32的固定IP地址
IPAddress gateway(192, 168, 1, 1);          // 路由器IP地址（网关）
IPAddress subnet(255, 255, 255, 0);        // 子网掩码
IPAddress primaryDNS(192, 168, 1, 1);      // DNS服务器1（路由器IP）
IPAddress secondaryDNS(8, 8, 8, 8);         // DNS服务器2（Google DNS）

// ==================== NTP时间服务器配置 ====================
const char* ntpServer = "cn.pool.ntp.org";           // NTP服务器地址，使用国内服务器速度更快
const long gmtOffset_sec = 8 * 3600;              // 时区偏移（秒），8小时表示东八区（北京时间）
const int daylightOffset_sec = 0;                 // 夏令时偏移（秒），中国不使用夏令时设为0

// ==================== 传感器校准参数 ====================
// DHT20传感器校准值（通过与其他温度计对比测得）
const float tempOffset = -1.0;       // 温度校准偏移值：测量值偏高的度数（负值表示减去）
const float humOffset = +3.0;        // 湿度校准偏移值：测量值偏低的百分比（正值表示加上）

// ==================== 全局变量 ====================
float currentTemperature = 0.0;          // 存储当前温度值（供Web服务器使用）
float currentHumidity = 0.0;             // 存储当前湿度值（供Web服务器使用）
char currentTime[32] = "";                // 存储当前时间字符串
char currentDate[32] = "";               // 存储当前日期字符串
bool firstDataReady = false;             // 标记是否已获取到第一组数据
struct tm lastValidTime;             // 存储最后一次有效时间（用于NTP失败时fallback）
bool hasValidTime = false;           // 标记是否有有效时间

// ==================== HC-SR501人体红外感应配置 ====================
#define PIR_SENSOR_PIN 13              // PIR传感器连接的GPIO引脚
unsigned long lastMotionTime = 0;      // 上次检测到人体活动的时间
const unsigned long screenOffDelay = 60000;  // 熄屏延迟时间（60秒=1分钟）
bool screenOn = true;                 // 屏幕状态：true=亮屏，false=熄屏
bool lastPirState = false;            // 上次PIR传感器状态

// ==================== 传感器刷新控制 ====================
int sensorUpdateCounter = 0;             // 传感器更新计数器
const int sensorUpdateInterval = 5;       // 传感器更新间隔（5次loop=5秒）

// ==================== 安全串口输出函数 ====================
/**
 * 安全的串口输出函数，避免USB断开时阻塞
 * 检查缓冲区空间，只有足够空间才输出
 */
void safeSerialPrint(const char* str) {
  if(Serial.availableForWrite() > strlen(str)) {
    Serial.print(str);
  }
}



void safeSerialPrintln(const char* str) {
  if(Serial.availableForWrite() > strlen(str) + 2) {
    Serial.println(str);
  }
}

// 将esp_reset_reason_t枚举转换为可读字符串
const char* resetReasonToString(esp_reset_reason_t reason) {
  switch(reason) {
    case ESP_RST_UNKNOWN:  return "Unknown";
    case ESP_RST_POWERON:  return "Power On";
    case ESP_RST_EXT:      return "External Pin";
    case ESP_RST_SW:       return "Software Reset";
    case ESP_RST_PANIC:    return "Exception/Panic";
    case ESP_RST_INT_WDT:  return "Interrupt Watchdog";
    case ESP_RST_TASK_WDT: return "Task Watchdog";
    case ESP_RST_WDT:      return "Other Watchdogs";
    case ESP_RST_DEEPSLEEP:return "Deep Sleep Wake";
    case ESP_RST_BROWNOUT: return "Brownout";
    case ESP_RST_SDIO:     return "SDIO";
    default:               return "Unknown Reason";
  }
}

// ==================== MQTT函数 ====================

// 函数声明
void sendMQTTDiscovery();

/**
 * 连接到MQTT代理服务器
 */
void connectMQTT() {
  // 等待WiFi连接
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  // 设置MQTT服务器
  mqttClient.setServer(mqtt_server, mqtt_port);
  
  // 增加缓冲区大小到512字节，容纳发现消息
  mqttClient.setBufferSize(512);
  Serial.println("Buffer size set to 512 bytes");
  
  // 尝试连接MQTT（使用匿名连接）
  Serial.print("Connecting to MQTT...");
  if (mqttClient.connect(mqtt_client_id)) {
    Serial.println(" connected!");
    
    // 发布设备在线状态
    Serial.println("Publishing availability message...");
    if (mqttClient.publish("esp32-1306/availability", "online", true)) {
      Serial.println("Availability message published successfully");
    } else {
      Serial.println("Failed to publish availability message");
    }
    
    // 发送MQTT发现消息
    Serial.println("Sending MQTT discovery messages...");
    sendMQTTDiscovery();
  } else {
    Serial.print(" failed, rc=");
    Serial.println(mqttClient.state());
  }
}

/**
 * 发送MQTT发现消息，让Home Assistant自动发现设备
 */
void sendMQTTDiscovery() {
  Serial.println("=== Sending MQTT discovery messages ===");
  
  // 温度传感器发现
  String tempDiscovery = "{";
  tempDiscovery += "\"name\": \"ESP32-1306 Temperature\",";
  tempDiscovery += "\"uniq_id\": \"esp32_1306_temperature\",";
  tempDiscovery += "\"device_class\": \"temperature\",";
  tempDiscovery += "\"unit_of_measurement\": \"°C\",";
  tempDiscovery += "\"state_topic\": \"esp32-1306/temperature\",";
  tempDiscovery += "\"availability_topic\": \"esp32-1306/availability\",";
  tempDiscovery += "\"payload_available\": \"online\",";
  tempDiscovery += "\"payload_not_available\": \"offline\",";
  tempDiscovery += "\"device\": {";
  tempDiscovery += "\"identifiers\": [\"esp32-1306-monitor\"],";
  tempDiscovery += "\"name\": \"ESP32-1306 Monitor\",";
  tempDiscovery += "\"model\": \"ESP32\",";
  tempDiscovery += "\"manufacturer\": \"ESP32\"";
  tempDiscovery += "}";
  tempDiscovery += "}";
  Serial.print("Temperature discovery: ");
  Serial.println(tempDiscovery);
  if (mqttClient.publish("homeassistant/sensor/esp32_1306_temperature/config", tempDiscovery.c_str(), true)) {
    Serial.println("Temperature discovery message published successfully");
  } else {
    Serial.println("Failed to publish temperature discovery message");
  }
  
  // 湿度传感器发现
  String humDiscovery = "{";
  humDiscovery += "\"name\": \"ESP32-1306 Humidity\",";
  humDiscovery += "\"uniq_id\": \"esp32_1306_humidity\",";
  humDiscovery += "\"device_class\": \"humidity\",";
  humDiscovery += "\"unit_of_measurement\": \"%\",";
  humDiscovery += "\"state_topic\": \"esp32-1306/humidity\",";
  humDiscovery += "\"availability_topic\": \"esp32-1306/availability\",";
  humDiscovery += "\"payload_available\": \"online\",";
  humDiscovery += "\"payload_not_available\": \"offline\",";
  humDiscovery += "\"device\": {";
  humDiscovery += "\"identifiers\": [\"esp32-1306-monitor\"],";
  humDiscovery += "\"name\": \"ESP32-1306 Monitor\",";
  humDiscovery += "\"model\": \"ESP32\",";
  humDiscovery += "\"manufacturer\": \"ESP32\"";
  humDiscovery += "}";
  humDiscovery += "}";
  Serial.print("Humidity discovery: ");
  Serial.println(humDiscovery);
  if (mqttClient.publish("homeassistant/sensor/esp32_1306_humidity/config", humDiscovery.c_str(), true)) {
    Serial.println("Humidity discovery message published successfully");
  } else {
    Serial.println("Failed to publish humidity discovery message");
  }
  
  // 人体感应传感器发现
  String motionDiscovery = "{";
  motionDiscovery += "\"name\": \"ESP32-1306 Motion\",";
  motionDiscovery += "\"uniq_id\": \"esp32_1306_motion\",";
  motionDiscovery += "\"device_class\": \"motion\",";
  motionDiscovery += "\"state_topic\": \"esp32-1306/motion\",";
  motionDiscovery += "\"availability_topic\": \"esp32-1306/availability\",";
  motionDiscovery += "\"payload_available\": \"online\",";
  motionDiscovery += "\"payload_not_available\": \"offline\",";
  motionDiscovery += "\"payload_on\": \"ON\",";
  motionDiscovery += "\"payload_off\": \"OFF\",";
  motionDiscovery += "\"device\": {";
  motionDiscovery += "\"identifiers\": [\"esp32-1306-monitor\"],";
  motionDiscovery += "\"name\": \"ESP32-1306 Monitor\",";
  motionDiscovery += "\"model\": \"ESP32\",";
  motionDiscovery += "\"manufacturer\": \"ESP32\"";
  motionDiscovery += "}";
  motionDiscovery += "}";
  Serial.print("Motion discovery: ");
  Serial.println(motionDiscovery);
  if (mqttClient.publish("homeassistant/binary_sensor/esp32_1306_motion/config", motionDiscovery.c_str(), true)) {
    Serial.println("Motion discovery message published successfully");
  } else {
    Serial.println("Failed to publish motion discovery message");
  }
  
  Serial.println("=== MQTT discovery messages sent ===");
}

/**
 * 发布传感器数据到MQTT
 */
void publishSensorData() {
  // 发布温度数据
  char tempBuffer[10];
  snprintf(tempBuffer, sizeof(tempBuffer), "%.1f", currentTemperature);
  mqttClient.publish("esp32-1306/temperature", tempBuffer);
  
  // 发布湿度数据
  char humBuffer[10];
  snprintf(humBuffer, sizeof(humBuffer), "%.1f", currentHumidity);
  mqttClient.publish("esp32-1306/humidity", humBuffer);
  
  // 发布人体感应数据
  int pirState = digitalRead(PIR_SENSOR_PIN);
  mqttClient.publish("esp32-1306/motion", pirState == HIGH ? "ON" : "OFF");
  
  // 发布设备在线状态
  mqttClient.publish("esp32-1306/availability", "online", true);
}

// ====================硬件级I2C超时保护 ====================
/**
 * 带超时的I2C读取函数，防止I2C死锁
 * 直接调用I2C函数，使用外层while循环控制超时
 */

// ==================== 系统保护变量 ====================
unsigned long lastWiFiCheck = 0;         // 上次检查WiFi的时间
unsigned long lastNTPCheck = 0;          // 上次检查NTP的时间
const unsigned long wifiCheckInterval = 30000;  // WiFi检查间隔（30秒）
const unsigned long ntpCheckInterval = 86400000;  // NTP检查间隔（24小时=一天）
int reconnectCount = 0;                  // WiFi重连次数
const int maxReconnectCount = 5;         // 最大重连次数后重启

// ==================== 重启计数器（用于检测重启循环）====================
RTC_DATA_ATTR int bootCount = 0;        // 存储在RTC内存，重启后保留
const int MAX_BOOT_COUNT = 10;           // 最大重启次数，超过则进入安全模式

// ==================== WiFi重连函数 ====================
/**
 * 检查并恢复WiFi连接
 * 如果WiFi断开,尝试重新连接
 * 重连失败超过maxReconnectCount次则重启ESP32
 */
void checkWiFiConnection() {
  unsigned long currentMillis = millis();
  
  // 每隔30秒检查一次WiFi状态
  if(currentMillis - lastWiFiCheck >= wifiCheckInterval) {
    lastWiFiCheck = currentMillis;
    
    // 检查WiFi是否连接
    if(WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected! Attempting to reconnect...");
      
      // OLED显示重连状态
      display.clearBuffer();
      display.setFont(u8g2_font_ncenB08_tr);
      display.drawStr(0, 15, "WiFi Lost!");
      String retryStr = "Retry: " + String(reconnectCount + 1);
      display.drawStr(0, 30, retryStr.c_str());
      display.sendBuffer();
      
      // 尝试重新连接
      WiFi.disconnect();
      WiFi.begin(ssid, password);
      
      // 等待连接（最多10秒），期间要喂狗
      int retryTimeout = 10;
      while(WiFi.status() != WL_CONNECTED && retryTimeout > 0) {
        esp_task_wdt_reset();  // 喂狗
        server.handleClient();  // 处理HTTP请求
        delay(100);
        retryTimeout--;
        if(retryTimeout % 10 == 0) Serial.print(".");
      }
      
      if(WiFi.status() == WL_CONNECTED) {
        // 重连成功
        Serial.println("\nWiFi reconnected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
        reconnectCount = 0;  // 重置重连计数
        
        // 重新配置静态IP，添加超时保护
        esp_task_wdt_reset();
        unsigned long configTimeout = millis();
        bool configSuccess = false;
        while(millis() - configTimeout < 3000 && !configSuccess) {
          esp_task_wdt_reset();  // 持续喂狗
          configSuccess = WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
          if(!configSuccess) delay(100);
        }
        
        if(!configSuccess) {
          Serial.println("Static IP configuration timeout, using current IP");
        } else {
          Serial.println("Static IP reconfigured successfully");
        }
      } else {
        // 重连失败
        Serial.println("\nWiFi reconnect failed!");
        reconnectCount++;
        
        // 超过最大重连次数,重启ESP32
        if(reconnectCount >= maxReconnectCount) {
          Serial.println("Max reconnect attempts reached. Restarting ESP32...");
          display.clearBuffer();
          display.setFont(u8g2_font_ncenB08_tr);
          display.drawStr(0, 15, "WiFi Failed!");
          display.drawStr(0, 30, "Restarting...");
          display.sendBuffer();
          
          // 重启前持续喂狗
          unsigned long restartDelayStart = millis();
          while(millis() - restartDelayStart < 2000) {
            esp_task_wdt_reset();
            delay(100);
          }
          ESP.restart();  // 重启ESP32
        }
      }
    } else {
      // WiFi正常,重置重连计数
      reconnectCount = 0;
    }
  }
}

// ==================== NTP时间同步函数 ====================
/**
 * 检查并同步NTP时间
 * 每24小时同步一次时间（掉电后重启才需要校准）
 */
void checkNTPSync() {
  unsigned long currentMillis = millis();

  // 每隔24小时检查一次NTP同步
  if(currentMillis - lastNTPCheck >= ntpCheckInterval) {
    lastNTPCheck = currentMillis;

    // 重新配置时间同步（清除NTP缓存，强制重新获取）
    configTime(0, 0, "pool.ntp.org");  // 临时重置
    delay(100);
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // 重新设置

    struct tm timeinfo;
    if(getLocalTime(&timeinfo)) {
      Serial.println("NTP time sync successful");
      Serial.print("NTP synced: ");
      Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
      // 同步成功后更新有效时间
      memcpy(&lastValidTime, &timeinfo, sizeof(struct tm));
      hasValidTime = true;
    } else {
      Serial.println("NTP time sync failed, using cached time");
    }
  }
}

// ==================== 内存监控函数 ====================
/**
 * 监控ESP32剩余内存
 * 如果内存不足,输出警告信息
 */
void checkMemory() {
  unsigned long freeHeap = ESP.getFreeHeap();
  unsigned long minFreeHeap = ESP.getMinFreeHeap();

  if(freeHeap < 20000) {  // 如果剩余内存小于20KB（降低阈值）
    Serial.print("WARNING: Low memory! Free: ");
    Serial.print(freeHeap);
    Serial.print(" bytes, Min: ");
    Serial.print(minFreeHeap);
    Serial.println(" bytes");

    // OLED显示内存警告（不阻塞）
    display.clearBuffer();
    display.setFont(u8g2_font_ncenB08_tr);
    display.drawStr(0, 15, "Low Memory!");
    char memStr[32];
    snprintf(memStr, sizeof(memStr), "Free: %dKB", freeHeap / 1024);
    display.drawStr(0, 30, memStr);
    display.sendBuffer();
    // 移除delay，避免阻塞
  }
}

// ==================== 居中显示文本函数（U8g2版本）====================
void printCentered(const char* text, int16_t y, const uint8_t* font) {
  display.setFont(font);                                   // 设置字体
  int16_t textWidth = display.getUTF8Width(text);         // 获取文本宽度（支持中文）
  int16_t x = (128 - textWidth) / 2;                      // 计算居中的x坐标
  display.drawStr(x, y, text);                           // 使用drawStr显示文本
}

/**
 * Web服务器 - 主页处理函数
 * 访问 http://ESP32_IP/ 时调用此函数
 * 返回一个美观的HTML页面，显示温度、湿度和时间信息
 */
void handleRoot() {
  // 添加CORS响应头，允许跨域访问（用于群晖反向代理）
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  // 使用预分配缓冲区，避免内存碎片
  int len = snprintf(htmlBuffer, HTML_BUFFER_SIZE,
    "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
    "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
    "<title>客厅温湿度监控</title><style>"
    "* { margin: 0; padding: 0; box-sizing: border-box; }"
    "body { font-family: 'Microsoft YaHei', Arial, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%%); min-height: 100vh; display: flex; align-items: center; justify-content: center; padding: 20px; }"
    ".container { background: white; border-radius: 20px; padding: 40px; box-shadow: 0 10px 40px rgba(0,0,0,0.1); max-width: 500px; width: 100%%; }"
    ".header { text-align: center; margin-bottom: 30px; padding-bottom: 20px; border-bottom: 2px solid #f0f0f0; }"
    ".title { font-size: 28px; color: #333; margin-bottom: 10px; font-weight: bold; }"
    ".subtitle { font-size: 14px; color: #999; }"
    ".time-display { text-align: center; font-size: 48px; font-weight: bold; color: #667eea; margin-bottom: 30px; font-family: 'Courier New', monospace; }"
    ".data-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; margin-bottom: 20px; }"
    ".data-card { border-radius: 15px; padding: 25px; text-align: center; color: #333; }"
    ".data-label { font-size: 16px; opacity: 0.9; margin-bottom: 10px; }"
    ".data-value { font-size: 42px; font-weight: bold; }"
    ".status-bar { background: #f8f9fa; border-radius: 10px; padding: 15px; text-align: center; font-size: 14px; color: #666; }"
    ".icon { font-size: 32px; margin-bottom: 10px; }"
    "@media (max-width: 480px) { .container { padding: 20px; } .title { font-size: 24px; } .time-display { font-size: 36px; } .data-card { padding: 15px; text-align: center; } .data-value { font-size: 32px; text-align: center; } }"
    "</style><script>"
    "function updateTime() {"
    "  const now = new Date();"
    "  const hours = String(now.getHours()).padStart(2, '0');"
    "  const minutes = String(now.getMinutes()).padStart(2, '0');"
    "  const seconds = String(now.getSeconds()).padStart(2, '0');"
    "  document.getElementById('time').textContent = hours + ':' + minutes + ':' + seconds;"
    "}"
    "const temperature = %.1f;"
    "let tempColor = temperature < 20 ? '#3498db' : (temperature >= 20 && temperature < 30 ? 'rgb(241,196,15)' : '#e74c3c');"
    "const humColor = '#28a745';"
    "document.addEventListener('DOMContentLoaded', function() {"
    "  document.getElementById('temp-value').style.color = tempColor;"
    "  document.getElementById('hum-value').style.color = humColor;"
    "});"
    "setInterval(updateTime, 1000);"
    "setInterval(() => location.reload(), 10000);"
    "window.onload = updateTime;"
    "</script></head><body><div class=\"container\">"
    "<div class=\"header\">"
    "<div class=\"icon\">🏠</div>"
    "<div class=\"title\">客厅温湿度监控</div>"
    "<div class=\"subtitle\">Living Room Monitor</div>"
    "</div>"
    "<div class=\"time-display\" id=\"time\">%s</div>"
    "<div class=\"data-grid\">"
    "<div class=\"data-card\">"
    "<div class=\"data-label\">🌡️ 温度</div>"
    "<div class=\"data-value\" id=\"temp-value\">%.1f°C</div>"
    "</div>"
    "<div class=\"data-card\">"
    "<div class=\"data-label\">💧 湿度</div>"
    "<div class=\"data-value\" id=\"hum-value\">%.1f%%</div>"
    "</div>"
    "</div>"
    "<div class=\"status-bar\">"
    "<span>📡 在线</span>"
    "<span style=\"margin: 0 10px;\">|</span>"
    "<span>页面每10秒自动刷新</span>"
    "</div>"
    "</div></body></html>",
    currentTemperature, currentTime, currentTemperature, currentHumidity
  );
  
  if(len > 0 && len < HTML_BUFFER_SIZE) {
    server.send(200, "text/html", htmlBuffer);
  } else {
    server.send(500, "text/plain", "HTML generation error");
  }
}

/**
 * Web服务器 - 温度API处理函数
 * 访问 http://ESP32_IP/temperature 时调用此函数
 * 返回纯文本格式的温度数据，方便其他程序读取
 */
void handleTemperature() {
  // 添加CORS响应头，允许跨域访问（用于群晖反向代理）
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  char tempText[32];
  snprintf(tempText, sizeof(tempText), "%.1f°C", currentTemperature);
  server.send(200, "text/plain", tempText);                // 发送纯文本响应
}

/**
 * Web服务器 - 湿度API处理函数
 * 访问 http://ESP32_IP/humidity 时调用此函数
 * 返回纯文本格式的湿度数据，方便其他程序读取
 */
void handleHumidity() {
  // 添加CORS响应头，允许跨域访问（用于群晖反向代理）
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  char humText[32];
  snprintf(humText, sizeof(humText), "%.1f%%", currentHumidity);
  server.send(200, "text/plain", humText);                 // 发送纯文本响应
}

/**
 * Web服务器 - JSON API处理函数
 * 访问 http://ESP32_IP/json 时调用此函数
 * 返回JSON格式的数据，包含温度、湿度、时间和日期
 */
void handleJson() {
  // 添加CORS响应头，允许跨域访问（用于群晖反向代理）
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  int len = snprintf(jsonBuffer, JSON_BUFFER_SIZE,
    "{\"temperature\": %.1f,\"humidity\": %.1f,\"time\": \"%s\",\"date\": \"%s\",\"status\": \"ok\"}",
    currentTemperature, currentHumidity, currentTime, currentDate
  );
  
  if(len > 0 && len < JSON_BUFFER_SIZE) {
    server.send(200, "application/json", jsonBuffer);
  } else {
    server.send(500, "text/plain", "JSON generation error");
  }
}

/**
 * Web服务器 - 404错误处理函数
 * 当访问不存在的路径时调用此函数
 */
void handleNotFound() {
  char notFoundBuffer[512];
  int len = snprintf(notFoundBuffer, sizeof(notFoundBuffer),
    "404 Not Found\n\nURI: %s\nMethod: %s\nArguments: %d",
    server.uri().c_str(),
    (server.method() == HTTP_GET) ? "GET" : "POST",
    server.args()
  );
  
  server.send(404, "text/plain", notFoundBuffer);
}

// ==================== PIR传感器控制函数 ====================
/**
 * 读取PIR传感器状态并控制OLED屏幕开关
 * 人来亮屏，人走1分钟后熄屏
 */
void checkPIRSensor() {
  static unsigned long lastPIRCheck = 0;  // 上次检查PIR的时间
  unsigned long currentMillis = millis();

  // 每秒检查一次PIR（避免过于频繁）
  if(currentMillis - lastPIRCheck >= 1000) {
    lastPIRCheck = currentMillis;

    // 读取PIR传感器状态（HIGH=有人，LOW=无人）
    int pirState = digitalRead(PIR_SENSOR_PIN);
    bool currentPirState = (pirState == HIGH);

    // 检测到人体活动
    if(currentPirState) {
      lastMotionTime = currentMillis;  // 更新最后活动时间
      if(!screenOn) {
        // 屏幕当前是熄灭状态，需要点亮
        screenOn = true;
        Serial.println("=== PIR: Motion detected! Screen ON ===");
      }
    }

    // 检查是否需要熄屏（人离开超过1分钟）
    if(screenOn && (currentMillis - lastMotionTime >= screenOffDelay)) {
      screenOn = false;
      Serial.println("=== PIR: No motion for 1 minute. Screen OFF ===");
    }

    // 记录当前PIR状态用于下次比较
    lastPirState = currentPirState;

    // 每分钟输出一次PIR状态（调试用）
    static unsigned long lastPirDebug = 0;
    if(currentMillis - lastPirDebug >= 60000) {
      lastPirDebug = currentMillis;
      Serial.print("PIR Status: ");
      Serial.print(currentPirState ? "HIGH (Motion)" : "LOW (No motion)");
      Serial.print(", Screen: ");
      Serial.println(screenOn ? "ON" : "OFF");
    }
  }
}

/**
 * setup() - 初始化函数
 * 程序启动时执行一次，用于初始化所有硬件和设置
 */
void setup() {
  // ==================== 立即启动看门狗（最高优先级）====================
  esp_task_wdt_init(30, true);                             // 30秒超时,panic模式(系统重启)
  esp_task_wdt_add(NULL);                                  // 注册当前任务到看门狗(必须!)
  
  // 初始化串口通信
  Serial.begin(115200);                                    // 设置串口波特率为115200
  delay(100);                                              // 等待串口稳定
  esp_task_wdt_reset();                                      // 喂狗
  
  // 输出启动信息
  safeSerialPrintln("\n========================================");
  safeSerialPrintln("ESP32 Temperature & Humidity Monitor");
  safeSerialPrintln("========================================");
  safeSerialPrint("Reset reason: ");
  safeSerialPrintln(resetReasonToString(esp_reset_reason()));
  safeSerialPrint("Free heap at startup: ");
  Serial.print(ESP.getFreeHeap());
  safeSerialPrintln(" bytes");
  
  // 检测重启循环
  bootCount++;
  safeSerialPrint("Boot count: ");
  Serial.println(bootCount);
  
  if(bootCount > MAX_BOOT_COUNT) {
    safeSerialPrintln("CRITICAL: Too many reboots! Entering safe mode...");
    safeSerialPrintln("Please check hardware connections and restart manually.");
    
    // 进入安全模式：禁用看门狗，停止所有操作
    esp_task_wdt_delete(NULL);  // 删除看门狗
    
    display.clearBuffer();
    display.setFont(u8g2_font_ncenB08_tr);
    display.drawStr(0, 15, "SAFE MODE");
    display.drawStr(0, 30, "Check HW!");
    display.sendBuffer();
    
    // 永久循环等待手动干预
    while(true) {
      delay(1000);
    }
  }
  
  esp_task_wdt_reset();                                      // 喂狗

  // 初始化PIR传感器
  pinMode(PIR_SENSOR_PIN, INPUT);                          // 设置PIR引脚为输入
  Serial.println("PIR sensor initialized on GPIO" + String(PIR_SENSOR_PIN));

  // 测试读取PIR传感器初始状态
  int initialPirState = digitalRead(PIR_SENSOR_PIN);
  Serial.print("PIR initial state: ");
  Serial.println(initialPirState == HIGH ? "HIGH" : "LOW");

  // 初始化lastMotionTime
  lastMotionTime = millis();
  Serial.println("PIR lastMotionTime initialized to " + String(lastMotionTime));

  // 初始化OLED显示屏（U8g2版本），添加I2C超时保护
  esp_task_wdt_reset();                                      // 喂狗
  unsigned long oledTimeout = millis();
  bool oledInitSuccess = false;
  
  // OLED初始化最多等待3秒
  while(millis() - oledTimeout < 3000 && !oledInitSuccess) {
    esp_task_wdt_reset();
    display.begin();  // 直接调用，不支持try-catch
    oledInitSuccess = true;
  }
  
  if(!oledInitSuccess) {
    Serial.println("WARNING: OLED init timeout, continuing without display");
  }
  
  display.clearBuffer();                                   // 清空显示缓冲区

  // 初始化温湿度传感器，添加I2C超时保护
  esp_task_wdt_reset();
  ahtWire.begin(AHT20_SDA, AHT20_SCL, 400000);  // 初始化第二个I2C总线
  
  unsigned long ahtTimeout = millis();
  bool ahtInitSuccess = false;
  
  // AHT20初始化最多等待3秒
  while(millis() - ahtTimeout < 3000 && !ahtInitSuccess) {
    esp_task_wdt_reset();
    if (aht.begin(&ahtWire, 0x38)) {               // 使用自定义Wire，地址0x38
      ahtInitSuccess = true;
    } else {
      delay(100);
    }
  }
  
  if(!ahtInitSuccess) {
    Serial.println("WARNING: AHT20 init timeout, continuing without sensor");
    display.clearBuffer();
    display.setFont(u8g2_font_ncenB08_tr);
    display.drawStr(0, 15, "Sensor Error!");
    display.drawStr(0, 30, "No AHT20");
    display.sendBuffer();
    esp_task_wdt_reset();
    delay(1000);  // 缩短延迟并喂狗
  } else {
    Serial.println("AHT20 initialized successfully");
    Serial.print("AHT20 I2C: SDA=GPIO");
    Serial.print(AHT20_SDA);
    Serial.print(", SCL=GPIO");
    Serial.println(AHT20_SCL);

    // AHT20传感器预热，确保首次读取准确
    Serial.println("AHT20 warming up...");
    display.clearBuffer();
    display.setFont(u8g2_font_ncenB08_tr);
    display.drawStr(0, 15, "Sensor Warming...");
    display.sendBuffer();
    esp_task_wdt_reset();
    delay(500);  // 预热0.5秒（缩短延迟）
  }

  // ==================== 连接WiFi网络 ====================
  // 添加WiFi连接超时保护（最多等待30秒）
  esp_task_wdt_reset();
  WiFi.begin(ssid, password);                             // 开始连接WiFi

  Serial.print("Connecting to WiFi");                      // 串口输出连接信息
  display.clearBuffer();                                   // 清空缓冲区
  display.setFont(u8g2_font_ncenB08_tr);                  // 设置小字体
  display.drawStr(0, 15, "Connecting WiFi...");           // 显示连接信息
  display.sendBuffer();                                    // 发送到OLED显示

  unsigned long wifiTimeout = millis();
  while(WiFi.status() != WL_CONNECTED) {                   // 循环等待WiFi连接成功
    esp_task_wdt_reset();                                  // 喂狗,防止看门狗超时
    delay(100);                                            // 缩短延迟为100ms
    if(millis() % 500 < 100) Serial.print(".");            // 每500ms打印一个点
    
    // 添加WiFi连接超时保护（30秒后放弃）
    if(millis() - wifiTimeout >= 30000) {
      Serial.println("\nWiFi connection timeout!");
      display.clearBuffer();
      display.setFont(u8g2_font_ncenB08_tr);
      display.drawStr(0, 15, "WiFi Timeout!");
      display.drawStr(0, 30, "Will retry later");
      display.sendBuffer();
      esp_task_wdt_reset();
      delay(2000);  // 显示2秒后继续（不要重启，让系统进入主循环）
      break;  // 跳出WiFi连接等待，让主循环中的checkWiFiConnection处理
    }
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println();                                        // 换行
    Serial.println("WiFi connected");                        // 输出连接成功信息
    Serial.print("IP Address: ");                           // 打印IP地址提示
    Serial.println(WiFi.localIP());                          // 打印ESP32的IP地址
    Serial.println("Open http://" + WiFi.localIP().toString() + " in your browser");  // 浏览器访问提示

    // 显示WiFi连接成功和IP地址
    display.clearBuffer();                                  // 清空缓冲区
    display.setFont(u8g2_font_ncenB08_tr);                   // 设置字体
    display.drawStr(0, 15, "WiFi Connected!");             // 显示连接成功
    String ipStr = "IP: " + WiFi.localIP().toString();      // 拼接IP地址字符串
    display.drawStr(0, 30, ipStr.c_str());                  // 显示IP地址
    display.sendBuffer();                                   // 发送到OLED
    esp_task_wdt_reset();  // 喂狗
    delay(1000);                                             // 显示1秒让用户看到IP地址
  }

  // 配置网络时间同步（NTP）
  // configTime用于配置ESP32的时间同步服务
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // 设置时区、夏令时和NTP服务器

  // 等待NTP时间同步成功（最多等待5秒）
  Serial.print("Syncing NTP time...");
  display.clearBuffer();
  display.setFont(u8g2_font_ncenB08_tr);
  display.drawStr(0, 15, "Syncing NTP...");
  display.sendBuffer();

  struct tm timeinfo;
  int syncAttempts = 0;
  const int maxSyncAttempts = 10;  // 最多尝试10次，每次延迟500ms，总共5秒

  while(!getLocalTime(&timeinfo) && syncAttempts < maxSyncAttempts) {
    esp_task_wdt_reset();  // 喂狗
    delay(100);
    if(syncAttempts % 5 == 0) Serial.print(".");  // 每500ms打印一个点
    syncAttempts++;
  }

  if(getLocalTime(&timeinfo)) {
    Serial.println("\nNTP time sync successful!");
    Serial.print("Current time: ");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
    // 保存初始有效时间
    memcpy(&lastValidTime, &timeinfo, sizeof(struct tm));
    hasValidTime = true;
  } else {
    Serial.println("\nNTP time sync failed, will retry in loop");
  }

  // ==================== 配置静态IP ====================
  // 添加超时保护
  esp_task_wdt_reset();
  unsigned long configTimeout = millis();
  bool configSuccess = false;
  while(millis() - configTimeout < 3000 && !configSuccess) {
    esp_task_wdt_reset();  // 持续喂狗
    configSuccess = WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
    if(!configSuccess) delay(100);
  }
  
  if (configSuccess) {
    Serial.println("Static IP configured successfully");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("External access: http://sumaj.synology.me:7788");
  } else {
    Serial.println("Static IP config timeout, using DHCP-assigned IP");
    Serial.print("Current IP: ");
    Serial.println(WiFi.localIP());
  }

  // ==================== 配置Web服务器 ====================
  server.on("/", handleRoot);                              // 注册根路径处理函数（主页）
  server.on("/temperature", handleTemperature);            // 注册温度API路径
  server.on("/humidity", handleHumidity);                  // 注册湿度API路径
  server.on("/json", handleJson);                          // 注册JSON API路径
  server.onNotFound(handleNotFound);                       // 注册404处理函数

  server.begin();                                           // 启动Web服务器
  Serial.println("HTTP server started");                   // 输出服务器启动成功信息
  Serial.println("Web server running on http://" + WiFi.localIP().toString());  // 显示服务器地址

  // 连接MQTT
  connectMQTT();

  display.clearBuffer();                                  // 清空OLED准备进入主循环显示
  display.setFont(u8g2_font_ncenB08_tr);                  // 设置字体
  display.drawStr(0, 32, "Starting...");                // 显示启动状态
  display.sendBuffer();                                   // 更新OLED

  Serial.println("System ready. Watchdog running.");
  Serial.println("=== Entering main loop ===");
}

/**
 * loop() - 主循环函数
 * 程序启动后无限循环执行，用于持续读取和显示数据，并处理Web请求
 */
void loop() {
  // ==================== 喂看门狗 ====================
  esp_task_wdt_reset();                                    // 重置看门狗计时器,防止系统重启
                                                            // 必须在30秒内调用一次

  // ==================== 系统保护检查 ====================
  checkWiFiConnection();                                   // 检查并恢复WiFi连接
  checkNTPSync();                                         // 定期同步NTP时间
  checkMemory();                                           // 监控剩余内存
  checkPIRSensor();                                        // 检查PIR传感器状态
  
  // ==================== MQTT客户端循环 ====================
  if (!mqttClient.connected()) {
    connectMQTT();
  }
  mqttClient.loop();

  // ==================== 获取时间 ====================
  struct tm timeinfo;                                      // 定义时间结构体变量
                                                            // tm结构体包含年、月、日、时、分、秒等字段

  // 获取本地时间
  // getLocalTime()会从NTP服务器获取时间并填充到timeinfo结构体
  if(!getLocalTime(&timeinfo)) {                          // 如果获取时间失败
    Serial.println("Failed to obtain time");              // 输出错误信息

    // 如果有上次有效时间，使用它（继续显示，不停止）
    if(hasValidTime) {
      memcpy(&timeinfo, &lastValidTime, sizeof(struct tm));
      // 手动增加1秒，保持时间继续走动
      timeinfo.tm_sec++;
      if(timeinfo.tm_sec >= 60) {
        timeinfo.tm_sec = 0;
        timeinfo.tm_min++;
        if(timeinfo.tm_min >= 60) {
          timeinfo.tm_min = 0;
          timeinfo.tm_hour++;
          if(timeinfo.tm_hour >= 24) {
            timeinfo.tm_hour = 0;
          }
        }
      }
      Serial.println("Using fallback time");
    } else {
      // 显示同步状态
      display.clearBuffer();
      display.setFont(u8g2_font_ncenB08_tr);
      display.drawStr(0, 32, "Syncing Time...");
      display.sendBuffer();
      esp_task_wdt_reset();  // 喂狗
      delay(500);           // 等待0.5秒后重试
      return;             // 跳过本次循环，等待下次重试
    }
  } else {
    // 时间获取成功，保存为有效时间
    memcpy(&lastValidTime, &timeinfo, sizeof(struct tm));
    hasValidTime = true;
  }

  // ==================== 读取温湿度（每5次循环读取一次=5秒） ====================
  sensorUpdateCounter++;
  if(sensorUpdateCounter >= sensorUpdateInterval) {
    sensorUpdateCounter = 0;  // 重置计数器

    // AHT20需要先触发测量，添加I2C超时保护
    esp_task_wdt_reset();  // 读传感器前喂狗
    sensors_event_t humidity, temp;
    
    // 使用try-catch模式避免I2C死锁
    unsigned long i2cTimeout = millis();
    bool readSuccess = false;
    
    // 给I2C读取最多2秒时间，避免无限阻塞
    while(millis() - i2cTimeout < 2000 && !readSuccess) {
      esp_task_wdt_reset();  // 持续喂狗
      aht.getEvent(&humidity, &temp);
      
      // 检查数据是否有效
      if(!isnan(temp.temperature) && !isnan(humidity.relative_humidity)) {
        readSuccess = true;
      } else {
        delay(50);  // 等待后重试
      }
    }
    
    // 如果I2C读取超时，跳过本次传感器更新
    if(!readSuccess) {
      Serial.println("WARNING: AHT20 I2C read timeout, skipping this update");
      return;  // 跳过本次循环
    }

    // 应用校准偏移值
    float temperature = temp.temperature + tempOffset;    // 温度校准后值（摄氏度）
    float hum = humidity.relative_humidity + humOffset;    // 湿度校准后值（百分比）

    // 调试输出（显示原始值和校准后值）
    Serial.print("Raw Temp: ");
    Serial.print(temp.temperature, 2);
    Serial.print("°C → Calibrated: ");
    Serial.print(temperature, 2);
    Serial.print("°C, Raw Hum: ");
    Serial.print(humidity.relative_humidity, 1);
    Serial.print("% → Calibrated: ");
    Serial.print(hum, 1);
    Serial.println("%");

    // 检查传感器是否正常工作
    if(isnan(temperature) || isnan(hum)) {  // 如果读取失败
      Serial.println("Error: AHT20 reading invalid!");
      display.clearBuffer();                                 // 清空缓冲区（U8g2版本）
      display.setFont(u8g2_font_ncenB08_tr);                // 设置字体
      display.drawStr(0, 15, "Sensor Error!");               // 显示传感器错误
      display.sendBuffer();                                 // 发送到OLED显示
      esp_task_wdt_reset();  // 喂狗
      delay(2000);          // 显示2秒
      return;                // 跳过本次循环
    }

    // 更新全局变量（供Web服务器使用）
    currentTemperature = temperature;                       // 保存当前温度值
    currentHumidity = hum;                               // 保存当前湿度值
    
    // 发布传感器数据到MQTT（每次读取后）
    publishSensorData();
    
    // 发布传感器数据到MQTT（每次读取后）
    publishSensorData();
    
    // 发布传感器数据到MQTT（每次读取后）
    publishSensorData();
  }

  // ==================== 更新时间和日期（每次循环都更新） ====================
  sprintf(currentTime, "%02d:%02d:%02d",                    // 格式化时间字符串
          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  sprintf(currentDate, "%04d-%02d-%02d",                    // 格式化日期字符串
          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
  firstDataReady = true;                                    // 标记数据已准备就绪

  // 准备显示字符串（用于OLED和串口）
  char dateStr[20];                                        // 定义字符数组存储日期字符串
  sprintf(dateStr, "%04d-%02d-%02d",                       // 格式化日期字符串
           timeinfo.tm_year + 1900,                         // 年份：2025
           timeinfo.tm_mon + 1,                             // 月份：1-12
           timeinfo.tm_mday);                               // 日期：1-31

  char timeStr[16];                                        // 定义字符数组存储时间字符串
  sprintf(timeStr, "%02d:%02d:%02d",                       // 格式化时间为HH:MM:SS
           timeinfo.tm_hour,                                // 小时：0-23
           timeinfo.tm_min,                                 // 分钟：0-59
           timeinfo.tm_sec);                                // 秒：0-59

  char tempHumStr[30];                                     // 定义字符数组存储温湿度字符串
  sprintf(tempHumStr, "%.1f\xB0""C  %.1f%%",              // 格式化温湿度字符串，\xB0是度数符号的十六进制码
          currentTemperature,                               // 温度值（使用全局变量）
          currentHumidity);                                 // 湿度值（使用全局变量）

  // ==================== OLED显示 ====================
  // 只有屏幕开启时才显示内容
  if(screenOn) {
    // 清空显示屏缓冲区（U8g2版本）
    display.clearBuffer();                                  // 清空所有待显示的内容
                                                                // 注意：此时OLED屏幕还没变，需要调用sendBuffer()才更新

    // ========== 左上角显示人体感应图标 ==========
    display.setFont(u8g2_font_open_iconic_all_1x_t);  // 使用小图标字体（1x）
    int pirState = digitalRead(PIR_SENSOR_PIN);
    if(pirState == HIGH) {
      // 有人：显示人形图标
      display.drawGlyph(0, 8, 0x40);  // 0x40是人形图标，小尺寸
    } else {
      // 无人：显示空心圆圈或叉号
      display.drawGlyph(0, 8, 0x45);  // 0x45是圆圈图标，小尺寸
    }

    // ========== 显示日期（居中） ==========
    printCentered(dateStr, 10, u8g2_font_6x10_tr);         // 在y=10位置居中显示日期，使用更稳定的6x10字体

    // ========== 显示时间（居中，大字体，第二行） ==========
    printCentered(timeStr, 38, u8g2_font_ncenB18_tr);       // 在y=38位置居中显示，使用大字体（屏幕正中央）

    // ========== 显示温湿度（居中，较小字体，第三行） ==========
    printCentered(tempHumStr, 60, u8g2_font_ncenB12_tf);    // 在y=60位置居中显示温湿度，使用支持完整字符集的字体

    // 刷新显示屏（U8g2版本）
    display.sendBuffer();                                   // 将缓冲区的所有内容发送到OLED屏幕显示
                                                                // 此时用户才能看到屏幕上的内容
  } else {
    // 屏幕关闭状态：清空OLED或熄屏
    display.clearBuffer();
    display.sendBuffer();  // 发送空白缓冲区，清空屏幕
  }

  // ==================== 串口输出（调试用） ====================
  // 使用安全串口输出，避免阻塞
  char debugBuffer[128];
  int len = snprintf(debugBuffer, sizeof(debugBuffer),
    "Time: %s  Temp: %.1f C  WiFi: %s  PIR: %s  FreeMem: %dKB",
    timeStr, currentTemperature,
    WiFi.status() == WL_CONNECTED ? "OK" : "LOST",
    digitalRead(PIR_SENSOR_PIN) == HIGH ? "HIGH" : "LOW",
    ESP.getFreeHeap() / 1024
  );
  
  if(len > 0 && len < sizeof(debugBuffer)) {
    safeSerialPrintln(debugBuffer);
  }

  // ==================== 处理Web请求和MQTT消息 ====================
  // 在delay期间也要持续处理HTTP请求和MQTT消息，避免请求堆积
  unsigned long delayStart = millis();
  while(millis() - delayStart < 1000) {
    server.handleClient();  // 持续处理HTTP请求
    mqttClient.loop();      // 处理MQTT消息
    delay(10);  // 短暂延迟，避免CPU占用过高
  }
}

/**
 * 程序执行流程总结（U8g2版本）：
 *
 * 1. setup()只执行一次：
 *    - 初始化串口（115200波特率）
 *    - 初始化U8g2 OLED显示屏
 *    - 初始化DHT20温湿度传感器
 *    - 连接WiFi网络
 *    - 配置NTP时间服务器
 *    - 启动Web服务器（监听80端口）
 *
 * 2. loop()无限循环（每秒一次）：
 *    - 从NTP获取当前时间
 *    - 读取DHT20温湿度
 *    - 检查传感器是否正常
 *    - 更新全局变量（供Web使用）
 *    - 清空缓冲区
 *    - 居中显示日期（小字体：u8g2_font_ncenB08_tr）
 *    - 居中显示时间（中等字体：u8g2_font_ncenB12_tr）
 *    - 居中显示温湿度（中等字体：u8g2_font_ncenB12_tr）
 *    - 刷新OLED屏幕（sendBuffer）
 *    - 处理Web服务器请求
 *    - 串口输出调试信息
 *    - 延迟1秒
 *
 * U8g2字体说明：
 * - u8g2_font_ncenB08_tr: 小字体（8像素高度），用于日期
 * - u8g2_font_ncenB12_tr: 中等字体（12像素高度），用于时间和温湿度
 * - 更多字体可在U8g2库文档中查找
 *
 * Web服务器功能：
 * - 访问 http://IP地址/ - 查看美观的网页界面（自动每3秒刷新）
 * - 访问 http://IP地址/temperature - 获取纯文本温度（如"25.3°C"）
 * - 访问 http://IP地址/humidity - 获取纯文本湿度（如"65.2%"）
 * - 访问 http://IP地址/json - 获取JSON格式数据
 *
 * 使用示例：
 * 假设ESP32的IP地址是192.168.1.100：
 * - 手机浏览器访问：http://192.168.1.100
 * - 电脑浏览器访问：http://192.168.1.100
 * - 其他程序调用API：curl http://192.168.1.100/json
 *
 * 关键概念：
 * - I2C通信：OLED和DHT20使用I2C协议（两根线：SCL时钟线、SDA数据线）
 * - DHT20：温湿度传感器，I2C接口，无需上拉电阻
 * - NTP：网络时间协议，从互联网服务器获取准确时间
 * - HTTP服务器：ESP32作为Web服务器，响应手机/电脑的HTTP请求
 * - HTML/CSS/JavaScript：构建美观的网页界面
 * - API接口：提供程序化访问数据的接口（JSON、纯文本）
 * - U8g2缓冲区模式：先绘制到缓冲区，再一次性发送到OLED
 * - getUTF8Width：获取文本宽度，支持UTF-8编码（包括中文）
 * - 居中算法：(屏幕宽度 - 文本宽度) / 2
 * - sprintf：C语言格式化字符串函数，用于拼接各种格式的数据
 */