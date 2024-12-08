#ifndef SERVO_H
#define SERVO_H

#include <array>
#include "sdkconfig.h"


#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_DUTY_RES           LEDC_TIMER_14_BIT // Set duty resolution to 14 bits
#define LEDC_FREQUENCY          (50) // Frequency in Hertz. Set frequency at 50 Hz

enum ServoIdx : int {
    SERVO_IDX_PITCH = 0,
    SERVO_IDX_ROW = 1,
    SERVO_IDX_YAW = 2,
};


class ServoGroup
{
public:
    ServoGroup();
    void SetAngle(int servoIdx, float angle);
    void FiveTimesInterpolation(int servoIdx, float angleStart, float angleEnd, float duration);
};

void servo_tasks_init();
void fivetimesInterpolation(int myservo, float thetai, float thetaf, float t);
void put_servo_data(int servo_num, float start, float end, float T);

#endif // SERVO_H
