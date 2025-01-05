import configparser
import os

# Create a ConfigParser object
config = configparser.ConfigParser()

# Read the configuration file with UTF-8 encoding
config.read('config.param', encoding='utf-8')

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

# Flash the firmware
ESP_TOOL_EXE = os.path.abspath('esptool.exe')
'''
esptool.main([
    '--chip', 'esp32c3',
    '--port', esp_com_port,
    '--baud', '460800',
    '--before', 'default_reset',
    '--after', 'hard_reset',
    'write_flash',
    '--flash_mode', 'dio',
    '--flash_freq', '80m',
    '--flash_size', '4MB',
    '0x0', 'bootloader.bin',
    '0x10000', 'app.bin',
    '0x8000', 'partition-table.bin'
])
'''

os.system(f'{ESP_TOOL_EXE} --chip esp32c3 --port {esp_com_port} --baud 460800 write_flash --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 bootloader.bin 0x10000 app.bin 0x8000 partition-table.bin')