#ifndef _MKC_PERSIST_MODULE_H_
#define _MKC_PERSIST_MODULE_H_
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_system.h"
#include "mkc_data_structure.h"
    
enum mkc_persist_module_type{
    MKC_PERSIST_MODULE_DUTY,
    MKC_PERSIST_MODULE_MLX90614_SWITCH,
    MKC_PERSIST_MODULE_AUTH,
};
    char* mkc_int2String(int num, char *str);
    int mkc_string2Int(char *str);
    esp_err_t mkc_persist_module_init();
    void mkc_persist_set_data(mkc_protocol_model_t data);
    mkc_protocol_model_t mkc_persist_get_data();

#endif