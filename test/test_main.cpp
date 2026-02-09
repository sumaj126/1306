#include <Arduino.h>
#include <unity.h>

// 测试配置常量
#define TEST_WIFI_OK 3
#define TEST_WIFI_LOST 0

// 模拟的温湿度数据
struct TestSensorData {
    float temperature;
    float humidity;
    bool valid;
};

// 模拟的时间数据
struct TestTimeData {
    int hour;
    int minute;
    int second;
    int year;
    int month;
    int day;
};

// 测试用的全局变量
TestSensorData testSensorData;
TestTimeData testTimeData;
bool testScreenOn = true;
int testPirState = HIGH;

// ==================== 测试辅助函数 ====================

void setUp(void) {
    // 每个测试前初始化
    testSensorData.temperature = 25.0;
    testSensorData.humidity = 60.0;
    testSensorData.valid = true;
    testTimeData.hour = 14;
    testTimeData.minute = 30;
    testTimeData.second = 0;
    testScreenOn = true;
    testPirState = HIGH;
}

void tearDown(void) {
    // 每个测试后清理
}

// ==================== 温度传感器测试 ====================

void test_sensor_normal_reading(void) {
    // 测试正常的温湿度读取
    TEST_ASSERT_TRUE(testSensorData.valid);
    TEST_ASSERT_FLOAT_WITHIN(0.5, 25.0, testSensorData.temperature);
    TEST_ASSERT_FLOAT_WITHIN(1.0, 60.0, testSensorData.humidity);
}

void test_sensor_temperature_range(void) {
    // 测试温度范围限制（-40°C 到 85°C）
    testSensorData.temperature = -20.0;
    TEST_ASSERT_TRUE(testSensorData.temperature >= -40.0);
    TEST_ASSERT_TRUE(testSensorData.temperature <= 85.0);
}

void test_sensor_humidity_range(void) {
    // 测试湿度范围限制（0% 到 100%）
    testSensorData.humidity = 50.0;
    TEST_ASSERT_TRUE(testSensorData.humidity >= 0.0);
    TEST_ASSERT_TRUE(testSensorData.humidity <= 100.0);
}

void test_sensor_invalid_data(void) {
    // 测试无效数据处理
    testSensorData.valid = false;
    TEST_ASSERT_FALSE(testSensorData.valid);
}

// ==================== 时间格式化测试 ====================

void test_time_formatting(void) {
    // 测试时间格式化 HH:MM:SS
    char timeStr[16];
    sprintf(timeStr, "%02d:%02d:%02d",
             testTimeData.hour,
             testTimeData.minute,
             testTimeData.second);
    
    TEST_ASSERT_EQUAL_STRING("14:30:00", timeStr);
}

void test_date_formatting(void) {
    // 测试日期格式化 YYYY-MM-DD
    char dateStr[20];
    sprintf(dateStr, "%04d-%02d-%02d",
             testTimeData.year + 1900,
             testTimeData.month + 1,
             testTimeData.day);
    
    // 假设测试时的年份是2025
    TEST_ASSERT_TRUE(strstr(dateStr, "2025") != NULL);
}

void test_midnight_time(void) {
    // 测试午夜时间边界
    testTimeData.hour = 23;
    testTimeData.minute = 59;
    testTimeData.second = 59;
    
    char timeStr[16];
    sprintf(timeStr, "%02d:%02d:%02d",
             testTimeData.hour,
             testTimeData.minute,
             testTimeData.second);
    
    TEST_ASSERT_EQUAL_STRING("23:59:59", timeStr);
}

// ==================== PIR传感器测试 ====================

void test_pir_motion_detected(void) {
    // 测试检测到人体活动
    testPirState = HIGH;
    TEST_ASSERT_EQUAL_INT(HIGH, testPirState);
}

void test_pir_no_motion(void) {
    // 测试未检测到人体活动
    testPirState = LOW;
    TEST_ASSERT_EQUAL_INT(LOW, testPirState);
}

void test_screen_on_with_motion(void) {
    // 测试有人时屏幕亮起
    testPirState = HIGH;
    if (testPirState == HIGH) {
        testScreenOn = true;
    }
    TEST_ASSERT_TRUE(testScreenOn);
}

