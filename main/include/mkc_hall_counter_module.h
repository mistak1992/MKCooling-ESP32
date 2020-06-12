#ifndef _MKC_HALL_COUNTER_MODULE_H_
#define _MKC_HALL_COUNTER_MODULE_H_
#include "esp_system.h"

    void mkc_hallInit(uint8_t input_gpio);
    void mkc_hallStartCounter();
    void mkc_hallStopCounter();
    void mkc_hallClearCounter();
    int16_t mkc_hallGetCounter(); 

#endif