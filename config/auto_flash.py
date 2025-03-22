import os
import subprocess
import flash_firmware
import write_config
import configparser

ESPTOOL_EXE = os.path.abspath("esptool.exe")

def exec(cmd):
    print(cmd)
    return subprocess.run(cmd, capture_output=True, shell=True)

def checkComPort():
    cmd = [ESPTOOL_EXE, "chip_id"]
    ret = exec(cmd)
    print(ret)
    if ret.returncode != 0:
        print("No ESP32 found")
        return
    
    msg = ret.stdout.decode("utf-8").split("\r\n")
    # find 'Serial port COM10'
    for i in msg:
        if "Serial port" in i:
            port = i.split(" ")[-1]
            break
    if port is None:
        print("No ESP32 found")
        return
    print(port)
    return port

def configGen():
    config = configparser.ConfigParser()
    config.optionxform = str  # 保留键的大小写
    config.read('config_template.ini', encoding='utf-8')
    # get the COM port
    port = checkComPort()
    if port is None:
        return
    config.set('DEFAULT', 'ESP_COM_PORT', port)
    # input the Wi-Fi name and password
    wifi_ssid = input("Enter the Wi-Fi SSID: ")
    wifi_password = input("Enter the Wi-Fi password: ")
    config.set('DEFAULT', 'ESP_WIFI_SSID', wifi_ssid)
    config.set('DEFAULT', 'ESP_WIFI_PASSWORD', wifi_password)
    # write the config file
    with open('config.ini', 'w', encoding='utf-8') as configfile:
        config.write(configfile)

def main():
    import sys
    if len(sys.argv) > 1:
        if sys.argv[1] == 'config':
            write_config.main()
            return
    configGen()
    flash_firmware.main()
    write_config.main()

if __name__ == '__main__':
    main()