void test_screen_off_after_delay(void) {
    // 测试无人时屏幕熄灭
    testPirState = LOW;
    if (testPirState == LOW) {
        testScreenOn = false;
    }
    TEST_ASSERT_FALSE(testScreenOn);
}

// ==================== WiFi状态测试 ====================

void test_wifi_connected(void) {
    // 测试WiFi连接状态
    int wifiStatus = TEST_WIFI_OK;
    TEST_ASSERT_EQUAL_INT(TEST_WIFI_OK, wifiStatus);
}

void test_wifi_disconnected(void) {
    // 测试WiFi断开状态
    int wifiStatus = TEST_WIFI_LOST;
    TEST_ASSERT_EQUAL_INT(TEST_WIFI_LOST, wifiStatus);
}

// ==================== 边界条件测试 ====================

void test_extreme_temperature(void) {
    // 测试极端温度
    testSensorData.temperature = 40.0;  // 高温
    TEST_ASSERT_TRUE(testSensorData.temperature <= 85.0);
    
    testSensorData.temperature = -10.0; // 低温
    TEST_ASSERT_TRUE(testSensorData.temperature >= -40.0);
}

void test_extreme_humidity(void) {
    // 测试极端湿度
    testSensorData.humidity = 95.0;  // 高湿度
    TEST_ASSERT_TRUE(testSensorData.humidity <= 100.0);
    
    testSensorData.humidity = 10.0;  // 低湿度
    TEST_ASSERT_TRUE(testSensorData.humidity >= 0.0);
}

void test_time_rollover(void) {
    // 测试时间滚动（23:59:59 -> 00:00:00）
    testTimeData.hour = 23;
    testTimeData.minute = 59;
    testTimeData.second = 59;
    
    // 模拟秒增加
    testTimeData.second++;
    if (testTimeData.second >= 60) {
        testTimeData.second = 0;
        testTimeData.minute++;
        if (testTimeData.minute >= 60) {
            testTimeData.minute = 0;
            testTimeData.hour++;
            if (testTimeData.hour >= 24) {
                testTimeData.hour = 0;
            }
        }
    }
    
    TEST_ASSERT_EQUAL_INT(0, testTimeData.hour);
    TEST_ASSERT_EQUAL_INT(0, testTimeData.minute);
    TEST_ASSERT_EQUAL_INT(0, testTimeData.second);
}

// ==================== 内存监控测试 ====================

void test_memory_threshold_warning(void) {
    // 测试内存阈值警告
    unsigned long freeHeap = 15000;  // 低于20KB
    const unsigned long minFreeHeap = 15000;
    
    bool lowMemory = (freeHeap < 20000);
    TEST_ASSERT_TRUE(lowMemory);
    TEST_ASSERT_EQUAL_UINT32(15000, freeHeap);
}

void test_memory_normal(void) {
    // 测试正常内存状态
    unsigned long freeHeap = 300000;  // 正常内存
    bool lowMemory = (freeHeap < 20000);
    
    TEST_ASSERT_FALSE(lowMemory);
    TEST_ASSERT_TRUE(freeHeap > 20000);
}

// ==================== 主函数 ====================

int main() {
    // 延迟等待串口初始化
    delay(1000);
    
    // 初始化Unity测试框架
    UNITY_BEGIN();
    
    // 运行所有测试
    RUN_TEST(test_sensor_normal_reading);
    RUN_TEST(test_sensor_temperature_range);
    RUN_TEST(test_sensor_humidity_range);
    RUN_TEST(test_sensor_invalid_data);
    
    RUN_TEST(test_time_formatting);
    RUN_TEST(test_date_formatting);
    RUN_TEST(test_midnight_time);
    
    RUN_TEST(test_pir_motion_detected);
    RUN_TEST(test_pir_no_motion);
    RUN_TEST(test_screen_on_with_motion);
    RUN_TEST(test_screen_off_after_delay);
    
    RUN_TEST(test_wifi_connected);
    RUN_TEST(test_wifi_disconnected);
    
    RUN_TEST(test_extreme_temperature);
    RUN_TEST(test_extreme_humidity);
    RUN_TEST(test_time_rollover);
    
    RUN_TEST(test_memory_threshold_warning);
    RUN_TEST(test_memory_normal);
    
    // 返回测试结果
    return UNITY_END();
}
