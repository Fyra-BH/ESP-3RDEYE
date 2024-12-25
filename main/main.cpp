/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <vector>
#include <chrono>
#include <stdio.h>
#include <inttypes.h>
#include <esp_pthread.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "connect_wifi.h"
#include "servo_group.h"
#include "udp_server.h"
#include "led_controller.h"
#include "esp_thread_helper.h"

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
    LedController::GetInstance().SetColor(50, 0, 0);
    connect_wifi();
    LedController::GetInstance().SetColor(0, 0, 50);
    
    EspThreadHelper th;
    th.AddTask([&] {
        while (true) {
            UdpServer UdpServer(CONFIG_EXAMPLE_PORT);
            UdpServer.StartListening();   
        }
    }, 8192);

    th.AddTask([&] {
        constexpr size_t LOOP_COUNT = 1000;
        for (size_t i = 0; i < LOOP_COUNT; ++i) {
            ESP_LOGI(TAG, "LED BLINK");
            LedController::GetInstance().SetColor(0, 50, 0);
            std::this_thread::sleep_for(std::chrono::microseconds(500));
            LedController::GetInstance().SetColor(0, 0, 50);
            std::this_thread::sleep_for(std::chrono::microseconds(500));
        }
        ESP_LOGI(TAG, "LED BLINK DONE");
    });

    th.AddTask([&] {
        while (true) {
            th.PrintInfo();
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    });

}