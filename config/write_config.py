import configparser
import os

# Create a ConfigParser object
config = configparser.ConfigParser()

# Read the configuration file with UTF-8 encoding
config.read('config.ini', encoding='utf-8')

# Get the Wi-Fi name and password
wifi_ssid = config.get('DEFAULT', 'ESP_WIFI_SSID')
wifi_password = config.get('DEFAULT', 'ESP_WIFI_PASSWORD')

# Output the read values
print(f"WiFi SSID: {wifi_ssid}")
print(f"WiFi Password: {wifi_password}")

# Get the COM port
esp_com_port = config.get('DEFAULT', 'ESP_COM_PORT')

# Output the read values
print(f"ESP COM Port: {esp_com_port}")

# Flash the config
ESP_TOOL_EXE = os.path.abspath('esptool.exe')
open('config.bin', 'wb').write(open('config.param', 'rb').read() + b'\0')
# esptool.main(['--chip', 'esp32c3', '--port', esp_com_port, 'write_flash', ' 0x150000', 'config.bin'])
os.system(f'{ESP_TOOL_EXE} --chip esp32c3 --port {esp_com_port} write_flash 0x150000 config.bin')