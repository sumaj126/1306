/**
 * ESP32 + DS18B20 + OLED 0.96寸(SSD1306) 温度时间显示项目 + Web服务器
 * 使用U8g2字体库，支持更多字体和语言
 * 功能：
 * 1. 从NTP服务器获取网络时间
 * 2. 从DS18B20读取温度
 * 3. 在OLED屏幕上居中显示时间和温度（使用U8g2精美字体）
 * 4. 启动Web服务器，手机可通过浏览器访问ESP32查看温度和时间
 *
 * 硬件连接：
 * - OLED (I2C): VCC->3.3V, GND->GND, SCL->GPIO22, SDA->GPIO21
 * - DS18B20: VCC->3.3V, GND->GND, DATA->GPIO4 (需4.7K上拉电阻到3.3V)
 *
 * 使用方法：
 * 1. 连接WiFi后，OLED会显示ESP32的IP地址
 * 2. 在手机浏览器输入该IP地址即可查看温度
 * 3. 访问 http://IP地址/temperature 可获取纯文本温度数据
 * 4. 访问 http://IP地址/json 可获取JSON格式数据
 */

// ==================== 头文件包含 ====================
#include <U8g2lib.h>                   // U8g2字体库，提供丰富的字体支持
#include <Wire.h>                      // I2C通信库，用于OLED显示屏
#include <OneWire.h>                   // OneWire单总线通信协议库，用于DS18B20
#include <DallasTemperature.h>         // Dallas温度传感器库，封装了DS18B20的操作
#include <WiFi.h>                      // ESP32 WiFi功能库
#include <WebServer.h>                 // ESP32 Web服务器库，用于创建HTTP服务器
#include <time.h>                      // C标准时间库，用于时间处理

// ==================== OLED显示屏配置 ====================
// 使用SSD1306驱动，I2C协议，完整帧缓冲模式
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// ==================== DS18B20引脚定义 ====================
#define ONE_WIRE_BUS 4                  // DS18B20数据引脚连接到ESP32的GPIO4

// ==================== 创建对象实例 ====================
// 创建OneWire对象，传入数据引脚
OneWire oneWire(ONE_WIRE_BUS);

// 创建DallasTemperature对象，传入OneWire对象（用于通信）
DallasTemperature sensors(&oneWire);

// 创建Web服务器对象，监听80端口（HTTP默认端口）
WebServer server(80);

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
const char* ntpServer = "pool.ntp.org";           // NTP服务器地址，全球时间同步服务器
const long gmtOffset_sec = 8 * 3600;              // 时区偏移（秒），8小时表示东八区（北京时间）
const int daylightOffset_sec = 0;                 // 夏令时偏移（秒），中国不使用夏令时设为0

// ==================== 全局变量 ====================
float currentTemperature = 0.0;          // 存储当前温度值（供Web服务器使用）
char currentTime[32] = "";                // 存储当前时间字符串
char currentDate[32] = "";               // 存储当前日期字符串
bool firstDataReady = false;             // 标记是否已获取到第一组数据

/**
 * 居中显示文本函数（U8g2版本）
 * @param text 要显示的文本字符串
 * @param y 垂直位置（像素坐标）
 * @param font 字体指针
 *
 * 原理：
 * - 使用U8g2的getUTF8Width获取文本的精确宽度
 * - 计算水平居中位置：(屏幕宽度 - 文本宽度) / 2
 * - 使用drawStr显示文本
 */
void printCentered(const char* text, int16_t y, const uint8_t* font) {
  display.setFont(font);                                   // 设置字体
  int16_t textWidth = display.getUTF8Width(text);         // 获取文本宽度（支持中文）
  int16_t x = (128 - textWidth) / 2;                      // 计算居中的x坐标
  display.drawStr(x, y, text);                           // 使用drawStr显示文本
}

/**
 * 居中显示温度函数（U8g2版本）
 * @param temp 温度值（浮点数）
 * @param y 垂直位置（像素坐标）
 * @param font 字体指针
 *
 * 原理：
 * - 将温度值格式化为字符串，保留一位小数
 * - 手动绘制度数符号（小圆圈）+ 绘制温度数值和字母C
 * - 计算总宽度后居中显示
 */
