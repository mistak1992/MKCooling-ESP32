/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

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

    MKC_IDX_NB,
};

enum mkc_ble_state{
    MKC_BLE_STATE_DISCONNECTED,
    MKC_BLE_STATE_CONNECTED,
};

typedef void (*update_callback_t)(enum mkc_idx_attributes attr_idx, uint16_t value);

    enum mkc_ble_state ble_get_state();
    void set_attributes(enum mkc_idx_attributes attr_idx, uint16_t value);
    uint16_t get_attrubutes(enum mkc_idx_attributes attr_idx);
    void ble_module_init(update_callback_t update_callback);
    void ble_module_reset();
    void ble_module_deinit();
    void set_sleep(bool is_sleep);
    void switch_to_reset_mode(bool is_on);

#endif