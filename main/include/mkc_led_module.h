#ifndef _MKC_LED_MODULE_H_
#define _MKC_LED_MODULE_H_
#include "esp_err.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "led_strip.h"

enum light_mode
{
  LIGHT_MODE_COMMON,
  LIGHT_MODE_RESET,
};

bool mkc_led_light_is_on();
enum light_mode mkc_led_current_mode();
esp_err_t mkc_led_module_init(uint8_t offset);
void mkc_led_set_common(uint8_t percentage);
void mkc_led_set_reset(uint8_t resetCount);
void mkc_led_set_off();

#endif