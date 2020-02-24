#ifndef _PERSIST_MODULE_H_
#define _PERSIST_MODULE_H_
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
    
enum persist_module_type{
    PERSIST_MODULE_DUTY,
    PERSIST_MODULE_MLX90614_SWITCH,
    PERSIST_MODULE_AUTH,
};
    char* Int2String(int num, char *str);
    int String2Int(char *str);
    esp_err_t persist_module_init();
    void persist_set_data(enum persist_module_type type, uint16_t value);
    char* persist_get_data(enum persist_module_type type, char* value);

#endif