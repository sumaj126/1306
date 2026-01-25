/**
 * ESP32 + DS18B20 + OLED 0.96寸(SSD1306) 温度时间显示项目
 * 功能：从NTP服务器获取网络时间，从DS18B20读取温度，在OLED屏幕上居中显示
 *
 * 硬件连接：
 * - OLED (I2C): VCC->3.3V, GND->GND, SCL->GPIO22, SDA->GPIO21
 * - DS18B20: VCC->3.3V, GND->GND, DATA->GPIO4 (需4.7K上拉电阻到3.3V)
 */

// ==================== 头文件包含 ====================
#include <Wire.h>                      // I2C通信库，用于OLED显示屏
#include <Adafruit_GFX.h>              // Adafruit图形库基础类，提供绘图功能
#include <Adafruit_SSD1306.h>          // SSD1306 OLED显示屏驱动库
#include <OneWire.h>                   // OneWire单总线通信协议库，用于DS18B20
#include <DallasTemperature.h>         // Dallas温度传感器库，封装了DS18B20的操作
#include <WiFi.h>                      // ESP32 WiFi功能库
#include <time.h>                      // C标准时间库，用于时间处理

// ==================== OLED显示屏配置 ====================
#define SCREEN_WIDTH 128               // OLED屏幕宽度（像素）
#define SCREEN_HEIGHT 64               // OLED屏幕高度（像素）
#define OLED_RESET -1                   // OLED复位引脚，-1表示不使用复位引脚
#define SCREEN_ADDRESS 0x3C             // OLED I2C地址（常见的0.3C或0x3D）

// ==================== DS18B20引脚定义 ====================
#define ONE_WIRE_BUS 4                  // DS18B20数据引脚连接到ESP32的GPIO4

// ==================== 创建对象实例 ====================
// 创建OLED显示对象，传入屏幕尺寸、Wire对象和复位引脚
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// 创建OneWire对象，传入数据引脚
OneWire oneWire(ONE_WIRE_BUS);

// 创建DallasTemperature对象，传入OneWire对象（用于通信）
DallasTemperature sensors(&oneWire);

// ==================== WiFi配置 ====================
// 注意：请修改为您的WiFi网络名称和密码
const char* ssid = "jiajia";        // WiFi名称（SSID）
const char* password = "9812061104"; // WiFi密码

// ==================== NTP时间服务器配置 ====================
const char* ntpServer = "pool.ntp.org";           // NTP服务器地址，全球时间同步服务器
const long gmtOffset_sec = 8 * 3600;              // 时区偏移（秒），8小时表示东八区（北京时间）
const int daylightOffset_sec = 0;                 // 夏令时偏移（秒），中国不使用夏令时设为0

/**
 * 居中显示文本函数
 * @param text 要显示的文本字符串
 * @param y 垂直位置（像素坐标）
 * @param textSize 文字大小（1=正常，2=双倍）
 * 
 * 原理：
 * - 计算文本宽度：字符数 × 6像素 × textSize（每个字符约6像素宽）
 * - 计算水平居中位置：(屏幕宽度 - 文本宽度) / 2
 */
void printCentered(const char* text, int16_t y, int textSize) {
  display.setTextSize(textSize);                           // 设置文字大小
  int16_t x = (SCREEN_WIDTH - (strlen(text) * 6 * textSize)) / 2;  // 计算居中的x坐标
  display.setCursor(x, y);                                 // 设置光标位置
  display.print(text);                                     // 显示文本
}

/**
 * 居中显示温度函数
 * @param temp 温度值（浮点数）
 * @param y 垂直位置（像素坐标）
 * @param textSize 文字大小
 * 
 * 原理：
 * - 将温度值格式化为字符串，保留一位小数
 * - 计算温度数值、度数符号(°)和字母"C"的总宽度
 * - 计算居中位置后依次打印各部分
 */
