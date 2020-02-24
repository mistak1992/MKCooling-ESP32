#ifndef _HALL_COUNTER_MODULE_H_
#define _HALL_COUNTER_MODULE_H_
#include "esp_system.h"

    void hallInit(uint8_t input_gpio);
    void hallStartCounter();
    void hallStopCounter();
    void hallClearCounter();
    int16_t hallGetCounter(); 

#endif