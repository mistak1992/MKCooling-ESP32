/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "include/ble_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "include/timer_tools_module.h"
#include "include/hall_counter_module.h"
#include "include/pwm_module.h"
#include "include/temp_ir_sensor.h"
#include "include/persist_module.h"
#include "include/mkc_protocol_adaptor.h"

#define MAIN_TAG "MAIN"

// #define HALL_COUNTER_MODULE
// #define PWM_MODULE
// #define MLX90614_IRTEMP_MODULE
// #define BLE_MODULE
// #define PERSIST_MODULE

#define LED_R_IO 5
#define HALL_GPIO 13
#define PWM_GPIO 14
#define MLX90614_SDA_GPIO 25
#define MLX90614_SCL_GPIO 18
#define MLX90614_GND_GPIO 23
#define MLX90614_VCC_GPIO 19
#define BLE_RESET_GPIO 21

#define FAN_DUTY_DEFAULT 0

// IR_Temperature
uint8_t switch_is_on;
uint8_t a_temp_int;
uint8_t a_temp_dec;
uint8_t o_temp_int;
uint8_t o_temp_dec;

// PWM
int fan_rpm;
uint8_t fan_duty;

// BLE
enum mkc_ble_state ble_state;

int count = 0;

int resetCounter = 0;

bool light_on = false;

void action() {
    // if (light_on == true){
    //     //红灯灭
    //     gpio_set_level(LED_R_IO, 1);
    // }else{
    //     //红灯亮
    //     gpio_set_level(LED_R_IO, 0);
    // }
    // light_on = !light_on;

    #ifdef HALL_COUNTER_MODULE
        fan_rpm = hallGetCounter() * 12;
        fan_duty = fan_get_duty();
        printf("rpm:%d duty:%d\r\n", fan_rpm, fan_duty);
        hallClearCounter();
    #endif 

    #ifdef MLX90614_IRTEMP_MODULE
        if (ble_state == MKC_BLE_STATE_CONNECTED){
            // if (gpio_get_level(MLX90614_VCC_GPIO) == 1){
            //     gpio_set_level(MLX90614_VCC_GPIO, 0);
            // }
        }else if (ble_state == MKC_BLE_STATE_DISCONNECTED){
            // if (gpio_get_level(MLX90614_VCC_GPIO) == 0){
            //     gpio_set_level(MLX90614_VCC_GPIO, 1);
            // }
        }
        if (switch_is_on != 0){
            float a_temp = 0;
            float o_temp = 0;
            a_temp = getAmbientTemp();
            o_temp = getObjectTemp();
            a_temp_int = (uint16_t)a_temp;
            a_temp_dec = (uint16_t)((a_temp - (float)a_temp_int) * 1000);
            o_temp_int = (uint16_t)o_temp;
            o_temp_dec = (uint16_t)((o_temp - (float)o_temp_int) * 1000);
            if (resetCounter == 0)
            {   
                if (o_temp_int > 45)
                {
                    resetCounter = 5;
                    switch_to_reset_mode(true);
                    ESP_LOGI(MAIN_TAG, "Temperature satisfied");
                    ble_module_reset();
                }
                else
                {
                    switch_to_reset_mode(false);
                }
            }
            if (resetCounter > 0)
            {
                resetCounter--;
                ESP_LOGI(MAIN_TAG, "Counter is running");
            }
            ESP_LOGI(MAIN_TAG, "IR Temperature int data :%d.%d %d.%d \n", a_temp_int, a_temp_dec, o_temp_int, o_temp_dec);
        }
    #endif

    #ifdef BLE_MODULE
        set_attributes(MKC_IDX_IR_TEMPA_INT_VAL, a_temp_int);
        set_attributes(MKC_IDX_IR_TEMPA_DEC_VAL, a_temp_dec);
        set_attributes(MKC_IDX_IR_TEMPO_INT_VAL, o_temp_int);
        set_attributes(MKC_IDX_IR_TEMPO_DEC_VAL, o_temp_dec);
        set_attributes(MKC_IDX_FAN_SPEED_RPM_VAL, (uint16_t)fan_rpm);
        if (resetCounter == 4)
        {
            fan_set_duty(100);
        }
        if (resetCounter == 3)
        {
            fan_set_duty(0);
        }
        if (resetCounter == 2)
        {
            fan_set_duty(100);
        }
        if (resetCounter == 1)
        {
            fan_set_duty(0);
        }
        uint16_t freq = get_attrubutes(MKC_IDX_DELAY_VAL);
        if ((int)freq != 0){
            count = freq * 30;
            set_attributes(MKC_IDX_DELAY_VAL, (uint16_t)0);
        }
        if (count > 1){
            count--;
        }else if(count == 1){
            set_attributes(MKC_IDX_FAN_SPEED_PERCENTAGE_VAL, (uint16_t)0);
            count = 0;
        }
        // int is_press = gpio_get_level(BLE_RESET_GPIO);
        // if (count > 2){
        //     count = 0;
        //     ble_module_reset();
        //     ESP_LOGI(MAIN_TAG, "remove bonds action");
        // }else if(is_press == 0){
        //     count++;
        //     ESP_LOGI(MAIN_TAG, "button pressed");
        // }else{   
        //     count = 0;
        // }
    #endif
}

