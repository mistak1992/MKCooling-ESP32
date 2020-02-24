#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
#define PWM_DEFAULT_DUTY_50 50.0
uint8_t static pwm_gpio;
mcpwm_unit_t static mcpwm_num = MCPWM_UNIT_0;
mcpwm_timer_t static timer_num = MCPWM_TIMER_0;
float static current_duty;

void pwm_init(uint8_t output_gpio)
{
    printf("[timer_tools]:failed to start\r\n");
    pwm_gpio = output_gpio;
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, pwm_gpio);
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 25000;
    pwm_config.cmpr_a = 0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
}

/**
 * @brief motor moves in forward direction, with duty cycle = duty %
 */
void fan_set_duty(float duty_cycle)
{
    current_duty = duty_cycle;
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
    mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

float fan_get_duty()
{
    return current_duty;
}

/**
 * @brief motor stop
 */
void fan_set_stop()
{
    fan_set_duty(0.0);
    mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
}

/**
 * @brief Configure MCPWM module for brushed dc motor
 */
void fan_set_start() {
    fan_set_duty(PWM_DEFAULT_DUTY_50);
}