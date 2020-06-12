/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "include/mkc_ble_module.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "include/mkc_timer_tools_module.h"
#include "include/mkc_hall_counter_module.h"
#include "include/mkc_pwm_module.h"
#include "include/mkc_temp_ir_sensor.h"
#include "include/mkc_persist_module.h"
#include "include/mkc_protocol_adaptor.h"
#include "esp32/rom/crc.h"

#define MAIN_TAG "MAIN"

#define MKC_HALL_COUNTER_MODULE
#define MKC_PWM_MODULE
#define MLX90614_IRTEMP_MODULE
#define MKC_BLE_MODULE
#define MKC_PERSIST_MODULE

#define LED_R_IO 5
#define HALL_GPIO 13
#define PWM_GPIO 14
#define MLX90614_SDA_GPIO 25
#define MLX90614_SCL_GPIO 18
#define MLX90614_GND_GPIO 23
#define MLX90614_VCC_GPIO 19
#define BLE_RESET_GPIO 21

#define MKC_FAN_DUTY_DEFAULT 0

// IR_Temperature
uint8_t switch_is_on;
// uint8_t a_temp_int;
// uint8_t a_temp_dec;
// uint8_t o_temp_int;
// uint8_t o_temp_dec;

// PWM
// int fan_rpm;
// uint8_t fan_duty;

