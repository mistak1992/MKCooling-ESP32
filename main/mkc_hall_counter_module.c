#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"
#include "esp_attr.h"
#include "esp_log.h"

pcnt_unit_t hallPcntUint = PCNT_UNIT_0;

void mkc_hallInit(uint8_t input_gpio) {
    pcnt_config_t pcnt_config = {
        // Set PCNT input signal and control GPIOs
        .pulse_gpio_num = input_gpio,
        .ctrl_gpio_num = -1,
        .channel = PCNT_CHANNEL_0,
        .unit = PCNT_UNIT_0,
        // What to do on the positive / negative edge of pulse input?
        .pos_mode = PCNT_COUNT_INC,   // Count up on the positive edge
        .neg_mode = PCNT_COUNT_DIS,   // Keep the counter value on the negative edge
        // What to do when control input is low or high?
        .lctrl_mode = PCNT_MODE_KEEP, // Reverse counting direction if low
        .hctrl_mode = PCNT_MODE_KEEP,    // Keep the primary counter mode if high
        // Set the maximum and minimum limit values to watch
        .counter_h_lim = 30000,
        .counter_l_lim = 0,
    };
    /* Initialize PCNT unit */
    pcnt_unit_config(&pcnt_config);

    pcnt_config.pulse_gpio_num=input_gpio;
    pcnt_config.unit=PCNT_UNIT_0;
    // pcnt_unit_config(&pcnt_config);         //初始化pcnt uint1

    // /* Configure and enable the input filter */
    // pcnt_set_filter_value(PCNT_UNIT_0, 1000);
    // pcnt_filter_enable(PCNT_UNIT_0);

    /* Initialize PCNT's counter */
    pcnt_counter_pause(PCNT_UNIT_0);
    pcnt_counter_clear(PCNT_UNIT_0);
    /* Everything is set up, now go to counting */
    
}

void mkc_hallStartCounter() {
    pcnt_counter_pause(hallPcntUint);
    pcnt_counter_clear(hallPcntUint);
    pcnt_counter_resume(hallPcntUint);  
}

void mkc_hallStopCounter() {
    pcnt_counter_pause(hallPcntUint);
    pcnt_counter_clear(hallPcntUint);          
}

void mkc_hallClearCounter() {
    pcnt_counter_clear(hallPcntUint);     
}

int16_t mkc_hallGetCounter() {
    int16_t count;
    pcnt_get_counter_value(hallPcntUint,&count);
    return  count;       
}