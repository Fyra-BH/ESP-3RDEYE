#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "led_strip.h"


class LedController {
public:
    static LedController& GetInstance();
    void SetColor(int red, int green, int blue);
private:
    LedController();
    led_strip_handle_t m_led_strip;

};

#endif // LED_H
