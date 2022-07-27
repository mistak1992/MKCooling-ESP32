<<<<<<< HEAD
set(COMPONENT_SRCS "mkc_persist_module.c" "mkc_pwm_module.c" "mkc_protocol_adaptor.c" "main.c" "mkc_ble_module.c" "./main.c ./mkc_timer_tools_module.c ./mkc_temp_ir_sensor.c ./mkc_pwm_module.c ./MLX90614_API.c ./MLX90614_SMBus_Driver.c ./mkc_hall_counter_module.c ./mkc_persist_module.c ./mkc_protocol_adaptor.c ./button.c ./led_strip_rmt_ws2812.c")
=======
set(COMPONENT_SRCS "mkc_timer_tools_module.c" "mkc_pwm_module.c" "mkc_protocol_adaptor.c" "main.c" "mkc_ble_module.c" "./main.c ./mkc_timer_tools_module.c ./mkc_temp_ir_sensor.c ./mkc_pwm_module.c ./MLX90614_API.c ./MLX90614_SMBus_Driver.c ./mkc_hall_counter_module.c ./mkc_persist_module.c ./mkc_protocol_adaptor.c")
>>>>>>> 9ce740068d74b4a9a5b1d13486b0cdf4454dbc75
set(COMPONENT_ADD_INCLUDEDIRS "./include")

register_component()