/**
 * @file servor_input_adapter.h
 * @author Fyra-BH (2472708282@qq.com)
 * @brief 为什么要做这个adapter？因为之前脑子抽了，把三路舵机的输入揉到了一个报文, 而写用的是us脉宽的格式, 
 *        后来需求变化, 要改用传输角度了，不得已做了这个适配器
 * @version 0.1
 * @date 2024-12-11
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef SERVO_INPUT_ADAPTER_H
#define SERVO_INPUT_ADAPTER_H

class ServoInputAdapter {
public:
    inline float PulseWidth2Angle(int pulseWidth)
    { // 500us ~ 2500us
        return (pulseWidth - 500.0) / 2000.0 * 180.0;
    }
    inline int Angle2PulseWidth(float angle)
    { // 500us ~ 2500us
        return angle / 180.0 * 2000.0 + 500;
    }
    inline float DutyCycle2Angle(float dutyCycle)
    { // 0.025 ~ 0.125
        return (dutyCycle - 0.025) / 0.1 * 180.0;
    }
};

#endif