void ble_data_update(enum mkc_idx_attributes attr_idx, uint16_t value){
    switch (attr_idx)
    {
    case MKC_IDX_TEMP_INT_VAL:{
        ESP_LOGI(MAIN_TAG, "PC Temperature int data : %d\n", value);
        break;
    }
    case MKC_IDX_TEMP_DEC_VAL:{
        ESP_LOGI(MAIN_TAG, "PC Temperature decimal data : %d\n", value);
        break;
    }
    case MKC_IDX_FAN_SPEED_PERCENTAGE_VAL:{
        ESP_LOGI(MAIN_TAG, "PC Temperature fan percentage data : %d\n", value);
        if (value == 0){
            value = 1;
        }
        fan_duty = value;
        fan_set_duty((float)value);
        #ifdef PERSIST_MODULE
            persist_set_data(PERSIST_MODULE_DUTY, fan_duty);
        #endif
        break;
    }
    case MKC_IDX_SWITCHA_VAL:{
        if (value == 0){    
            gpio_set_level(MLX90614_VCC_GPIO, 0);
            switch_is_on = 0;
        }else{
            gpio_set_level(MLX90614_VCC_GPIO, 1);
            switch_is_on = 1;
        }
        #ifdef PERSIST_MODULE
            persist_set_data(PERSIST_MODULE_MLX90614_SWITCH, switch_is_on);
        #endif
        break;
    }
    case MKC_IDX_AUTH_VAL:{
        #ifdef PERSIST_MODULE
            persist_set_data(PERSIST_MODULE_AUTH, value);
        #endif
        break;
    }
    default:
        break;
    }
}

void app_main()
{   
    uint8_t ble_auth = 0;

    #ifdef PERSIST_MODULE
        persist_module_init();
        char *value_str = NULL;
        value_str = persist_get_data(PERSIST_MODULE_DUTY, value_str);
        fan_duty = String2Int(value_str);
        value_str = NULL;
        value_str = persist_get_data(PERSIST_MODULE_MLX90614_SWITCH, value_str);
        switch_is_on = String2Int(value_str);
        value_str = NULL;
        value_str = persist_get_data(PERSIST_MODULE_AUTH, value_str);
        ble_auth = String2Int(value_str);
    #endif
    
    printf("Hello world!\n");

    mkc_protocol_model_t model;
    model.hdr = 0x10;
    model.typ = 0x02;
    model.len = 12;
    uint8_t data[9] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99};
    model.data = &data;
    uint8_t token[4] = {0xaa, 0xbb, 0xcc, 0xdd};
    model.token = &token;
    model.crc = 0x99;
    uint8_t *datas;
    mkc_protocol_model_to_data(&datas, &model);

    /* Print chip information */
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    printf("This is ESP32 chip with %d CPU cores, WiFi%s%s, ",
            chip_info.cores,
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
            (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");

    printf("silicon revision %d, ", chip_info.revision);

    printf("%dMB %s flash\n", spi_flash_get_chip_size() / (1024 * 1024),
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    // for (int i = 10; i >= 0; i--) {
    //     printf("Restarting in %d seconds...\n", i);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // //选择IO
    // gpio_pad_select_gpio(LED_R_IO);
    // //设置IO为输出
    // gpio_set_direction(LED_R_IO, GPIO_MODE_OUTPUT);
    // hall
    
    #ifdef HALL_COUNTER_MODULE
        hallInit(HALL_GPIO);
        hallStartCounter();
    #endif

    #ifdef PWM_MODULE
        pwm_init(PWM_GPIO);
        fan_set_start();
        if (fan_duty == 0){
            fan_duty = FAN_DUTY_DEFAULT;
        }
        fan_set_duty(fan_duty);
    #endif
    
    #ifdef MLX90614_IRTEMP_MODULE
        gpio_pad_select_gpio(LED_R_IO);
        gpio_set_direction(LED_R_IO, GPIO_MODE_OUTPUT);
        gpio_set_level(LED_R_IO, 0);
        gpio_pad_select_gpio(MLX90614_VCC_GPIO);
        gpio_set_direction(MLX90614_VCC_GPIO, GPIO_MODE_OUTPUT);
        gpio_set_level(MLX90614_VCC_GPIO, (int)switch_is_on);
        gpio_pad_select_gpio(MLX90614_GND_GPIO);
        gpio_set_direction(MLX90614_GND_GPIO, GPIO_MODE_OUTPUT);
        gpio_set_level(MLX90614_GND_GPIO, 0);
        ir_temp_Init(MLX90614_SDA_GPIO, MLX90614_SCL_GPIO);
    #endif

    
    #ifdef BLE_MODULE
        ble_module_init(&ble_data_update);
        //选择IO
        gpio_pad_select_gpio(BLE_RESET_GPIO);
        gpio_set_direction(BLE_RESET_GPIO, GPIO_MODE_INPUT);
        set_attributes(MKC_IDX_FAN_SPEED_PERCENTAGE_VAL, fan_duty);
        set_attributes(MKC_IDX_SWITCHA_VAL, switch_is_on);
        set_attributes(MKC_IDX_AUTH_VAL, ble_auth);
        // ble_module_reset();
    #endif

    // timer
    timer_tools_Init(&action, 4000);
    timer_tools_start();

    
    
    // printf("Restarting now.\n");
    // fflush(stdout);
    // esp_restart();
}
