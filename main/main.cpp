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
    std::vector<std::thread> threads;
    
    auto cfg = esp_pthread_get_default_config();
    cfg.stack_size = 8192;
    cfg.prio = 5;
    esp_pthread_set_cfg(&cfg);
    threads.push_back(std::thread([&] {
        while (true) {
            UdpServer UdpServer(CONFIG_EXAMPLE_PORT);
            UdpServer.StartListening();   
        }
    }));

    threads.push_back(std::thread([&]{
        while (true) {
            std::stringstream ss;
            ss << "core id: " << xPortGetCoreID()
            << ", prio: " << uxTaskPriorityGet(nullptr)
            << ", minimum free stack: " << uxTaskGetStackHighWaterMark(nullptr) << " bytes.";
            ESP_LOGI(pcTaskGetName(nullptr), "%s", ss.str().c_str());
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }));
    for (auto &t : threads) {
        t.join();
    }
}