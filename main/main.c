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
#include "sdkconfig.h"
#include "include/mkc_timer_tools_module.h"
#include "include/mkc_hall_counter_module.h"
#include "include/mkc_pwm_module.h"
#include "include/mkc_temp_ir_sensor.h"
#include "include/mkc_persist_module.h"
#include "include/mkc_protocol_adaptor.h"
#include "esp32/rom/crc.h"
#include "include/button.h"
#include "include/mkc_led_module.h"

#define MAIN_TAG "MAIN"

#define MKC_HALL_COUNTER_MODULE
#define MKC_PWM_MODULE
// #define MLX90614_IRTEMP_MODULE
#define MKC_BLE_MODULE
#define MKC_PERSIST_MODULE
#define MKC_CONTROL_MODULE
#define MKC_RGB_MODULE
// 设计中 TODO: 这个模式目前存在问题，手动模式下无法分辨是否有新的指令
// #define MKC_SLEEP_MODULE

#define LED_R_IO 27
#define HALL_GPIO 32 // 13
#define PWM_GPIO 26  // 14
#define MLX90614_SDA_GPIO 25
#define MLX90614_SCL_GPIO 18
#define MLX90614_GND_GPIO 23
#define MLX90614_VCC_GPIO 19
#define BLE_RESET_GPIO 39

#define MKC_FAN_DUTY_DEFAULT 0
// 基础计时器运行间隔100ms
#define MKC_TIMER_INTERVAL 100
// 基础计数器预设次数
#define MKC_BASE_FACTOR 40
// LED计数器预设次数
#define MKC_LED_FACTOR 1
// 休眠计数器预设次数
#define MKC_SLEEP_FACTOR 300
// LED渐变数
#define MKC_LED_OFFSET 4
// IR_Temperature
uint8_t switch_is_on;

// data model
mkc_protocol_model_t data_model;

// BLE
enum mkc_ble_state ble_state;

// button
button_event_t ev;
QueueHandle_t button_events;
bool isLongpressContinue = false;

// timerCounter
int baseCounter = 0;

int count = 0;
// 重制计数器
int resetCounter = 0;
// LED计数器
int ledCounter = 0;
// 休眠计数器
int sleepCounter = 0;

// 休眠
bool sleeping = false;

