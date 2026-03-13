#!/usr/bin/env python3
"""
手动下载PlatformIO所需文件（绕过SSL验证）
"""
import os
import sys
import urllib.request
import ssl

# 禁用SSL验证
ssl._create_default_https_context = ssl._create_unverified_context

# 创建缓存目录
cache_dir = r"C:\Users\Administrator\.platformio\.cache"
os.makedirs(cache_dir, exist_ok=True)

# 下载列表
downloads = [
    {
        "url": "https://github.com/espressif/arduino-esp32/releases/download/3.3.7/esp32-core-3.3.7.tar.xz",
        "filename": "esp32-core-3.3.7.tar.xz"
    },
    {
        "url": "https://github.com/pioarduino/esp_install/releases/download/v5.3.4/esp_install-v5.3.4.zip",
        "filename": "esp_install-v5.3.4.zip"
    }
]

print("=" * 60)
print("开始下载PlatformIO所需文件...")
print("=" * 60)

for download in downloads:
    url = download["url"]
    filename = download["filename"]
    filepath = os.path.join(cache_dir, filename)

    print(f"\n正在下载: {filename}")
    print(f"URL: {url}")

    try:
        # 下载文件
        urllib.request.urlretrieve(url, filepath)
        print(f"✓ 下载成功: {filepath}")
        print(f"文件大小: {os.path.getsize(filepath)} 字节")
    except Exception as e:
        print(f"✗ 下载失败: {e}")

print("\n" + "=" * 60)
print("下载完成!")
print("=" * 60)
print(f"\n文件已下载到: {cache_dir}")
print("\n现在请手动将文件移动到正确的PlatformIO缓存目录:")
print("1. esp32-core-3.3.7.tar.xz -> %USERPROFILE%\\.platformio\\.cache\\platformio\\packages\\framework-arduinoespressif32\\")
print("2. esp_install-v5.3.4.zip -> %USERPROFILE%\\.platformio\\.cache\\platformio\\packages\\tool-esp_install\\")
print("\n然后运行: platformio run")
