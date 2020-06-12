#include "include/MLX90614_API.h"
#include "include/MLX90614_SMBus_Driver.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "include/mkc_data_structure.h"

#define MLX90614_DEFAULT_ADDRESS 0x5A

bool mkc_ir_temp_Init(gpio_num_t sda_gpio, gpio_num_t scl_gpio){
    MLX90614_SMBusInit(sda_gpio, scl_gpio, 50000);
    return true;
}

float mkc_get_ambient_temp() {
    float ta = 0;
    MLX90614_GetTa(MLX90614_DEFAULT_ADDRESS, &ta);
    return ta;
}

float mkc_get_object_temp() {
    float to = 0;
    MLX90614_GetTo(MLX90614_DEFAULT_ADDRESS, &to);
    return to;
}

mkc_temp_ir_data_t mkc_get_ambient_temp_struct(){
    float a_temp = mkc_get_ambient_temp();
    mkc_temp_ir_data_t data;
    data.temp_int = (uint8_t)a_temp;
    data.temp_dec = (uint8_t)((a_temp - data.temp_int) * 100);
    return data;
}

mkc_temp_ir_data_t mkc_get_object_temp_struct(){
    float o_temp = mkc_get_object_temp();
    mkc_temp_ir_data_t data;
    data.temp_int = (uint8_t)o_temp;
    data.temp_dec = (uint8_t)((o_temp - data.temp_int) * 100);
    return data;
}