void action()
{
  // LED 事件
  ledCounter++;
  if (ledCounter >= MKC_LED_FACTOR)
  {
    ledCounter = 0;
#ifdef MKC_RGB_MODULE
    if (mkc_led_light_is_on() == true && sleeping == false)
    {
      if (mkc_led_current_mode() == LIGHT_MODE_COMMON)
      {
        uint8_t fan_duty = (uint8_t)(mkc_fan_get_duty());
        mkc_led_set_common(fan_duty);
      }
      else
      {
        mkc_led_set_reset(resetCounter);
      }
      if (resetCounter == 1)
      {
        mkc_led_set_off();
        resetCounter = 0;
      }
      else if (resetCounter > 0)
      {
        resetCounter -= 1;
      }
    }
    else
    {
      mkc_led_set_off();
    }
#endif
  }
  // 通用事件
  // MKC_BASE_FACTOR 次才执行
  baseCounter++;
  if (baseCounter >= MKC_BASE_FACTOR)
  {
    // if (true) {
    baseCounter = 0;
    ESP_LOGI(MAIN_TAG, "Current Data || temp_a:%d.%d temp_o:%d.%d fan_duty:%d fan_rpm:%d", data_model.temp_ir_a_data.temp_int, data_model.temp_ir_a_data.temp_dec, data_model.temp_ir_o_data.temp_int, data_model.temp_ir_o_data.temp_dec, data_model.fan_duty, data_model.fan_rpm);

#ifdef MKC_HALL_COUNTER_MODULE
    data_model.fan_rpm = (uint16_t)(mkc_hallGetCounter() * 12);
    data_model.fan_duty = (uint8_t)(mkc_fan_get_duty());
    // printf("rpm:%d duty:%d\r\n", data_model.fan_rpm, data_model.fan_duty);
    mkc_hallClearCounter();
#endif
#ifdef MLX90614_IRTEMP_MODULE
    if (ble_state == MKC_BLE_STATE_CONNECTED)
    {
      // if (gpio_get_level(MLX90614_VCC_GPIO) == 1){
      //     gpio_set_level(MLX90614_VCC_GPIO, 0);
      // }
    }
    else if (ble_state == MKC_BLE_STATE_DISCONNECTED)
    {
      // if (gpio_get_level(MLX90614_VCC_GPIO) == 0){
      //     gpio_set_level(MLX90614_VCC_GPIO, 1);
      // }
    }
#ifdef MKC_BLE_MODULE
    if (switch_is_on != 0)
    {
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
        if (resetCounter > 0)
        {
          resetCounter--;
          ESP_LOGI(MAIN_TAG, "Counter is running");
        }
        ESP_LOGI(MAIN_TAG, "IR Temperature int data :%d.%d %d.%d \n", data_model.temp_ir_a_data.temp_int, data_model.temp_ir_a_data.temp_dec, data_model.temp_ir_o_data.temp_int, data_model.temp_ir_o_data.temp_dec);
      }
#endif
#endif

      // #ifdef MKC_BLE_MODULE
      //       if (resetCounter == 4)
      //       {
      //         mkc_fan_set_duty(100);
      //       }
      //       ESP_LOGI(MAIN_TAG, "IR Temperature int data :%d.%d %d.%d \n", data_model.temp_ir_a_data.temp_int, data_model.temp_ir_a_data.temp_dec, data_model.temp_ir_o_data.temp_int, data_model.temp_ir_o_data.temp_dec);
      // #endif

#ifdef MKC_BLE_MODULE
      // mkc_set_attributes(MKC_IDX_IR_TEMPA_INT_VAL, a_temp_int);
      // mkc_set_attributes(MKC_IDX_IR_TEMPA_DEC_VAL, a_temp_dec);
      // mkc_set_attributes(MKC_IDX_IR_TEMPO_INT_VAL, o_temp_int);
      // mkc_set_attributes(MKC_IDX_IR_TEMPO_DEC_VAL, o_temp_dec);
      // mkc_set_attributes(MKC_IDX_FAN_SPEED_RPM_VAL, (uint16_t)fan_rpm);
      // if (resetCounter == 4)
      // {
      //   mkc_fan_set_duty(100);
      // }
      // if (resetCounter == 3)
      // {
      //   mkc_fan_set_duty(0);
      // }
      // if (resetCounter == 2)
      // {
      //   mkc_fan_set_duty(100);
      // }
      // if (resetCounter == 1)
      // {
      //   mkc_fan_set_duty(0);
      // }
      // uint16_t freq = mkc_get_attrubutes(MKC_IDX_DELAY_VAL);
      // if ((int)freq != 0){
      //     count = freq * 30;
      //     mkc_set_attributes(MKC_IDX_DELAY_VAL, (uint16_t)0);
      // }
      if (count > 1)
      {
        count--;
      }
      else if (count == 1)
      {
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

#ifdef MKC_CONTROL_MODULE
    if (xQueueReceive(button_events, &ev, 100 / portTICK_PERIOD_MS))
    {
      if (ev.pin == BLE_RESET_GPIO)
      {
        // ...
        if (ev.event == BUTTON_DOWN)
        {
        }
        if (ev.event == BUTTON_HELD)
        {
          if (isLongpressContinue == false)
          {
            isLongpressContinue = true;
          }
        }
        if (ev.event == BUTTON_UP)
        {
          if (isLongpressContinue == true)
          {
            mkc_ble_module_reset();
            // light_on = true;
            // led_mode = LIGHT_MODE_RESET;
            resetCounter = 50;
            mkc_led_set_reset(resetCounter);
            isLongpressContinue = false;
          }
          else
          {
            // led_mode = LIGHT_MODE_COMMON;
            // light_on = !light_on;
            mkc_led_set_common((uint8_t)(mkc_fan_get_duty()));
          }
        }
      }
    }
#endif

#ifdef MKC_SLEEP_MODULE
    sleepCounter++;
    if (sleepCounter >= MKC_SLEEP_FACTOR)
    {
      mkc_fan_set_duty(0);
      sleeping = true;
    }
    else
    {
      sleeping = false;
    }
#endif
  }

  void resetSleepCounter()
  {
#ifdef MKC_SLEEP_MODULE
    sleepCounter = 0;
#endif
  }

  esp_err_t ble_receive_datas(enum mkc_idx_attributes attr_idx, uint16_t len, uint8_t * value)
  {
    mkc_protocol_model_t model = mkc_protocol_data_to_model(value);
    mkc_persist_set_data(model);
    // data_model = model;
    switch (attr_idx)
    {
    case MKC_IDX_WRITEIN_VAL:
    {
      // decode

      switch (model.typ)
      {
      case 02:
      {
        // fetchInfo
        return ESP_OK;
        break;
      }
      case 03:
      {
        float fan_duty = model.fan_duty;
        ESP_LOGI(MAIN_TAG, "Set duty: %lf", fan_duty);
        mkc_fan_set_duty(fan_duty);
        resetSleepCounter();
        return ESP_OK;
        break;
      }
      case 04:
      {
        uint16_t delay = model.delay;
        return ESP_OK;
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

  esp_err_t ble_compose_response(enum mkc_ble_response_typ rsp_typ, uint8_t * value)
  {
    switch (rsp_typ)
    {
    case MKC_BLE_RESPONSE_TYP_INFO:
    {
      mkc_protocol_model_t model;
      model.hdr = 0x10;
      model.typ = 0xfe;
      model.temp_ir_a_data = data_model.temp_ir_a_data;
      model.temp_ir_o_data = data_model.temp_ir_o_data;
      model.fan_duty = data_model.fan_duty;
      model.fan_rpm = data_model.fan_rpm;
      model.delay = data_model.delay;
      uint8_t token[4] = {0x11, 0x22, 0x33, 0x44};
      model.token = &token;
      model.crc = crc8_be(0, model.token, 4);
      // to data
      mkc_protocol_model_to_data(value, model);
      return ESP_OK;
      break;
    }
    case MKC_BLE_RESPONSE_TYP_STATUS:
    {
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

// 持久化
#ifdef MKC_PERSIST_MODULE
    mkc_persist_module_init();
    mkc_protocol_model_t model = mkc_persist_get_data();
    // data_model = new mkc_protocol_model_t;
    data_model.fan_duty = model.fan_duty;
#endif

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

// 风扇转速
#ifdef MKC_HALL_COUNTER_MODULE
    mkc_hallInit(HALL_GPIO);
    mkc_hallStartCounter();
#endif

// 设置风扇转速
#ifdef MKC_PWM_MODULE
    mkc_pwm_init(PWM_GPIO);
    mkc_fan_set_start();
    if (model.fan_duty == 0)
    {
      model.fan_duty = MKC_FAN_DUTY_DEFAULT;
    }
    mkc_fan_set_duty(model.fan_duty);
#endif

// 红外温度模块
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

// BLE模块
#ifdef MKC_BLE_MODULE
    mkc_ble_module_init(&ble_receive_datas, &ble_compose_response);
    //选择IO
    gpio_pad_select_gpio(BLE_RESET_GPIO);
    gpio_set_direction(BLE_RESET_GPIO, GPIO_MODE_INPUT);
    // mkc_set_attributes(MKC_IDX_FAN_SPEED_PERCENTAGE_VAL, fan_duty);
    // mkc_set_attributes(MKC_IDX_SWITCHA_VAL, switch_is_on);
    // mkc_set_attributes(MKC_IDX_AUTH_VAL, ble_auth);
    // mkc_ble_module_reset();
#endif

#ifdef MKC_CONTROL_MODULE
    // button
    button_events = button_init(PIN_BIT(BLE_RESET_GPIO));
#endif

#ifdef MKC_RGB_MODULE
    mkc_led_module_init(MKC_LED_OFFSET);
#endif

    // timer
    mkc_timer_tools_Init(&action, MKC_TIMER_INTERVAL);
    mkc_timer_tools_start();
    // printf("Restarting now.\n");
    // fflush(stdout);
    // esp_restart();
  }
