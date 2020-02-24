#include "esp_system.h"
#include "esp_timer.h"

typedef void (*timer_tools_callback_t)(void* arg);
timer_tools_callback_t callback_t;
uint64_t time_interval;
//定时器句柄
esp_timer_handle_t timer_tools_handle = 0;

void timer_tools_action() {
	// int64_t tick = esp_timer_get_time();
	if (callback_t != NULL) {
        callback_t(timer_tools_handle);
    }
}

//定义一个周期重复运行的定时器结构体
esp_timer_create_args_t timer_tools_arg = { 
    .callback = &timer_tools_action, //设置回调函数
    .arg = NULL, //不携带参数
    .name = "timer_tools" //定时器名字
};

bool timer_tools_Init(timer_tools_callback_t callback, uint64_t interval){
    callback_t = callback;
    time_interval = interval;
    return true;
}

bool timer_tools_start() {
    //开始创建一个重复周期的定时器并且执行
	esp_err_t err = esp_timer_create(&timer_tools_arg, &timer_tools_handle);
	err = esp_timer_start_periodic(timer_tools_handle, time_interval * 1000);
	if (err != ESP_OK) {
        printf("[timer_tools]:failed to start\r\n");
        return false;
    }
    return true;
}

bool timer_tools_stop() {
    esp_err_t err = esp_timer_stop(timer_tools_handle);
    if (err != ESP_OK) {
        printf("[timer_tools]:failed to stop\r\n");
        return false;
    }
    return true;
}

bool timer_tools_dealloc() {
    esp_err_t err = esp_timer_delete(timer_tools_handle);
    if (err != ESP_OK) {
        printf("[timer_tools]:failed to dealloc\r\n");
        return false;
    }
    return true;
}

