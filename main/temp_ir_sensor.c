#include "include/MLX90614_API.h"
#include "include/MLX90614_SMBus_Driver.h"
#include "esp_system.h"
#include "driver/gpio.h"

#define MLX90614_DEFAULT_ADDRESS 0x5A

bool ir_temp_Init(gpio_num_t sda_gpio, gpio_num_t scl_gpio){
    MLX90614_SMBusInit(sda_gpio, scl_gpio, 50000);
    return true;
}

float getAmbientTemp() {
    float ta = 0;
    MLX90614_GetTa(MLX90614_DEFAULT_ADDRESS, &ta);
    return ta;
}

float getObjectTemp() {
    float to = 0;
    MLX90614_GetTo(MLX90614_DEFAULT_ADDRESS, &to);
    return to;
}