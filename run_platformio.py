#!/usr/bin/env python3
"""
临时脚本：禁用SSL验证后运行PlatformIO
"""
import os
import ssl
import sys
import urllib3

# 禁用SSL警告
urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

# 创建不验证SSL的上下文
ssl._create_default_https_context = ssl._create_unverified_context

# 设置环境变量
os.environ['PYTHONHTTPSVERIFY'] = '0'

# 运行PlatformIO
import subprocess
result = subprocess.run(['platformio', 'run'] + sys.argv[1:], shell=True)
sys.exit(result.returncode)
