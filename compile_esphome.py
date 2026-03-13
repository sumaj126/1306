import ssl
import os

# 禁用SSL证书验证
ssl._create_default_https_context = ssl._create_unverified_context

# 设置环境变量
os.environ['PYTHONHTTPSVERIFY'] = '0'
os.environ['CURL_CA_BUNDLE'] = ''
os.environ['REQUESTS_CA_BUNDLE'] = ''
os.environ['SSL_CERT_FILE'] = ''

# 运行esphome命令
import subprocess
import sys

config_file = r'd:\ESP32\ESP32_1306\esp32-temperature-monitor.yaml'
result = subprocess.run([
    sys.executable, '-m', 'esphome', 'compile', config_file
], env=os.environ)

sys.exit(result.returncode)
