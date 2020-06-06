/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)
{
    "explorer.confirmDelete": false,
    "terminal.integrated.setLocaleVariables": true,
    "C_Cpp.updateChannel": "Insiders",
    "dart.openDevTools": "flutter",
    "dart.debugExternalLibraries": false,
    "dart.debugSdkLibraries": false,
    "terminal.integrated.shell.osx": "/bin/zsh",
    "idf.pythonSystemBinPath": "/Users/mist/Documents/esp/python_env/idf4.0_py2.7_env/bin/python",
    "idf.espIdfPath": "/Users/mist/Documents/esp/esp-idf",
    "idf.toolsPath": "/Users/mist/Documents/esp",
    "idf.customExtraPaths": ".:/usr/bin:/Users/mist/Documents/esp/tools/xtensa-esp32-elf/esp-2019r2-8.2.0/xtensa-esp32-elf/bin:/Users/mist/Documents/esp/tools/esp32ulp-elf/2.28.51.20170517/esp32ulp-elf-binutils/bin:/Users/mist/Documents/esp/tools/openocd-esp32/v0.10.0-esp32-20190313/openocd-esp32/bin",
    "idf.customExtraVars": "{\"OPENOCD_SCRIPTS\":\"/Users/mist/Documents/esp/tools/openocd-esp32/v0.10.0-esp32-20190313/openocd-esp32/share/openocd/scripts\"}",
    "idf.showOnboardingOnInit": false
}

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifndef _BLE_MODULE_H_
#define _BLE_MODULE_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"


/*
 * DEFINES
 ****************************************************************************************
 */

#define MKC_IR_TEMP_MAX_LEN            (13)

#define MKC_TEMP_MASK             (0x0F)
#define MKC_IR_TEMP_MASK       (0x30)
#define MKC_FAN_SPEED_MASK            (0xC0)

///Attributes State Machine
enum mkc_idx_attributes 
{
    MKC_IDX_SVC,

    MKC_IDX_TEMP_INT_CHAR,
    MKC_IDX_TEMP_INT_VAL,

    MKC_IDX_TEMP_DEC_CHAR,
    MKC_IDX_TEMP_DEC_VAL,

    MKC_IDX_IR_TEMPO_INT_CHAR,
    MKC_IDX_IR_TEMPO_INT_VAL,

    MKC_IDX_IR_TEMPO_DEC_CHAR,
    MKC_IDX_IR_TEMPO_DEC_VAL,

    MKC_IDX_IR_TEMPA_INT_CHAR,
    MKC_IDX_IR_TEMPA_INT_VAL,

    MKC_IDX_IR_TEMPA_DEC_CHAR,
    MKC_IDX_IR_TEMPA_DEC_VAL,

    MKC_IDX_FAN_SPEED_RPM_CHAR,
    MKC_IDX_FAN_SPEED_RPM_VAL,

    MKC_IDX_FAN_SPEED_PERCENTAGE_CHAR,
    MKC_IDX_FAN_SPEED_PERCENTAGE_VAL,

    MKC_IDX_SWITCHA_CHAR,
    MKC_IDX_SWITCHA_VAL,

    MKC_IDX_AUTH_CHAR,
    MKC_IDX_AUTH_VAL,

    MKC_IDX_DELAY_CHAR,
    MKC_IDX_DELAY_VAL,

    MKC_IDX_WRITEIN_CHAR,
    MKC_IDX_WRITEIN_VAL,

    MKC_IDX_NB,
};

enum mkc_ble_state{
    MKC_BLE_STATE_DISCONNECTED,
    MKC_BLE_STATE_CONNECTED,
};

typedef void (*receive_datas_callback_t)(enum mkc_idx_attributes attr_idx, uint16_t value);

    enum mkc_ble_state ble_get_state();
    void set_attributes(enum mkc_idx_attributes attr_idx, uint16_t value);
    uint16_t get_attrubutes(enum mkc_idx_attributes attr_idx);
    void ble_module_init(receive_datas_callback_t update_callback);
    void ble_module_reset();
    void ble_module_deinit();
    void set_sleep(bool is_sleep);
    void switch_to_reset_mode(bool is_on);

#endif