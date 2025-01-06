#include <cstring>
#include "led_controller.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "esp_partition_param.h"

static const char *TAG = "led_controller";

LedController &LedController::GetInstance()
{
    static LedController instance;
    return instance;
}

LedController::LedController()
{
    ESP_LOGI(TAG, "Example configured to blink addressable LED!");
    led_strip_config_t strip_config;
    led_strip_rmt_config_t rmt_config;
    memset(&strip_config, 0, sizeof(led_strip_config_t));
    memset(&rmt_config, 0, sizeof(led_strip_rmt_config_t));

    strip_config.strip_gpio_num = 
        EspPartitionParam::GetInstance().GetIntParam("LED_GPIO", CONFIG_BLINK_GPIO);
    strip_config.max_leds = 1; // at least one LED on board

    rmt_config.resolution_hz = 10 * 1000 * 1000; // 10MHz
    rmt_config.flags.with_dma = false;

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &m_led_strip));
        /* Set all LED off to clear all pixels */
    led_strip_clear(m_led_strip);

}


void LedController::SetColor(int red, int green, int blue)
{
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    led_strip_set_pixel(m_led_strip, 0, green, red, blue);
    /* Refresh the strip to send data */
    led_strip_refresh(m_led_strip);
}