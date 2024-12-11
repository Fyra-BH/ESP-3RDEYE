#include "servo_group.h"
#include <thread>
#include <chrono>

#include "driver/ledc.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include <math.h>


static const char *TAG = "SERVO";

constexpr int SERVO_NUM = 3;
std::array<int, SERVO_NUM> SERVO_PULSE_GPIOS = {
    CONFIG_SERVO_PULSE_GPIO_PITCH,
    CONFIG_SERVO_PULSE_GPIO_ROLL,
    CONFIG_SERVO_PULSE_GPIO_YAW
};

ServoGroup &ServoGroup::GetInstance()
{
    static ServoGroup instance;
    return instance;
}

ServoGroup::ServoGroup()
{
    /*########################### PWM #################################*/
    // 对于ESP32C3，使用LEDC驱动PWM
    // Prepare and then apply the LEDC PWM timer configuration
    const std::array<int, SERVO_NUM> &gpio_array=SERVO_PULSE_GPIOS;
    
    ledc_timer_config_t ledc_timer;

    ledc_timer.speed_mode = LEDC_MODE;
    ledc_timer.duty_resolution = LEDC_DUTY_RES;
    ledc_timer.timer_num = LEDC_TIMER;
    ledc_timer.freq_hz = LEDC_FREQUENCY;  // Set output frequency at 50Hz
    ledc_timer.clk_cfg = LEDC_AUTO_CLK;

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel[SERVO_NUM];
    for (int i = 0; i < SERVO_NUM; i++) {
        ledc_channel[i].speed_mode     = LEDC_MODE;
        ledc_channel[i].channel        = static_cast<ledc_channel_t>(i);
        ledc_channel[i].timer_sel      = LEDC_TIMER;
        ledc_channel[i].intr_type      = LEDC_INTR_DISABLE;
        ledc_channel[i].gpio_num       = gpio_array[i];
        ledc_channel[i].duty           = 0; // Set duty to 0%
        ledc_channel[i].hpoint         = 0;
        
        ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel[i]));
    }
}

void ServoGroup::SetAngle(int servoIdx, float angle)
{
    if (servoIdx < 0 || servoIdx >= SERVO_NUM) {
        ESP_LOGE(TAG, "servoIdx out of range");
        return;
    }
    float duty = (angle * 2.0 / 180.0 + 0.5) / 20.0; // 将角度转换为占空比;20.0代表20ms
    int pulse_width = (uint32_t)(duty * (float)(1 << LEDC_DUTY_RES));
    ledc_set_duty(LEDC_MODE, static_cast<ledc_channel_t>(servoIdx), pulse_width);
    ledc_update_duty(LEDC_MODE, static_cast<ledc_channel_t>(servoIdx));
}

// 五次插值函数，输入为目标舵机、初始角度、目标角度
void ServoGroup::FiveTimesInterpolation(int servoIdx, float angleStart, float angleEnd, float duration)
{
    // 定义插值时间总长度 (us)，可以根据需求调整
    float T = duration * 1000.0 * 1000.0; // 插值时间为2秒 (2000_000 us)
    int steps = 50;   // 插值步数，越多越平滑
    float dt = T / steps;

    // 五次插值多项式系数
    float a0 = angleStart;
    float a1 = 0;
    float a2 = 0;
    float a3 = 10 * (angleEnd - angleStart) / (T * T * T);
    float a4 = -15 * (angleEnd - angleStart) / (T * T * T * T);
    float a5 = 6 * (angleEnd - angleStart) / (T * T * T * T * T);

    // 插值过程
    for (int i = 0; i <= steps; i++) {
        float t = i * dt; // 当前时间
        float theta = a0 + a1 * t + a2 * t * t + a3 * t * t * t + a4 * t * t * t * t + a5 * t * t * t * t * t;
        
        // 将插值角度设置到舵机
        SetAngle(servoIdx, theta);
        std::this_thread::sleep_for(std::chrono::microseconds(static_cast<int>(dt)));
    }
}
