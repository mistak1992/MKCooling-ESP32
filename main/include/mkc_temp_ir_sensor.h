#ifndef _MKC_TEMP_IR_SENSOR_H_
#define _MKC_TEMP_IR_SENSOR_H_
#include "driver/gpio.h"
#include "mkc_data_structure.h"

bool mkc_ir_temp_Init(gpio_num_t sda_gpio, gpio_num_t scl_gpio);
float mkc_get_ambient_temp();
float mkc_get_object_temp();
mkc_temp_ir_data_t mkc_get_ambient_temp_struct();
mkc_temp_ir_data_t mkc_get_object_temp_struct();

#endif
