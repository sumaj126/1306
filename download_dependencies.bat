@echo off
echo ============================================================
echo PlatformIO 缺失文件下载指南
echo ============================================================
echo.
echo 由于SSL证书验证失败，需要手动下载以下文件：
echo.
echo 1. ESP32 Core (必需):
echo    下载地址: https://github.com/espressif/arduino-esp32/releases/download/3.3.7/esp32-core-3.3.7.tar.xz
echo    保存位置: %USERPROFILE%\.platformio\.cache\platformio\packages\framework-arduinoespressif32\
echo.
echo 2. ESP安装工具 (可选):
echo    下载地址: https://github.com/pioarduino/esp_install/releases/download/v5.3.4/esp_install-v5.3.4.zip
echo    保存位置: %USERPROFILE%\.platformio\.cache\platformio\packages\tool-esp_install\
echo.
echo ============================================================
echo 使用说明:
echo 1. 使用浏览器访问上述下载链接
echo 2. 创建对应的目录（如果不存在）
echo 3. 将下载的文件放到指定目录
echo 4. 然后运行: platformio run
echo ============================================================
echo.
pause
