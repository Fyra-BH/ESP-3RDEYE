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

// 舵机数据预处理配置
constexpr float SERVO_SCALE_PITCH = 1.0f / 3.0f;
constexpr float SERVO_OFFSET_PITCH = 110.0f;
constexpr float SERVO_ZEROPOINT_PITCH = 60.0f;
constexpr float SERVO_MIN_ANGLE_PITCH = 0.0f;
constexpr float SERVO_MAX_ANGLE_PITCH = 90.0f;
constexpr bool SERVO_IS_REVERSE_PITCH = true;

constexpr float SERVO_SCALE_ROLL = 1.0f / 3.0;
constexpr float SERVO_OFFSET_ROLL = 0.0f;
constexpr float SERVO_ZEROPOINT_ROLL = 90.0f;
constexpr float SERVO_MIN_ANGLE_ROLL = 45.0f;
constexpr float SERVO_MAX_ANGLE_ROLL = 135.0f;
constexpr bool SERVO_IS_REVERSE_ROLL = true;

constexpr float SERVO_SCALE_YAW = 1.0f / 3.0f;
constexpr float SERVO_OFFSET_YAW = -120.0f;
constexpr float SERVO_ZEROPOINT_YAW = 120.0f;
constexpr float SERVO_MIN_ANGLE_YAW = 120.0f;
constexpr float SERVO_MAX_ANGLE_YAW = 180.0f;
constexpr bool SERVO_IS_REVERSE_YAW = true;

namespace {

constexpr ServoDataConfig SERVO_CONFIG_PITCH {
    SERVO_SCALE_PITCH,
    SERVO_OFFSET_PITCH,
    SERVO_ZEROPOINT_PITCH,
    SERVO_MIN_ANGLE_PITCH,
    SERVO_MAX_ANGLE_PITCH,
    SERVO_IS_REVERSE_PITCH
};

const ServoDataConfig SERVO_CONFIG_ROLL {
    SERVO_SCALE_ROLL,
    SERVO_OFFSET_ROLL,
    SERVO_ZEROPOINT_ROLL,
    SERVO_MIN_ANGLE_ROLL,
    SERVO_MAX_ANGLE_ROLL,
    SERVO_IS_REVERSE_ROLL
};

constexpr ServoDataConfig SERVO_CONFIG_YAW {
    SERVO_SCALE_YAW,
    SERVO_OFFSET_YAW,
    SERVO_ZEROPOINT_YAW,
    SERVO_MIN_ANGLE_YAW,
    SERVO_MAX_ANGLE_YAW,
    SERVO_IS_REVERSE_YAW
};
}

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
    ledc_timer.deconfigure = false;

    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));
    
    ledc_channel_config_t ledc_channel;
    ledc_channel.speed_mode     = LEDC_MODE;
    ledc_channel.timer_sel      = LEDC_TIMER;
    ledc_channel.intr_type      = LEDC_INTR_DISABLE;
    ledc_channel.duty           = 0; // Set duty to 0%
    ledc_channel.hpoint         = 0;

    ledc_channel.gpio_num = gpio_array[SERVO_IDX_PITCH];
    ledc_channel.channel = static_cast<ledc_channel_t>(SERVO_IDX_PITCH);
    ledc_channel_config(&ledc_channel);
    SetServoDataPreprocessor(SERVO_IDX_PITCH, SERVO_CONFIG_PITCH);

    ledc_channel.gpio_num = gpio_array[SERVO_IDX_ROLL];
    ledc_channel.channel = static_cast<ledc_channel_t>(SERVO_IDX_ROLL);
    ledc_channel_config(&ledc_channel);
    SetServoDataPreprocessor(SERVO_IDX_ROLL, SERVO_CONFIG_ROLL);

    ledc_channel.gpio_num = gpio_array[SERVO_IDX_YAW];
    ledc_channel.channel = static_cast<ledc_channel_t>(SERVO_IDX_YAW);
    ledc_channel_config(&ledc_channel);
    SetServoDataPreprocessor(SERVO_IDX_YAW, SERVO_CONFIG_YAW);
}

/**
 * @brief 设置舵机的角度
 * 
 * @param servoIdx 舵机索引
 * @param theta 舵机角度
 */
void ServoGroup::SetAngle(int servoIdx, float theta)
{
    if (servoIdx < 0 || servoIdx >= SERVO_NUM) {
        ESP_LOGE(TAG, "servoIdx out of range");
        return;
    }
    float scale = m_servoDataPreprocessorMap[servoIdx].scale;
    float offset = m_servoDataPreprocessorMap[servoIdx].offset;
    float zeroPoint = m_servoDataPreprocessorMap[servoIdx].zeroPoint;
    float minAngle = m_servoDataPreprocessorMap[servoIdx].minAngle;
    float maxAngle = m_servoDataPreprocessorMap[servoIdx].maxAngle;
    bool isReverse = m_servoDataPreprocessorMap[servoIdx].isReverse;

    theta = isReverse ? 180 - theta : theta;
    theta = ((theta - offset) - zeroPoint) * scale + zeroPoint;

    if (theta > maxAngle) {
        theta = maxAngle;
    } else if (theta < minAngle) {
        theta = minAngle;
    }
    float duty = (theta * 2.0 / 180.0 + 0.5) / 20.0; // 将角度转换为占空比;20.0代表20ms
    int pulse_width = (uint32_t)(duty * (float)(1 << LEDC_DUTY_RES));
    // ESP_LOGI(TAG, "servoIdx: %d, theta: %f, duty: %f, pulse_width: %d", servoIdx, theta, duty, pulse_width);
    ledc_set_duty(LEDC_MODE, static_cast<ledc_channel_t>(servoIdx), pulse_width);
    ledc_update_duty(LEDC_MODE, static_cast<ledc_channel_t>(servoIdx));
}


/**
 * @brief 设置舵机的预处理参数
 * 
 * @param servoIdx 舵机索引
 * @param servoDataPreprocessor 舵机预处理配置
 */
void ServoGroup::SetServoDataPreprocessor(int servoIdx, const ServoDataConfig &servoDataPreprocessor)
{
    m_servoDataPreprocessorMap.insert({servoIdx, servoDataPreprocessor});
}

/**
 * @brief 五次插值函数，输入为目标舵机、初始角度、目标角度
 * 
 * @param servoIdx 舵机索引
 * @param angleStart 初始角度
 * @param angleEnd 目标角度
 * @param duration 插值时间 (s)
 */
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