void printTempCentered(float temp, int16_t y, const uint8_t* font) {
  display.setFont(font);                                   // 设置字体

  // 获取字体参数（必须在设置字体后调用）
  int16_t fontAscent = display.getFontAscent();           // 字体上升部分高度
  int16_t fontDescent = display.getFontDescent();         // 字体下降部分高度
  int16_t fontHeight = fontAscent - fontDescent;          // 总字体高度

  // 将温度格式化为字符串（保留一位小数）
  char tempNumStr[10];                                     // 定义字符数组存储温度字符串
  sprintf(tempNumStr, "%.1f", temp);                       // 格式化温度值，例如"25.3"

  // 获取各部分的宽度
  int16_t numWidth = display.getUTF8Width(tempNumStr);    // 数值部分宽度
  int16_t cWidth = display.getUTF8Width("C");             // 字母C宽度

  // 手动绘制度数符号的宽度（固定大小）
  int16_t degreeWidth = 8;                                // 度数符号宽度（小圆圈）

  // 定义间距（像素）
  int16_t spacing = 2;                                    // 各部分之间的间距

  // 计算总宽度（包括间距）
  int16_t totalWidth = numWidth + degreeWidth + cWidth + (spacing * 2);  // 总宽度 + 2个间距

  // 计算起始x坐标（居中）
  int16_t startX = (128 - totalWidth) / 2;

  // 依次绘制各部分
  int16_t currentX = startX;                              // 当前绘制位置

  // 绘制温度数值
  display.drawStr(currentX, y, tempNumStr);
  currentX += numWidth + spacing;                          // 移动到度数符号位置

  // 手动绘制度数符号（小圆圈）
  // U8g2的drawStr中y坐标是字符的基线（底部），所以圆圈需要向上调整
  int16_t circleCenterY = y - fontHeight / 2 - 2;         // 圆圈中心Y坐标（基于字体基线调整，再向上2像素）
  display.drawCircle(currentX + degreeWidth / 2, circleCenterY, degreeWidth / 2 - 1);  // 绘制空心圆圈
  currentX += degreeWidth + spacing;                       // 移动到C位置

  // 绘制字母C
  display.drawStr(currentX, y, "C");
}

/**
 * Web服务器 - 主页处理函数
 * 访问 http://ESP32_IP/ 时调用此函数
 * 返回一个美观的HTML页面，显示温度和时间信息
 */
void handleRoot() {
  // 添加CORS响应头，允许跨域访问（用于群晖反向代理）
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  String html = "<!DOCTYPE html>\n";                       // HTML5文档声明
  html += "<html>\n<head>\n";                              // HTML开始标签
  html += "<meta charset=\"UTF-8\">\n";                     // 设置字符编码为UTF-8
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";  // 适配移动端
  html += "<title>ESP32 温度监控</title>\n";                // 网页标题
  html += "<style>\n";                                     // CSS样式开始

  // 美观的CSS样式
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; ";  // 页面字体和内边距
  html += "background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); ";      // 渐变背景
  html += "min-height: 100vh; display: flex; justify-content: center; align-items: center; }\n";  // 居中布局
  html += ".container { background: white; padding: 30px; border-radius: 20px; ";  // 容器样式
  html += "box-shadow: 0 10px 40px rgba(0,0,0,0.2); max-width: 400px; width: 100%; text-align: center; }\n";  // 阴影和圆角
  html += "h1 { color: #333; margin-bottom: 10px; font-size: 28px; }\n";      // 标题样式
  html += ".temperature { font-size: 48px; font-weight: bold; color: #e74c3c; margin: 20px 0; }\n";  // 温度大字体
  html += ".time { font-size: 24px; color: #666; margin: 10px 0; }\n";         // 时间样式
  html += ".date { font-size: 18px; color: #888; margin-bottom: 20px; }\n";    // 日期样式
  html += ".icon { font-size: 60px; margin-bottom: 10px; }\n";                 // 图标样式
  html += ".refresh-info { font-size: 12px; color: #aaa; margin-top: 20px; }\n";  // 刷新提示
  html += ".unit { font-size: 24px; }\n";                                      // 单位样式

  html += "</style>\n";                                    // CSS样式结束
  html += "<script>\n";                                    // JavaScript开始

  // 自动刷新页面（每5秒刷新一次）
  html += "setTimeout(function(){location.reload();}, 5000);\n";  // 5秒后自动刷新
  html += "</script>\n";                                   // JavaScript结束
  html += "</head>\n<body>\n";                             // head结束，body开始
  html += "<div class=\"container\">\n";                   // 容器开始

  // 网页内容
  html += "<div class=\"icon\">🌡️</div>\n";                // 温度计图标
  html += "<h1>老苏书房实时温度</h1>\n";                       // 主标题
  html += "<div class=\"date\">" + String(currentDate) + "</div>\n";  // 显示日期
  html += "<div class=\"time\">" + String(currentTime) + "</div>\n";  // 显示时间
  html += "<div class=\"temperature\">" + String(currentTemperature, 1) + "<span class=\"unit\">°C</span></div>\n";  // 显示温度
  html += "<div class=\"refresh-info\">页面每5秒自动刷新</div>\n";    // 刷新提示

  html += "</div>\n";                                      // 容器结束
  html += "</body>\n</html>\n";                            // body结束，HTML结束

  server.send(200, "text/html", html);                    // 发送HTML响应给客户端
                                                            // 200表示成功，text/html表示HTML格式
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

  String tempText = String(currentTemperature, 1) + "°C";  // 格式化温度为字符串，如"25.3°C"
  server.send(200, "text/plain", tempText);                // 发送纯文本响应
}

