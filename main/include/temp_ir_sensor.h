#ifndef _TEMP_IR_SENSOR_H_
#define _TEMP_IR_SENSOR_H_
#include "driver/gpio.h"

    bool ir_temp_Init(gpio_num_t sda_gpio, gpio_num_t scl_gpio);
    float getAmbientTemp();
    float getObjectTemp();

#endif
