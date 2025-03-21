#include "servo_group.h"
#include <thread>
#include <chrono>

#include "driver/ledc.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_partition_param.h"
#include "var_hacker.hpp"

static const char *TAG = "SERVO";

constexpr int SERVO_NUM = 3;

// 舵机数据预处理配置
float SERVO_SCALE_CH1 = 1.0f / 3.0f;
float SERVO_OFFSET_CH1 = 0.0f;
float SERVO_ZEROPOINT_CH1 = 90.0f;
float SERVO_MIN_ANGLE_CH1 = 45.0f;
float SERVO_MAX_ANGLE_CH1 = 135.0f;
bool SERVO_IS_REVERSE_CH1 = true;

float SERVO_SCALE_CH2 = 1.0f / 3.0;
float SERVO_OFFSET_CH2 = 40.0f;
float SERVO_ZEROPOINT_CH2 = 60.0f;
float SERVO_MIN_ANGLE_CH2 = 0.0f;
float SERVO_MAX_ANGLE_CH2 = 90.0f;
bool SERVO_IS_REVERSE_CH2 = true;

float SERVO_SCALE_CH3 = 1.0f / 3.0f;
float SERVO_OFFSET_CH3 = -40.0f;
float SERVO_ZEROPOINT_CH3 = 120.0f;
float SERVO_MIN_ANGLE_CH3 = 120.0f;
float SERVO_MAX_ANGLE_CH3 = 180.0f;
bool SERVO_IS_REVERSE_CH3 = true;

namespace {

ServoDataConfig g_ServoConfigCH1;
ServoDataConfig g_ServoConfigCH2;
ServoDataConfig g_ServoConfigCH3;
std::array<int, SERVO_NUM> g_ServoPwmGpios;

void SetUpServoDataPreprocessor()
{
    g_ServoConfigCH1 = {
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_SCALE_CH1", SERVO_SCALE_CH1),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_OFFSET_CH1", SERVO_OFFSET_CH1),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_ZEROPOINT_CH1", SERVO_ZEROPOINT_CH1),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_MIN_ANGLE_CH1", SERVO_MIN_ANGLE_CH1),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_MAX_ANGLE_CH1", SERVO_MAX_ANGLE_CH1),
        EspPartitionParam::GetInstance().GetBoolParam("SERVO_IS_REVERSE_CH1", SERVO_IS_REVERSE_CH1)
    };

    g_ServoConfigCH2 = {
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_SCALE_CH2", SERVO_SCALE_CH2),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_OFFSET_CH2", SERVO_OFFSET_CH2),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_ZEROPOINT_CH2", SERVO_ZEROPOINT_CH2),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_MIN_ANGLE_CH2", SERVO_MIN_ANGLE_CH2),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_MAX_ANGLE_CH2", SERVO_MAX_ANGLE_CH2),
        EspPartitionParam::GetInstance().GetBoolParam("SERVO_IS_REVERSE_CH2", SERVO_IS_REVERSE_CH2)
    };

    g_ServoConfigCH3 = {
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_SCALE_CH3", SERVO_SCALE_CH3),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_OFFSET_CH3", SERVO_OFFSET_CH3),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_ZEROPOINT_CH3", SERVO_ZEROPOINT_CH3),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_MIN_ANGLE_CH3", SERVO_MIN_ANGLE_CH3),
        EspPartitionParam::GetInstance().GetFloatParam("SERVO_MAX_ANGLE_CH3", SERVO_MAX_ANGLE_CH3),
        EspPartitionParam::GetInstance().GetBoolParam("SERVO_IS_REVERSE_CH3", SERVO_IS_REVERSE_CH3)
    };

    g_ServoPwmGpios = {
        EspPartitionParam::GetInstance().GetIntParam("SERVO_PULSE_GPIO_CH1", CONFIG_SERVO_PULSE_GPIO_CH1),
        EspPartitionParam::GetInstance().GetIntParam("SERVO_PULSE_GPIO_CH2", CONFIG_SERVO_PULSE_GPIO_CH2),
        EspPartitionParam::GetInstance().GetIntParam("SERVO_PULSE_GPIO_CH3", CONFIG_SERVO_PULSE_GPIO_CH3)
    };
}

void SetUpServoVarHacker()
{
    VH::VarHacker::GetInstance().RegisterVar("CH1.scale", &g_ServoConfigCH1.scale);
    VH::VarHacker::GetInstance().RegisterVar("CH1.offset", &g_ServoConfigCH1.offset);
    VH::VarHacker::GetInstance().RegisterVar("CH1.zeroPoint", &g_ServoConfigCH1.zeroPoint);
    VH::VarHacker::GetInstance().RegisterVar("CH1.minAngle", &g_ServoConfigCH1.minAngle);
    VH::VarHacker::GetInstance().RegisterVar("CH1.maxAngle", &g_ServoConfigCH1.maxAngle);
    VH::VarHacker::GetInstance().RegisterVar("CH1.isReverse", &g_ServoConfigCH1.isReverse);

    VH::VarHacker::GetInstance().RegisterVar("CH2.scale", &g_ServoConfigCH2.scale);
    VH::VarHacker::GetInstance().RegisterVar("CH2.offset", &g_ServoConfigCH2.offset);
    VH::VarHacker::GetInstance().RegisterVar("CH2.zeroPoint", &g_ServoConfigCH2.zeroPoint);
    VH::VarHacker::GetInstance().RegisterVar("CH2.minAngle", &g_ServoConfigCH2.minAngle);
    VH::VarHacker::GetInstance().RegisterVar("CH2.maxAngle", &g_ServoConfigCH2.maxAngle);
    VH::VarHacker::GetInstance().RegisterVar("CH2.isReverse", &g_ServoConfigCH2.isReverse);

    VH::VarHacker::GetInstance().RegisterVar("CH3.scale", &g_ServoConfigCH3.scale);
    VH::VarHacker::GetInstance().RegisterVar("CH3.offset", &g_ServoConfigCH3.offset);
    VH::VarHacker::GetInstance().RegisterVar("CH3.zeroPoint", &g_ServoConfigCH3.zeroPoint);
    VH::VarHacker::GetInstance().RegisterVar("CH3.minAngle", &g_ServoConfigCH3.minAngle);
    VH::VarHacker::GetInstance().RegisterVar("CH3.maxAngle", &g_ServoConfigCH3.maxAngle);
    VH::VarHacker::GetInstance().RegisterVar("CH3.isReverse", &g_ServoConfigCH3.isReverse);
}

}


