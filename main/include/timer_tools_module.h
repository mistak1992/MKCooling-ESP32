#ifndef _TIMER_TOOLS_H_
#define _TIMER_TOOLS_H_
#include "esp_timer.h"

    typedef void (*timer_tools_callback_t)(void* arg);
    bool timer_tools_Init(timer_tools_callback_t callback, uint64_t interval);
    bool timer_tools_start();
    bool timer_tools_stop();
    bool timer_tools_dealloc();

#endif

