#include "include/mkc_led_module.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0

led_strip_t *strip;

uint8_t led_offset = 0;
bool light_on = false;
enum light_mode led_mode = LIGHT_MODE_COMMON;
uint32_t red = 0;
uint32_t green = 0;
uint32_t blue = 0;
uint16_t hue = 0;
uint16_t start_rgb = 0;
uint8_t led_percentage = 0;

/**
 * @brief Simple helper function, converting HSV color space to RGB color space
 *
 * Wiki: https://en.wikipedia.org/wiki/HSL_and_HSV
 *
 */
void led_strip_hsv2rgb(uint32_t h, uint32_t s, uint32_t v, uint32_t *r, uint32_t *g, uint32_t *b)
{
  h %= 360; // h -> [0,360]
  uint32_t rgb_max = v * 2.55f;
  uint32_t rgb_min = rgb_max * (100 - s) / 100.0f;

  uint32_t i = h / 60;
  uint32_t diff = h % 60;

  // RGB adjustment amount by hue
  uint32_t rgb_adj = (rgb_max - rgb_min) * diff / 60;

  switch (i)
  {
  case 0:
    *r = rgb_max;
    *g = rgb_min + rgb_adj;
    *b = rgb_min;
    break;
  case 1:
    *r = rgb_max - rgb_adj;
    *g = rgb_max;
    *b = rgb_min;
    break;
  case 2:
    *r = rgb_min;
    *g = rgb_max;
    *b = rgb_min + rgb_adj;
    break;
  case 3:
    *r = rgb_min;
    *g = rgb_max - rgb_adj;
    *b = rgb_max;
    break;
  case 4:
    *r = rgb_min + rgb_adj;
    *g = rgb_min;
    *b = rgb_max;
    break;
  default:
    *r = rgb_max;
    *g = rgb_min;
    *b = rgb_max - rgb_adj;
    break;
  }
}

void led_strip_duty(uint8_t duty, uint32_t *r, uint32_t *g, uint32_t *b)
{
  uint8_t rgb_max = 255;
  float duty_max = 100.f;
  *r = rgb_max * (float)(duty > 33 && duty < 66 ? (duty - 33) : (duty <= 33 ? 0 : (66 - 33))) / (66.f - 33.f);
  *g = rgb_max * (1 - (float)(duty > 66 ? (duty - 66) : 0) / (duty_max - 66.f));
  *b = rgb_max * (float)(duty < 33 ? (33 - duty) : 0) / 33.f;
}

void led_strip_reset(int rc, uint32_t *r, uint32_t *g, uint32_t *b)
{
  if (rc % 2 == 0 && rc > 2)
  {
    *r = 255.f;
  }
  else
  {
    *r = 0.f;
  }
  *g = 0.f;
  if (rc == 2)
  {
    *b = 255.f;
  }
  else
  {
    *b = 0.f;
  }
}

bool mkc_led_light_is_on()
{
  return light_on;
}

enum light_mode mkc_led_current_mode()
{
  return led_mode;
}

esp_err_t mkc_led_module_init(uint8_t offset)
{
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX(27, RMT_TX_CHANNEL);
  // set counter clock to 40MHz
  config.clk_div = 2;
  // install ws2812 driver
  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
  led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(1, (led_strip_dev_t)config.channel);
  strip = led_strip_new_rmt_ws2812(&strip_config);
  led_offset = offset;
  return ESP_OK;
}

void mkc_led_set_common(uint8_t fan_duty)
{
  if (light_on == false)
  {
    light_on = true;
  }
  led_mode = LIGHT_MODE_COMMON;
  // 渐变LED
  if (fan_duty > led_percentage)
  {
    if (fan_duty - led_percentage > led_offset)
    {
      led_percentage += led_offset;
    }
    else
    {
      led_percentage = fan_duty;
    }
  }
  else
  {
    if (led_percentage - fan_duty > led_offset)
    {
      led_percentage -= led_offset;
    }
    else
    {
      led_percentage = fan_duty;
    }
  }
  led_strip_duty(led_percentage, &red, &green, &blue);
  // Write RGB values to strip driver
  ESP_ERROR_CHECK(strip->set_pixel(strip, 0, red, green, blue));
  // Flush RGB values to LEDs
  ESP_ERROR_CHECK(strip->refresh(strip, 100));
}
void mkc_led_set_reset(uint8_t resetCount)
{
  if (light_on == false)
  {
    light_on = true;
  }
  led_mode = LIGHT_MODE_RESET;
  led_strip_reset(resetCount, &red, &green, &blue);
  ESP_ERROR_CHECK(strip->set_pixel(strip, 0, red, green, blue));
  ESP_ERROR_CHECK(strip->refresh(strip, 100));
}
void mkc_led_set_off()
{
  light_on = false;
  ESP_ERROR_CHECK(strip->clear(strip, 100));
}