ServoGroup &ServoGroup::GetInstance()
{
    static ServoGroup instance;
    return instance;
}

ServoGroup::ServoGroup()
{
    SetUpServoDataPreprocessor();
    SetUpServoVarHacker();
    /*########################### PWM #################################*/
    // 对于ESP32C3，使用LEDC驱动PWM
    // Prepare and then apply the LEDC PWM timer configuration
    const std::array<int, SERVO_NUM> &gpio_array=g_ServoPwmGpios;
    
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

    ledc_channel.gpio_num = gpio_array[SERVO_IDX_CH1];
    ledc_channel.channel = static_cast<ledc_channel_t>(SERVO_IDX_CH1);
    ledc_channel_config(&ledc_channel);
    SetServoDataPreprocessor(SERVO_IDX_CH1, &g_ServoConfigCH1);

    ledc_channel.gpio_num = gpio_array[SERVO_IDX_CH2];
    ledc_channel.channel = static_cast<ledc_channel_t>(SERVO_IDX_CH2);
    ledc_channel_config(&ledc_channel);
    SetServoDataPreprocessor(SERVO_IDX_CH2, &g_ServoConfigCH2);

    ledc_channel.gpio_num = gpio_array[SERVO_IDX_CH3];
    ledc_channel.channel = static_cast<ledc_channel_t>(SERVO_IDX_CH3);
    ledc_channel_config(&ledc_channel);
    SetServoDataPreprocessor(SERVO_IDX_CH3, &g_ServoConfigCH3);
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
    float scale = m_servoDataPreprocessorMap[servoIdx]->scale;
    float offset = m_servoDataPreprocessorMap[servoIdx]->offset;
    float zeroPoint = m_servoDataPreprocessorMap[servoIdx]->zeroPoint;
    float minAngle = m_servoDataPreprocessorMap[servoIdx]->minAngle;
    float maxAngle = m_servoDataPreprocessorMap[servoIdx]->maxAngle;
    bool isReverse = m_servoDataPreprocessorMap[servoIdx]->isReverse;

    theta = isReverse ? 180 - theta : theta;
    theta = (theta - zeroPoint) * scale + zeroPoint - offset;

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
void ServoGroup::SetServoDataPreprocessor(int servoIdx, ServoDataConfig *servoDataPreprocessor)
{
    m_servoDataPreprocessorMap[servoIdx] = servoDataPreprocessor;
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
    // 定义插值时间总长度 (从s转换为ms)，可以根据需求调整
    float T = duration * 1000.0;
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
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(dt)));
    }
}

void ServoGroup::MultiServoInterpolation(
    const ActionInstruction& instruction,
    float duration)
{
    // 时间参数计算
    const float T = duration * 1000.0f;  // 总时长（毫秒）
    const int steps = 50;                // 插值步数
    const float dt = T / steps;          // 时间间隔

    // 运动轨迹参数容器
    struct TrajectoryParams {
        float a0, a3, a4, a5;
        bool active = false;  // 标记是否需要运动
    };
    
    std::array<TrajectoryParams, SERVO_MAX_NUM> params;

    // 预计算各通道参数
    for (int ch = 0; ch < SERVO_MAX_NUM; ++ch) {
        auto&& [start, end] = instruction[ch];
        
        // 跳过无效指令（起点终点相同）
        if (std::abs(start - end) < 1e-6) {
            params[ch].active = false;
            continue;
        }

        const float delta = end - start;
        params[ch] = {
            start,                           // a0
            10 * delta / (T*T*T),           // a3
            -15 * delta / (T*T*T*T),        // a4
            6 * delta / (T*T*T*T*T),        // a5
            true                            // active
        };
    }

    // 运动执行循环
    for (int step = 0; step <= steps; ++step) {
        const float t = step * dt;
        
        // 并行更新所有有效舵机
        for (int ch = 0; ch < SERVO_MAX_NUM; ++ch) {
            if (!params[ch].active) continue;

            const auto& p = params[ch];
            const float theta = p.a0 
                + p.a3 * t*t*t 
                + p.a4 * t*t*t*t 
                + p.a5 * t*t*t*t*t;
            
            SetAngle(ch, theta);  // 直接使用枚举索引
        }
        // 精确时间控制
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(dt)));
    }
}