set(COMPONENT_SRCS "mkc_protocol_adaptor.c" "main.c" "ble_module.c" "./main.c ./timer_tools_module.c ./temp_ir_sensor.c ./pwm_module.c ./MLX90614_API.c ./MLX90614_SMBus_Driver.c ./hall_counter_module.c ./ble_module.c ./persist_module.c ./mkc_protocol_adaptor.c")
set(COMPONENT_ADD_INCLUDEDIRS "./include")

register_component()