/**
 * Web服务器 - JSON API处理函数
 * 访问 http://ESP32_IP/json 时调用此函数
 * 返回JSON格式的数据，包含温度、时间和日期
 */
void handleJson() {
  // 添加CORS响应头，允许跨域访问（用于群晖反向代理）
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");

  String json = "{";                                       // JSON开始
  json += "\"temperature\": " + String(currentTemperature, 1) + ",";  // 温度值
  json += "\"time\": \"" + String(currentTime) + "\",";     // 时间字符串
  json += "\"date\": \"" + String(currentDate) + "\",";     // 日期字符串
  json += "\"unit\": \"C\",";                               // 温度单位
  json += "\"status\": \"ok\"";                             // 状态
  json += "}";                                              // JSON结束

  server.send(200, "application/json", json);             // 发送JSON响应
}

/**
 * Web服务器 - 404错误处理函数
 * 当访问不存在的路径时调用此函数
 */
void handleNotFound() {
  String message = "404 Not Found\n\n";                    // 错误信息
  message += "URI: " + server.uri() + "\n";                // 访问的URI
  message += "Method: " + String((server.method() == HTTP_GET) ? "GET" : "POST") + "\n";  // 请求方法
  message += "Arguments: " + String(server.args()) + "\n"; // 参数数量
  for (uint8_t i = 0; i < server.args(); i++) {            // 遍历所有参数
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";  // 参数名和值
  }
  server.send(404, "text/plain", message);                 // 发送404错误响应
}

/**
 * setup() - 初始化函数
 * 程序启动时执行一次，用于初始化所有硬件和设置
 */
void setup() {
  // 初始化串口通信
  Serial.begin(115200);                                    // 设置串口波特率为115200
                                                            // 用于向电脑输出调试信息

  // 初始化OLED显示屏（U8g2版本）
  display.begin();                                          // 初始化U8g2显示屏
  display.clearBuffer();                                   // 清空显示缓冲区

  // 初始化温度传感器
  sensors.begin();                                         // 启动DS18B20传感器
  Serial.println("DS18B20 initialized");                   // 输出传感器初始化成功信息

  // 连接WiFi网络
  WiFi.begin(ssid, password);                             // 开始连接WiFi

  Serial.print("Connecting to WiFi");                      // 串口输出连接信息
  while(WiFi.status() != WL_CONNECTED) {                   // 循环等待WiFi连接成功
    delay(500);                                            // 延迟500毫秒
    Serial.print(".");                                     // 打印一个点表示等待中

    // 在OLED上显示连接进度
    display.clearBuffer();                                 // 清空缓冲区
    display.setFont(u8g2_font_ncenB08_tr);                // 设置小字体
    display.drawStr(0, 15, "Connecting WiFi...");         // 显示连接信息
    display.sendBuffer();                                  // 发送到OLED显示
  }
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
  delay(3000);                                             // 显示3秒让用户看到IP地址

  // 配置网络时间同步（NTP）
  // configTime用于配置ESP32的时间同步服务
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // 设置时区、夏令时和NTP服务器
  Serial.println("NTP configured");                         // 输出NTP配置成功信息

  // ==================== 配置静态IP ====================
  if (WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Static IP configured successfully");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    Serial.println("External access: http://sumaj.synology.me:7788");
  } else {
    Serial.println("Failed to configure Static IP, using DHCP");
  }

  // ==================== 配置Web服务器 ====================
  server.on("/", handleRoot);                              // 注册根路径处理函数（主页）
  server.on("/temperature", handleTemperature);            // 注册温度API路径
  server.on("/json", handleJson);                          // 注册JSON API路径
  server.onNotFound(handleNotFound);                       // 注册404处理函数

  server.begin();                                           // 启动Web服务器
  Serial.println("HTTP server started");                   // 输出服务器启动成功信息
  Serial.println("Web server running on http://" + WiFi.localIP().toString());  // 显示服务器地址

  display.clearBuffer();                                  // 清空OLED准备进入主循环显示
  display.sendBuffer();                                   // 更新OLED
}

