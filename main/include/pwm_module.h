#ifndef _PWM_MODULE_H_
#define _PWM_MODULE_H_
#include "esp_system.h"

    void pwm_init(uint8_t output_gpio);
    void fan_set_duty(float duty_cycle);
    float fan_get_duty();
    void fan_set_stop();
    void fan_set_start();

#endif
