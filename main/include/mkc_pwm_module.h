#ifndef _MKC_PWM_MODULE_H_
#define _MKC_PWM_MODULE_H_
#include "esp_system.h"

    void mkc_pwm_init(uint8_t output_gpio);
    void mkc_fan_set_duty(float duty_cycle);
    float mkc_fan_get_duty();
    void mkc_fan_set_stop();
    void mkc_fan_set_start();

#endif