/**
 * loop() - 主循环函数
 * 程序启动后无限循环执行，用于持续读取和显示数据，并处理Web请求
 */
void loop() {
  struct tm timeinfo;                                      // 定义时间结构体变量
                                                            // tm结构体包含年、月、日、时、分、秒等字段

  // 获取本地时间
  // getLocalTime()会从NTP服务器获取时间并填充到timeinfo结构体
  if(!getLocalTime(&timeinfo)) {                          // 如果获取时间失败
    Serial.println("Failed to obtain time");              // 输出错误信息
    return;                                                // 跳过本次循环，等待下次重试
  }

  // 请求温度传感器读取温度
  sensors.requestTemperatures();                           // 发送温度转换命令给DS18B20
  float temperature = sensors.getTempCByIndex(0);         // 读取第0个传感器的温度（摄氏度）
                                                            // getTempCByIndex()返回摄氏度温度值

  // 检查温度传感器是否正常工作
  // DEVICE_DISCONNECTED_C是错误代码，表示传感器未连接或故障
  if(temperature == DEVICE_DISCONNECTED_C) {              // 如果返回错误代码
    Serial.println("Error: DS18B20 not connected!");       // 输出错误信息
    display.clearBuffer();                                 // 清空缓冲区（U8g2版本）
    display.setFont(u8g2_font_ncenB08_tr);                // 设置字体
    display.drawStr(0, 15, "Sensor Error!");               // 显示传感器错误
    display.sendBuffer();                                 // 发送到OLED显示
    delay(2000);                                           // 显示2秒
    return;                                                // 跳过本次循环
  }

  // 更新全局变量（供Web服务器使用）
  currentTemperature = temperature;                       // 保存当前温度值
  sprintf(currentTime, "%02d:%02d:%02d",                    // 格式化时间字符串
          timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  sprintf(currentDate, "%04d-%02d-%02d",                    // 格式化日期字符串
          timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
  firstDataReady = true;                                    // 标记数据已准备就绪

  // 清空显示屏缓冲区（U8g2版本）
  display.clearBuffer();                                  // 清空所有待显示的内容
                                                            // 注意：此时OLED屏幕还没变，需要调用sendBuffer()才更新

  // ========== 显示日期（居中） ==========
  char dateStr[20];                                        // 定义字符数组存储日期字符串
  sprintf(dateStr, "%04d-%02d-%02d",                       // 格式化日期字符串
           timeinfo.tm_year + 1900,                         // 年份：2025
           timeinfo.tm_mon + 1,                             // 月份：1-12
           timeinfo.tm_mday);                               // 日期：1-31
  printCentered(dateStr, 8, u8g2_font_ncenB08_tr);        // 在y=8位置居中显示日期，使用小字体

  // ========== 显示时间（居中，大字体，屏幕正中央） ==========
  char timeStr[16];                                        // 定义字符数组存储时间字符串
  sprintf(timeStr, "%02d:%02d:%02d",                       // 格式化时间为HH:MM:SS
           timeinfo.tm_hour,                                // 小时：0-23
           timeinfo.tm_min,                                 // 分钟：0-59
           timeinfo.tm_sec);                                // 秒：0-59
  printCentered(timeStr, 36, u8g2_font_ncenB18_tr);       // 在y=36位置居中显示，使用大字体（屏幕正中央）

  // ========== 显示温度（居中，中等字体） ==========
  printTempCentered(temperature, 60, u8g2_font_ncenB14_tr);  // 在y=60位置居中显示温度，使用中等字体（接近底部）

  // 刷新显示屏（U8g2版本）
  display.sendBuffer();                                   // 将缓冲区的所有内容发送到OLED屏幕显示
                                                            // 此时用户才能看到屏幕上的内容

  // ========== 输出到串口（调试用） ==========
  Serial.print("Time: ");                                  // 打印"Time: "
  Serial.print(timeStr);                                   // 打印时间字符串，如"14:30:45"
  Serial.print("  Temp: ");                               // 打印"  Temp: "
  Serial.print(temperature, 2);                           // 打印温度值，保留2位小数，如"25.37"
  Serial.println(" C");                                    // 打印" C"并换行

  // ========== 处理Web请求 ==========
  server.handleClient();                                   // 处理来自客户端的HTTP请求
                                                            // 这个函数需要频繁调用，以确保及时响应客户端

  // ========== 等待1秒后继续循环 ==========
  delay(1000);                                             // 延迟1000毫秒（1秒）
                                                            // 这样每秒更新一次显示
}

/**
 * 程序执行流程总结（U8g2版本）：
 *
 * 1. setup()只执行一次：
 *    - 初始化串口（115200波特率）
 *    - 初始化U8g2 OLED显示屏
 *    - 初始化DS18B20温度传感器
 *    - 连接WiFi网络
 *    - 配置NTP时间服务器
 *    - 启动Web服务器（监听80端口）
 *
 * 2. loop()无限循环（每秒一次）：
 *    - 从NTP获取当前时间
 *    - 读取DS18B20温度
 *    - 检查传感器是否正常
 *    - 更新全局变量（供Web使用）
 *    - 清空缓冲区
 *    - 居中显示日期（小字体：u8g2_font_ncenB08_tr）
 *    - 居中显示时间（中等字体：u8g2_font_ncenB14_tr）
 *    - 居中显示温度（大字体：u8g2_font_ncenB18_tr）
 *    - 刷新OLED屏幕（sendBuffer）
 *    - 处理Web服务器请求
 *    - 串口输出调试信息
 *    - 延迟1秒
 *
 * U8g2字体说明：
 * - u8g2_font_ncenB08_tr: 小字体（8像素高度），用于日期
 * - u8g2_font_ncenB14_tr: 中等字体（14像素高度），用于时间
 * - u8g2_font_ncenB18_tr: 大字体（18像素高度），用于温度
 * - 更多字体可在U8g2库文档中查找
 *
 * Web服务器功能：
 * - 访问 http://IP地址/ - 查看美观的网页界面（自动每3秒刷新）
 * - 访问 http://IP地址/temperature - 获取纯文本温度（如"25.3°C"）
 * - 访问 http://IP地址/json - 获取JSON格式数据
 *
 * 使用示例：
 * 假设ESP32的IP地址是192.168.1.100：
 * - 手机浏览器访问：http://192.168.1.100
 * - 电脑浏览器访问：http://192.168.1.100
 * - 其他程序调用API：curl http://192.168.1.100/json
 *
 * 关键概念：
 * - I2C通信：OLED使用I2C协议（两根线：SCL时钟线、SDA数据线）
 * - OneWire：DS18B20使用单总线协议（一根数据线）
 * - NTP：网络时间协议，从互联网服务器获取准确时间
 * - HTTP服务器：ESP32作为Web服务器，响应手机/电脑的HTTP请求
 * - HTML/CSS/JavaScript：构建美观的网页界面
 * - API接口：提供程序化访问数据的接口（JSON、纯文本）
 * - U8g2缓冲区模式：先绘制到缓冲区，再一次性发送到OLED
 * - getUTF8Width：获取文本宽度，支持UTF-8编码（包括中文）
 * - 居中算法：(屏幕宽度 - 文本宽度) / 2
 * - sprintf：C语言格式化字符串函数，用于拼接各种格式的数据
 */