void printTempCentered(float temp, int16_t y, int textSize) {
  display.setTextSize(textSize);                           // 设置文字大小

  // 将温度格式化为字符串（保留一位小数）
  // %.1f 表示浮点数格式，保留1位小数
  char tempNumStr[10];                                     // 定义字符数组存储温度字符串
  sprintf(tempNumStr, "%.1f", temp);                       // 格式化温度值，例如"25.3"

  // 度数符号和C的宽度计算
  // 每个字符约6像素宽，度数符号和C各占一个字符宽度
  int symbolWidth = 2 * 6 * textSize;                     // 符号部分总宽度
  int tempNumWidth = strlen(tempNumStr) * 6 * textSize;   // 温度数值部分宽度

  // 计算总宽度并居中
  int totalWidth = tempNumWidth + symbolWidth;            // 总宽度 = 数值 + 符号
  int16_t x = (SCREEN_WIDTH - totalWidth) / 2;            // 计算居中的x坐标
  display.setCursor(x, y);                                 // 设置光标位置

  // 打印温度值（使用格式化后的字符串）
  display.print(tempNumStr);                               // 打印"25.3"
  // 打印度数符号
  display.print((char)247);                                // ASCII 247是度数符号°
  // 打印C
  display.print("C");                                      // 打印字母"C"，最终显示为"25.3°C"
}

/**
 * setup() - 初始化函数
 * 程序启动时执行一次，用于初始化所有硬件和设置
 */
void setup() {
  // 初始化串口通信
  Serial.begin(115200);                                    // 设置串口波特率为115200
                                                            // 用于向电脑输出调试信息
  
  // 初始化OLED显示屏
  // SSD1306_SWITCHCAPVCC表示使用内部电荷泵（3.3V供电）
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {  // 尝试初始化OLED
    Serial.println(F("SSD1306 allocation failed"));       // 如果初始化失败，输出错误信息
    for(;;);                                               // 无限循环，停止程序
  }
  display.clearDisplay();                                  // 清空显示屏缓冲区
  display.setTextSize(1);                                  // 设置文字大小为1（正常大小）
  display.setTextColor(SSD1306_WHITE);                     // 设置文字颜色为白色（点亮像素）
  display.setCursor(0,0);                                  // 设置光标到左上角(0,0)
  display.println("Initializing...");                      // 显示初始化信息
  display.display();                                       // 将缓冲区内容发送到OLED显示
  
  // 初始化温度传感器
  sensors.begin();                                         // 启动DS18B20传感器
  Serial.println("DS18B20 initialized");                   // 输出传感器初始化成功信息
  
  // 连接WiFi网络
  WiFi.begin(ssid, password);                             // 开始连接WiFi
  display.setCursor(0, 16);                                // 设置光标到第2行
  display.println("Connecting WiFi...");                   // 显示正在连接WiFi
  display.display();                                       // 更新OLED显示
  
  Serial.print("Connecting to WiFi");                      // 串口输出连接信息
  while(WiFi.status() != WL_CONNECTED) {                   // 循环等待WiFi连接成功
    delay(500);                                            // 延迟500毫秒
    Serial.print(".");                                     // 打印一个点表示等待中
    display.print(".");                                     // OLED上也显示点
    display.display();                                     // 更新OLED
  }
  Serial.println();                                        // 换行
  Serial.println("WiFi connected");                        // 输出连接成功信息
  
  display.clearDisplay();                                  // 清空OLED
  display.setCursor(0, 0);                                 // 设置光标到左上角
  display.println("WiFi Connected!");                      // 显示连接成功
  display.println("IP: " + WiFi.localIP().toString());    // 显示ESP32的IP地址
  display.display();                                       // 更新OLED
  delay(2000);                                             // 显示2秒让用户看到IP地址
  
  // 配置网络时间同步（NTP）
  // configTime用于配置ESP32的时间同步服务
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // 设置时区、夏令时和NTP服务器
  Serial.println("NTP configured");                         // 输出NTP配置成功信息
  
  display.clearDisplay();                                  // 清空OLED准备进入主循环显示
  display.display();                                       // 更新OLED
}

