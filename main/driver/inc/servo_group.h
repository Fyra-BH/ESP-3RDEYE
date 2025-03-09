#ifndef SERVO_H
#define SERVO_H

#include <array>
#include <map>
#include "sdkconfig.h"
#include "servor_input_adapter.h"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_14_BIT // Set duty resolution to 14 bits
#define LEDC_FREQUENCY          (50) // Frequency in Hertz. Set frequency at 50 Hz

enum ServoIdx : int {
    SERVO_IDX_CH1 = 0,
    SERVO_IDX_CH2 = 1,
    SERVO_IDX_CH3 = 2,
    SERVO_MAX_NUM = 3
};

using AngleLimits = std::pair<float, float>;
using ActionInstruction = std::pair<float, float>[SERVO_MAX_NUM];

class ServoGroup
{
public:
    static ServoGroup &GetInstance(); // Singleton
    void SetAngle(int servoIdx, float theta);
    void SetAngleSmooth(const ActionInstruction &instruction, float duration);
    void SetServoDataPreprocessor(int servoIdx, ServoDataConfig *servoDataPreprocessor);
    void FiveTimesInterpolation(int servoIdx, float angleStart, float angleEnd, float duration);
    void MultiServoInterpolation(const ActionInstruction &instruction, float duration);
private:
    ServoGroup();
    std::map<int, ServoDataConfig*> m_servoDataPreprocessorMap;
private:
};

void servo_tasks_init();
void fivetimesInterpolation(int myservo, float thetai, float thetaf, float t);
void put_servo_data(int servo_num, float start, float end, float T);

#endif // SERVO_H
