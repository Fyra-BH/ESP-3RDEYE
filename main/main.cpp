/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <vector>
#include <thread>
#include <chrono>
#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "servo.h"
#include "connect_wifi.h"
#include "remote_proc.h"

static const char *TAG = "APP MAIN";


void greeting(void)
{
    printf("Hello world!\n");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("silicon revision v%d.%d, ", major_rev, minor_rev);
    if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

}

extern "C" void app_main(void)
{
    greeting();
    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    connect_wifi();
    ServoGroup servoGroup;
    std::vector<std::thread> threads;
    threads.push_back(std::thread([&]{
        while (true) {
            servoGroup.FiveTimesInterpolation(SERVO_IDX_PITCH, 0, 180, 1.5);
            servoGroup.FiveTimesInterpolation(SERVO_IDX_PITCH, 180, 0, 1.5);
        }
    }));

    {
        RemoteProc remoteProc(3333);
        for (int i = 0; i < 2; i++)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            uint8_t dst_ip[4] = {192, 168, 31, 146};
            std::initializer_list<std::string> data_to_send = {
                "Hello",
                "World",
                "From",
                "ESP32"
            };
            for (auto &data : data_to_send) {
                remoteProc.SendBytes(data + "\n", dst_ip, 3333);
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
        uint8_t dst_ip[4] = {192, 168, 31, 255};
        remoteProc.SendBytes("Broadcast Message", dst_ip, 3333);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    for (auto &tid : threads) {
        tid.join();
    }
}