// data model
mkc_protocol_model_t data_model;

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

    #ifdef MKC_HALL_COUNTER_MODULE
        data_model.fan_rpm = mkc_hallGetCounter() * 12;
        data_model.fan_duty = mkc_fan_get_duty();
        printf("rpm:%d duty:%d\r\n", data_model.fan_rpm, data_model.fan_duty);
        mkc_hallClearCounter();
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
    #ifdef MKC_BLE_MODULE
        if (switch_is_on != 0){
            // float a_temp = 0;
            // float o_temp = 0;
            // a_temp = mkc_getAmbientTemp();
            // o_temp = mkc_getObjectTemp();
            // a_temp_int = (uint16_t)a_temp;
            // a_temp_dec = (uint16_t)((a_temp - (float)a_temp_int) * 1000);
            // o_temp_int = (uint16_t)o_temp;
            // o_temp_dec = (uint16_t)((o_temp - (float)o_temp_int) * 1000);
            data_model.temp_ir_o_data = mkc_get_object_temp_struct();
            data_model.temp_ir_a_data = mkc_get_ambient_temp_struct();
            if (resetCounter == 0)
            {   
                if (data_model.temp_ir_o_data.temp_int > 45)
                {
                    resetCounter = 5;
                    mkc_switch_to_reset_mode(true);
                    ESP_LOGI(MAIN_TAG, "Temperature satisfied");
                    mkc_ble_module_reset();
                }
                else
                {
                    mkc_switch_to_reset_mode(false);
                }
            }
            if (resetCounter > 0)
            {
                resetCounter--;
                ESP_LOGI(MAIN_TAG, "Counter is running");
            }
            ESP_LOGI(MAIN_TAG, "IR Temperature int data :%d.%d %d.%d \n", data_model.temp_ir_a_data.temp_int, data_model.temp_ir_a_data.temp_dec, data_model.temp_ir_o_data.temp_int, data_model.temp_ir_o_data.temp_dec);
        }
    #endif
    #endif

    #ifdef MKC_BLE_MODULE
        // mkc_set_attributes(MKC_IDX_IR_TEMPA_INT_VAL, a_temp_int);
        // mkc_set_attributes(MKC_IDX_IR_TEMPA_DEC_VAL, a_temp_dec);
        // mkc_set_attributes(MKC_IDX_IR_TEMPO_INT_VAL, o_temp_int);
        // mkc_set_attributes(MKC_IDX_IR_TEMPO_DEC_VAL, o_temp_dec);
        // mkc_set_attributes(MKC_IDX_FAN_SPEED_RPM_VAL, (uint16_t)fan_rpm);
        if (resetCounter == 4)
        {
            mkc_fan_set_duty(100);
        }
        if (resetCounter == 3)
        {
            mkc_fan_set_duty(0);
        }
        if (resetCounter == 2)
        {
            mkc_fan_set_duty(100);
        }
        if (resetCounter == 1)
        {
            mkc_fan_set_duty(0);
        }
        // uint16_t freq = mkc_get_attrubutes(MKC_IDX_DELAY_VAL);
        // if ((int)freq != 0){
        //     count = freq * 30;
        //     mkc_set_attributes(MKC_IDX_DELAY_VAL, (uint16_t)0);
        // }
        if (count > 1){
            count--;
        }else if(count == 1){
            // mkc_set_attributes(MKC_IDX_FAN_SPEED_PERCENTAGE_VAL, (uint16_t)0);
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

esp_err_t ble_receive_datas(enum mkc_idx_attributes attr_idx, uint16_t len, uint8_t *value){
    mkc_protocol_model_t model = mkc_protocol_data_to_model(value);
    mkc_persist_set_data(model);
    switch (attr_idx)
    {
    case MKC_IDX_WRITEIN_VAL:{
        // decode 
        
        switch (model.typ)
        {
        case 03:{
            float fan_duty = model.fan_duty / 100.0;
            mkc_fan_set_duty(fan_duty);
            break;
        }
        case 04:{
            uint16_t delay = model.delay;
            break;
        }
        default:
            break;
        }
        // print debug
        // for (size_t i = 0; i < 16; i++)
        // {
        //     printf("%02x", value[i]);
        //     if (i == 15)
        //     {
        //         printf("\n");
        //     }else{
        //         printf(":");
        //     }
        // }
        break;
    }
    default:
        break;
    }
    return ESP_FAIL;
}

esp_err_t ble_compose_response(enum mkc_ble_response_typ rsp_typ, uint8_t *value){
    switch (rsp_typ)
    {
    case MKC_BLE_RESPONSE_TYP_INFO:{
        mkc_protocol_model_t model;
        model.hdr = 0x10;
        model.typ = 0xfe;
        uint8_t token[4] = {0x11, 0x22, 0x33, 0x44};
        model.token = &token;
        model.crc = crc8_be(0, model.token, 4);
        // to data
        mkc_protocol_model_to_data(value, model);
        return ESP_OK;
        break;
    }
    case MKC_BLE_RESPONSE_TYP_STATUS:{
        mkc_protocol_model_t model;
        model.hdr = 0x10;
        model.typ = 0xff;
        model.len = 1;
        uint8_t data[1] = {0x00};
        model.data = &data;
        uint8_t token[4] = {0x11, 0x22, 0x33, 0x44};
        model.token = &token;
        model.crc = crc8_be(0, model.token, 4);
        // to data
        mkc_protocol_model_to_data(value, model);
        return ESP_OK;
        break;
    }
    default:
        return ESP_FAIL;
        break;
    }
}

void app_main()
{   
    uint8_t ble_auth = 0;
    // init
    memset(&data_model, 0, sizeof(data_model));

    #ifdef MKC_PERSIST_MODULE
        mkc_persist_module_init();
        mkc_protocol_model_t model = mkc_persist_get_data();
        data_model.fan_duty = model.fan_duty;
    #endif

    // uint8_t fakeData[16] = {0x10, 0x02, 0x08, 0x1e, 0x02, 0x19, 0x05, 0x56, 0x08, 0x00, 0x64, 0x1f, 0x96, 0x0c, 0x24, 0xd2};

    // // mkc_protocol_model_t model;
    // model.hdr = 0x10;
    // model.typ = 0x02;
    // model.len = 8;
    // // uint8_t data[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    // // model.data = &data;
    // mkc_temp_ir_data_t temp_o;
    // temp_o.temp_int = 30;
    // temp_o.temp_dec = 2;
    // mkc_temp_ir_data_t temp_a;
    // temp_a.temp_int = 25;
    // temp_a.temp_dec = 5;
    // model.temp_ir_a_data = temp_a;
    // model.temp_ir_o_data = temp_o;
    // model.fan_duty = 86;
    // model.fan_rpm = 2048;
    // model.delay = 0x64;
    // uint8_t token[4] = {0xaa, 0xbb, 0xcc, 0xdd};
    // model.token = &token;
    // model.crc = 0x99;
    // uint8_t *datas;
    // mkc_protocol_model_to_data(&datas, model);

    // model = mkc_protocol_data_to_model(&datas);
    // printf("typ: %02u\n", model.typ);

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
    
    #ifdef MKC_HALL_COUNTER_MODULE
        mkc_hallInit(HALL_GPIO);
        mkc_hallStartCounter();
    #endif

    #ifdef MKC_PWM_MODULE
        mkc_pwm_init(PWM_GPIO);
        mkc_fan_set_start();
        if (model.fan_duty == 0)
        {
            model.fan_duty = MKC_FAN_DUTY_DEFAULT;
        }
        mkc_fan_set_duty(model.fan_duty);
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
        mkc_ir_temp_Init(MLX90614_SDA_GPIO, MLX90614_SCL_GPIO);
    #endif

    
    #ifdef MKC_BLE_MODULE
        mkc_ble_module_init(&ble_receive_datas, &ble_compose_response);
        //选择IO
        gpio_pad_select_gpio(BLE_RESET_GPIO);
        gpio_set_direction(BLE_RESET_GPIO, GPIO_MODE_INPUT);
        // mkc_set_attributes(MKC_IDX_FAN_SPEED_PERCENTAGE_VAL, fan_duty);
        // mkc_set_attributes(MKC_IDX_SWITCHA_VAL, switch_is_on);
        // mkc_set_attributes(MKC_IDX_AUTH_VAL, ble_auth);
        mkc_ble_module_reset();
    #endif

    // timer
    mkc_timer_tools_Init(&action, 4000);
    mkc_timer_tools_start();

    
    
    // printf("Restarting now.\n");
    // fflush(stdout);
    // esp_restart();
}