/**
 * loop() - 主循环函数
 * 程序启动后无限循环执行，用于持续读取和显示数据
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
    display.clearDisplay();                                // 清空OLED
    display.setTextSize(1);                                // 设置文字大小为1
    display.setCursor(0, 0);                             // 设置光标到左上角
    display.println("Sensor Error!");                      // 显示传感器错误
    display.display();                                     // 更新OLED
    delay(2000);                                           // 显示2秒
    return;                                                // 跳过本次循环
  }
  
  // 清空显示屏缓冲区
  display.clearDisplay();                                  // 清空所有待显示的内容
                                                            // 注意：此时OLED屏幕还没变，需要调用display()才更新
  
  // ========== 显示日期（居中） ==========
  display.setTextSize(1);                                  // 设置文字大小为1（正常大小）
  char dateStr[20];                                        // 定义字符数组存储日期字符串
  // sprintf格式化日期字符串
  // %04d: 4位数字，不足补0（年份）
  // %02d: 2位数字，不足补0（月份和日期）
  // timeinfo.tm_year: 从1900年开始的年数，所以+1900得到实际年份
  // timeinfo.tm_mon: 0-11表示1-12月，所以+1得到实际月份
  // timeinfo.tm_mday: 1-31表示日期
  sprintf(dateStr, "%04d-%02d-%02d",                       // 格式化字符串模板
           timeinfo.tm_year + 1900,                         // 年份：2025
           timeinfo.tm_mon + 1,                             // 月份：1-12
           timeinfo.tm_mday);                               // 日期：1-31
  printCentered(dateStr, 2, 1);                            // 在y=2位置居中显示日期
  
  // ========== 显示时间（居中，大字体） ==========
  char timeStr[16];                                        // 定义字符数组存储时间字符串
  sprintf(timeStr, "%02d:%02d:%02d",                       // 格式化时间为HH:MM:SS
           timeinfo.tm_hour,                                // 小时：0-23
           timeinfo.tm_min,                                 // 分钟：0-59
           timeinfo.tm_sec);                                // 秒：0-59
  printCentered(timeStr, 20, 2);                           // 在y=20位置居中显示，字体大小为2（双倍）
  
  // ========== 显示温度（居中） ==========
  printTempCentered(temperature, 48, 2);                    // 在y=48位置居中显示温度，字体大小为2
  
  // 刷新显示屏
  display.display();                                       // 将缓冲区的所有内容发送到OLED屏幕显示
                                                            // 此时用户才能看到屏幕上的内容
  
  // ========== 输出到串口（调试用） ==========
  Serial.print("Time: ");                                  // 打印"Time: "
  Serial.print(timeStr);                                   // 打印时间字符串，如"14:30:45"
  Serial.print("  Temp: ");                               // 打印"  Temp: "
  Serial.print(temperature, 2);                           // 打印温度值，保留2位小数，如"25.37"
  Serial.println(" C");                                    // 打印" C"并换行
  
  // ========== 等待1秒后继续循环 ==========
  delay(1000);                                             // 延迟1000毫秒（1秒）
                                                            // 这样每秒更新一次显示
}

/**
 * 程序执行流程总结：
 * 
 * 1. setup()只执行一次：
 *    - 初始化串口（115200波特率）
 *    - 初始化OLED显示屏
 *    - 初始化DS18B20温度传感器
 *    - 连接WiFi网络
 *    - 配置NTP时间服务器
 * 
 * 2. loop()无限循环（每秒一次）：
 *    - 从NTP获取当前时间
 *    - 读取DS18B20温度
 *    - 检查传感器是否正常
 *    - 清空屏幕
 *    - 居中显示日期（小字体）
 *    - 居中显示时间（大字体）
 *    - 居中显示温度（大字体）
 *    - 刷新OLED屏幕
 *    - 串口输出调试信息
 *    - 延迟1秒
 * 
 * 关键概念：
 * - I2C通信：OLED使用I2C协议（两根线：SCL时钟线、SDA数据线）
 * - OneWire：DS18B20使用单总线协议（一根数据线）
 * - NTP：网络时间协议，从互联网服务器获取准确时间
 * - 居中算法：(屏幕宽度 - 文本宽度) / 2
 * - sprintf：C语言格式化字符串函数，用于拼接各种格式的数据
 */
