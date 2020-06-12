#ifndef _MKC_TIMER_TOOLS_H_
#define _MKC_TIMER_TOOLS_H_
#include "esp_timer.h"

    typedef void (*timer_tools_callback_t)(void* arg);
    bool mkc_timer_tools_Init(timer_tools_callback_t callback, uint64_t interval);
    bool mkc_timer_tools_start();
    bool mkc_timer_tools_stop();
    bool mkc_timer_tools_dealloc();

#endif

