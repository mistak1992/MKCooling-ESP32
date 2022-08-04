/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "include/mkc_ble_module.h"
#include "esp_system.h"

#define GATTS_TABLE_TAG "BLE_MODULE"

#define GATT_UUID_MKCOOLING_SVC                       0x1890
#define MKCOOLING_PROFILE_NUM                         1
#define MKCOOLING_PROFILE_APP_IDX                     0
#define MKCOOLING_APP_ID                              0x55
// 开发中使用别名代替 "MKCooling"
#define EXCAMPLE_DEVICE_NAME                          "MKCooling"
#define MKCOOLING_SVC_INST_ID                         0

#define GATTS_DEMO_CHAR_VAL_LEN_MAX                   0x40

#define ADV_CONFIG_FLAG                               (1 << 0)
#define SCAN_RSP_CONFIG_FLAG                          (1 << 1)

bool user_need_add_device = false; 

bool switcher = false; 

enum mkc_ble_state state;

static receive_datas_callback_t receive_callback_t;

static compose_response_callback_t response_callback_t;

static uint8_t adv_config_done = 0;

static uint16_t cooling_handle_table[MKC_IDX_NB];

uint8_t local_mac_address[6];

uint8_t manufacturer_data[12]={'M', 'K', 0x00, 0x01};

static uint8_t sec_service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x18, 0x90, 0x00, 0x00,
};

bool canNotify = false;

esp_gatt_if_t global_gatt_if;

uint16_t global_conn_id;

uint32_t global_trans_id;

// config adv data
esp_ble_adv_data_t cooling_adv_config = {
    .set_scan_rsp = false,
    .include_txpower = true,
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = sizeof(manufacturer_data),
    .p_manufacturer_data = manufacturer_data,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(sec_service_uuid),
    .p_service_uuid = sec_service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
// config scan response data
static esp_ble_adv_data_t cooling_scan_rsp_config = {
    .set_scan_rsp = true,
    .include_name = true,
    .manufacturer_len = sizeof(manufacturer_data),
    .p_manufacturer_data = manufacturer_data,
};

static esp_ble_adv_params_t cooling_adv_add_device_params = {
    .adv_int_min        = 0x100,
    .adv_int_max        = 0x100,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_ble_adv_params_t cooling_adv_normal_params = {
    .adv_int_min        = 0x100,
    .adv_int_max        = 0x100,
    .adv_type           = ADV_TYPE_DIRECT_IND_HIGH,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_WLST_CON_WLST,
};

struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

void auto_kick_on();

static enum mkc_idx_attributes get_idx_attributes(uint16_t handle){
    for (size_t i = 0; i < MKC_IDX_NB; i++)
    {
        if (handle == cooling_handle_table[i])
        {
            return i;
        }
    }
    return 0;
}

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst cooling_profile_tab[MKCOOLING_PROFILE_NUM] = {
    [MKCOOLING_PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },

};

/*
 *  MKCOOLING PROFILE ATTRIBUTES
 ****************************************************************************************
 */

/// MKCooling Service
static const uint16_t cooling_svc = GATT_UUID_MKCOOLING_SVC;

#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))
static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
// static const uint8_t char_prop_notify = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
// static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
// static const uint8_t char_prop_read_write = ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_read_write_notify = ESP_GATT_CHAR_PROP_BIT_WRITE|ESP_GATT_CHAR_PROP_BIT_READ|ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t write_in_ccc[2]      = {0x00, 0x00};

/// MKCooling Sensor Service - MKCooling WriteIn, write&read
static const uint16_t cooling_write_in_uuid = 0x1891;
static uint8_t write_in_val[16] = {0x01};

/// Full HRS Database Description - Used to add attributes into the database
static const esp_gatts_attr_db_t cooling_gatt_db[MKC_IDX_NB] =
{
    // MKCooling Service Declaration
    [MKC_IDX_SVC] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ_ENCRYPTED,
      sizeof(uint16_t), sizeof(cooling_svc), (uint8_t *)&cooling_svc}},

    // MKCooling Delay Characteristic Declaration
    [MKC_IDX_WRITEIN_CHAR] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ_ENCRYPTED,
      CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    // MKCooling WriteIn Characteristic Value  |ESP_GATT_PERM_READ_ENCRYPTED
    [MKC_IDX_WRITEIN_VAL] =
    {{ESP_GATT_RSP_BY_APP}, {ESP_UUID_LEN_16, (uint8_t *)&cooling_write_in_uuid, ESP_GATT_PERM_WRITE_ENCRYPTED,
      sizeof(uint8_t) * 16, sizeof(write_in_val), (uint8_t *)write_in_val}},

    [MKC_IDX_WRITEIN_CHAR_CFG]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(write_in_ccc), (uint8_t *)write_in_ccc}},
};

static char *esp_key_type_to_str(esp_ble_key_type_t key_type)
{
   char *key_str = NULL;
   switch(key_type) {
    case ESP_LE_KEY_NONE:
        key_str = "ESP_LE_KEY_NONE";
        break;
    case ESP_LE_KEY_PENC:
        key_str = "ESP_LE_KEY_PENC";
        break;
    case ESP_LE_KEY_PID:
        key_str = "ESP_LE_KEY_PID";
        break;
    case ESP_LE_KEY_PCSRK:
        key_str = "ESP_LE_KEY_PCSRK";
        break;
    case ESP_LE_KEY_PLK:
        key_str = "ESP_LE_KEY_PLK";
        break;
    case ESP_LE_KEY_LLK:
        key_str = "ESP_LE_KEY_LLK";
        break;
    case ESP_LE_KEY_LENC:
        key_str = "ESP_LE_KEY_LENC";
        break;
    case ESP_LE_KEY_LID:
        key_str = "ESP_LE_KEY_LID";
        break;
    case ESP_LE_KEY_LCSRK:
        key_str = "ESP_LE_KEY_LCSRK";
        break;
    default:
        key_str = "INVALID BLE KEY TYPE";
        break;

   }

   return key_str;
}

static char *esp_auth_req_to_str(esp_ble_auth_req_t auth_req)
{
   char *auth_str = NULL;
   switch(auth_req) {
    case ESP_LE_AUTH_NO_BOND:
        auth_str = "ESP_LE_AUTH_NO_BOND";
        break;
    case ESP_LE_AUTH_BOND:
        auth_str = "ESP_LE_AUTH_BOND";
        break;
    case ESP_LE_AUTH_REQ_MITM:
        auth_str = "ESP_LE_AUTH_REQ_MITM";
        break;
    case ESP_LE_AUTH_REQ_BOND_MITM:
        auth_str = "ESP_LE_AUTH_REQ_BOND_MITM";
        break;
    case ESP_LE_AUTH_REQ_SC_ONLY:
        auth_str = "ESP_LE_AUTH_REQ_SC_ONLY";
        break;
    case ESP_LE_AUTH_REQ_SC_BOND:
        auth_str = "ESP_LE_AUTH_REQ_SC_BOND";
        break;
    case ESP_LE_AUTH_REQ_SC_MITM:
        auth_str = "ESP_LE_AUTH_REQ_SC_MITM";
        break;
    case ESP_LE_AUTH_REQ_SC_MITM_BOND:
        auth_str = "ESP_LE_AUTH_REQ_SC_MITM_BOND";
        break;
    default:
        auth_str = "INVALID BLE AUTH REQ";
        break;
   }

   return auth_str;
}

esp_err_t send_notify(enum mkc_idx_attributes attr_idx, uint16_t len, uint8_t *value){
    switch (attr_idx)
    {
    case MKC_IDX_WRITEIN_VAL:{
        if (canNotify == true){
            // memcpy(&write_in_val, value, sizeof(uint8_t) * 16);
            esp_ble_gatts_set_attr_value(cooling_handle_table[MKC_IDX_WRITEIN_VAL], len, value);
            esp_ble_gatts_send_indicate(global_gatt_if, global_conn_id, cooling_handle_table[MKC_IDX_WRITEIN_VAL], len, value, false);
            // for (size_t i = 0; i < 16; i++)
            // {
            //     printf("%02x", value[i]);
            //     if (i == 15)
            //     {
            //         printf("\n");
            //     }else{
            //         printf(":");
            //     }
            // }
            return ESP_OK;
        }else{
            return ESP_FAIL;
        }
        break;
    }
    default:
        return ESP_FAIL;
        break;
    }
}

static void show_bonded_devices(void)
{
    int dev_num = esp_ble_get_bond_device_num();

    esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    esp_ble_get_bond_device_list(&dev_num, dev_list);
    ESP_LOGI(GATTS_TABLE_TAG, "Bonded devices number : %d\n", dev_num);

    ESP_LOGI(GATTS_TABLE_TAG, "Bonded devices list : %d\n", dev_num);
    for (int i = 0; i < dev_num; i++) {
        esp_log_buffer_hex(GATTS_TABLE_TAG, (void *)dev_list[i].bd_addr, sizeof(esp_bd_addr_t));
    }

    uint16_t len;
    esp_ble_gap_get_whitelist_size(&len);
    ESP_LOGE(GATTS_TABLE_TAG, "whitelist length:%d", len);

    free(dev_list);
}

static void __attribute__((unused)) remove_all_bonded_devices(void)
{
    int dev_num = esp_ble_get_bond_device_num();

    esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
    esp_ble_get_bond_device_list(&dev_num, dev_list);
    for (int i = 0; i < dev_num; i++) {
        esp_ble_remove_bond_device(dev_list[i].bd_addr);
        esp_ble_gap_update_whitelist(false, dev_list[i].bd_addr, BLE_ADDR_TYPE_PUBLIC);
    }

    free(dev_list);
}

void ble_gap_start_advertising_custom(){
    esp_ble_gap_stop_advertising();
    if (user_need_add_device == true || esp_ble_get_bond_device_num() <= 0)
    {
        esp_ble_gap_start_advertising(&cooling_adv_add_device_params);
        
    }else if(esp_ble_get_bond_device_num() >= 1){  
        int dev_num = esp_ble_get_bond_device_num();
        esp_ble_bond_dev_t *dev_list = (esp_ble_bond_dev_t *)malloc(sizeof(esp_ble_bond_dev_t) * dev_num);
        esp_ble_get_bond_device_list(&dev_num, dev_list);
        memcpy(cooling_adv_normal_params.peer_addr, dev_list[0].bd_addr, sizeof(esp_bd_addr_t));
        cooling_adv_normal_params.peer_addr_type = BLE_ADDR_TYPE_PUBLIC;
        esp_ble_gap_start_advertising(&cooling_adv_normal_params);
    }
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    ESP_LOGV(GATTS_TABLE_TAG, "GAP_EVT, event %d\n", event);

    switch (event) {
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
        if (adv_config_done == 0){
            ble_gap_start_advertising_custom();
        }
        break;
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~ADV_CONFIG_FLAG);
        if (adv_config_done == 0){
            ble_gap_start_advertising_custom();
        }
        break;
    case ESP_GAP_BLE_UPDATE_WHITELIST_COMPLETE_EVT:{
        //the unit of the duration is second
        break;
    }
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TABLE_TAG, "advertising start failed, error status = %x", param->adv_start_cmpl.status);
            break;
        }
        ESP_LOGI(GATTS_TABLE_TAG, "advertising start success");
        break;
    case ESP_GAP_BLE_PASSKEY_REQ_EVT:                           /* passkey request event */
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_PASSKEY_REQ_EVT");
        /* Call the following function to input the passkey which is displayed on the remote device */
        //esp_ble_passkey_reply(cooling_profile_tab[HEART_PROFILE_APP_IDX].remote_bda, true, 0x00);
        break;
    case ESP_GAP_BLE_OOB_REQ_EVT: {
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_OOB_REQ_EVT");
        uint8_t tk[16] = {1}; //If you paired with OOB, both devices need to use the same tk
        esp_ble_oob_req_reply(param->ble_security.ble_req.bd_addr, tk, sizeof(tk));
        break;
    }
    case ESP_GAP_BLE_LOCAL_IR_EVT:                               /* BLE local IR event */
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_LOCAL_IR_EVT");
        break;
    case ESP_GAP_BLE_LOCAL_ER_EVT:                               /* BLE local ER event */
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_LOCAL_ER_EVT");
        break;
    case ESP_GAP_BLE_NC_REQ_EVT:
        /* The app will receive this evt when the IO has DisplayYesNO capability and the peer device IO also has DisplayYesNo capability.
        show the passkey number to the user to confirm it with the number displayed by peer device. */
        esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, true);
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_NC_REQ_EVT, the passkey Notify number:%d", param->ble_security.key_notif.passkey);
        break;
    case ESP_GAP_BLE_SEC_REQ_EVT:
        /* send the positive(true) security response to the peer device to accept the security request.
        If not accept the security request, should send the security response with negative(false) accept value*/
        esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_SEC_REQ_EVT, the passkey :%d %d", param->ble_security.ble_key.p_key_value.penc_key.ediv, param->ble_security.ble_key.p_key_value.lenc_key.div);
        break;
    case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:  ///the app will receive this evt when the IO  has Output capability and the peer device IO has Input capability.
        ///show the passkey number to the user to input it in the peer device.
        ESP_LOGI(GATTS_TABLE_TAG, "The passkey Notify number:%06d", param->ble_security.key_notif.passkey);
        break;
    case ESP_GAP_BLE_KEY_EVT:
        //shows the ble key info share with peer device to the user.
        ESP_LOGI(GATTS_TABLE_TAG, "key type = %s, value = %d %d", esp_key_type_to_str(param->ble_security.ble_key.key_type), param->ble_security.ble_key.p_key_value.penc_key.ediv, param->ble_security.ble_key.p_key_value.lenc_key.div);
        break;
    case ESP_GAP_BLE_AUTH_CMPL_EVT: {
        esp_bd_addr_t bd_addr;
        memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTS_TABLE_TAG, "remote BD_ADDR: %08x%04x",\
                (bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
                (bd_addr[4] << 8) + bd_addr[5]);
        ESP_LOGI(GATTS_TABLE_TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
        ESP_LOGI(GATTS_TABLE_TAG, "pair status = %s",param->ble_security.auth_cmpl.success ? "success" : "fail");
        if(!param->ble_security.auth_cmpl.success) {
            ESP_LOGI(GATTS_TABLE_TAG, "fail reason = 0x%x",param->ble_security.auth_cmpl.fail_reason);
        } else {
            ESP_LOGI(GATTS_TABLE_TAG, "auth mode = %s",esp_auth_req_to_str(param->ble_security.auth_cmpl.auth_mode));
        }
        // show_bonded_devices();
        // auto_kick_on();
        break;
    }
    case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT: {
        ESP_LOGD(GATTS_TABLE_TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT status = %d", param->remove_bond_dev_cmpl.status);
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV");
        ESP_LOGI(GATTS_TABLE_TAG, "-----ESP_GAP_BLE_REMOVE_BOND_DEV----");
        esp_log_buffer_hex(GATTS_TABLE_TAG, (void *)param->remove_bond_dev_cmpl.bd_addr, sizeof(esp_bd_addr_t));
        ESP_LOGI(GATTS_TABLE_TAG, "------------------------------------");
        break;
    }
    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT:
        if (param->local_privacy_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(GATTS_TABLE_TAG, "config local privacy failed, error status = %x", param->local_privacy_cmpl.status);
            break;
        }
        esp_err_t ret = esp_ble_gap_config_adv_data(&cooling_adv_config);
        if (ret){
            ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
        }else{
            adv_config_done |= ADV_CONFIG_FLAG;
        }

        ret = esp_ble_gap_config_adv_data(&cooling_scan_rsp_config);
        if (ret){
            ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
        }else{
            adv_config_done |= SCAN_RSP_CONFIG_FLAG;
        }

        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS){
            ESP_LOGE(GATTS_TABLE_TAG, "adv stop failed, error status = %x", param->adv_stop_cmpl.status);
            break;
        }
        ESP_LOGI(GATTS_TABLE_TAG, "stop adv successfully");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
         ESP_LOGI(GATTS_TABLE_TAG, "update connetion params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                  param->update_conn_params.status,
                  param->update_conn_params.min_int,
                  param->update_conn_params.max_int,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
                                        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{   
    global_gatt_if = gatts_if;
    ESP_LOGV(GATTS_TABLE_TAG, "event = %x\n",event);
    switch (event) {
        case ESP_GATTS_REG_EVT:
            esp_ble_gap_set_device_name(EXCAMPLE_DEVICE_NAME);
            //generate a resolvable random address
            esp_ble_gap_config_local_privacy(true);
            esp_ble_gatts_create_attr_tab(cooling_gatt_db, gatts_if,
                                      MKC_IDX_NB, MKCOOLING_SVC_INST_ID);
            // esp_ble_gatts_send_indicate(gatts_if, param->read.conn_id, param->read.trans_id, gatt_status, &rsp);
            break;
        case ESP_GATTS_READ_EVT:{
            global_conn_id = param->read.conn_id;
            global_trans_id = param->read.trans_id;
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_READ_EVT");
            esp_gatt_status_t gatt_status = ESP_GATT_OK;
            esp_gatt_rsp_t rsp;
            memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
      		rsp.attr_value.handle = param->read.handle;
       		rsp.attr_value.len = sizeof(write_in_val);
       		// rsp.attr_value.value[0] = get_attrubutes_with_handle(param->read.handle);
            uint8_t *response_datas = calloc(16, sizeof(uint8_t));
            if (get_idx_attributes(param->read.handle) == MKC_IDX_WRITEIN_VAL)
            {
                esp_err_t ret = response_callback_t(MKC_BLE_RESPONSE_TYP_INFO, write_in_val);
                if (ret == ESP_OK)
                {
                    rsp.attr_value.len = sizeof(write_in_val);
                    // rsp.attr_value.value = write_in_val;
                    memcpy(&(rsp.attr_value.value), write_in_val, sizeof(uint8_t) * 16);
                    gatt_status = ESP_GATT_OK;
                }else{
                    gatt_status = ESP_GATT_UNKNOWN_ERROR;
                }
            }
       		esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id, gatt_status, &rsp);
            free(response_datas);
            break;
        }
        case ESP_GATTS_WRITE_EVT:{  
            global_conn_id = param->write.conn_id;
            global_trans_id = param->write.trans_id;
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_WRITE_EVT, write value:");
            esp_log_buffer_hex(GATTS_TABLE_TAG, param->write.value, param->write.len);
            // printf("handle:%d conn:%d trans:%d\n", param->write.handle, param->write.conn_id, param->write.trans_id);
            if (cooling_handle_table[MKC_IDX_WRITEIN_CHAR_CFG] == param->write.handle && param->write.len == 2){
                uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
                if (descr_value == 0x0001){
                    ESP_LOGI(GATTS_TABLE_TAG, "notify enable");
                    uint8_t notify_data[15];
                    for (int i = 0; i < sizeof(notify_data); ++i)
                    {
                        notify_data[i] = i % 0xff;
                    }
                    //the size of notify_data[] need less than MTU size
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, cooling_handle_table[MKC_IDX_WRITEIN_CHAR_CFG],
                                            sizeof(notify_data), notify_data, false);
                }else if (descr_value == 0x0002){
                    ESP_LOGI(GATTS_TABLE_TAG, "indicate enable");
                    uint8_t indicate_data[15];
                    for (int i = 0; i < sizeof(indicate_data); ++i)
                    {
                        indicate_data[i] = i % 0xff;
                    }
                    //the size of indicate_data[] need less than MTU size
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, cooling_handle_table[MKC_IDX_WRITEIN_CHAR_CFG],
                                        sizeof(indicate_data), indicate_data, true);
                }
                else if (descr_value == 0x0000){
                    ESP_LOGI(GATTS_TABLE_TAG, "notify/indicate disable ");
                }else{
                    ESP_LOGE(GATTS_TABLE_TAG, "unknown descr value");
                    esp_log_buffer_hex(GATTS_TABLE_TAG, param->write.value, param->write.len);
                }
                return;
            }
            esp_log_buffer_hex(GATTS_TABLE_TAG, write_in_val, sizeof(uint8_t) * 16);
            esp_gatt_rsp_t rsp;
            esp_gatt_status_t gatt_status = ESP_GATT_OK;
            uint8_t *response_datas = calloc(16, sizeof(uint8_t));
            if (get_idx_attributes(param->write.handle) == MKC_IDX_WRITEIN_VAL)
            {
                esp_err_t ret = receive_callback_t(MKC_IDX_WRITEIN_VAL, param->write.len, param->write.value);
                if (ret == ESP_OK)
                {
                    gatt_status = ESP_GATT_OK;
                    ret = response_callback_t(MKC_BLE_RESPONSE_TYP_STATUS, response_datas);
                    if (ret == ESP_OK)
                    {
                        rsp.attr_value.len = sizeof(write_in_val);
                        // rsp.attr_value.value = &response_datas;
                        memcpy(&(rsp.attr_value.value), response_datas, sizeof(uint8_t) * 16);
                        gatt_status = ESP_GATT_OK;
                    }else{
                        gatt_status = ESP_GATT_UNKNOWN_ERROR;
                    }
                }else{  
                    gatt_status = ESP_GATT_UNKNOWN_ERROR;
                }
            }
       		esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, gatt_status, &rsp);
            free(response_datas);
            break;
        }
        case ESP_GATTS_EXEC_WRITE_EVT:{
            global_conn_id = param->exec_write.conn_id;
            global_trans_id = param->write.trans_id;
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT");
            break;
        }
            
        case ESP_GATTS_RESPONSE_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_RESPONSE_EVT");
            break;
        case ESP_GATTS_MTU_EVT:
            global_conn_id = param->mtu.conn_id;
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_MTU_EVT");
            break;
        case ESP_GATTS_CONF_EVT:
            global_conn_id = param->conf.conn_id;
            // ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONF_EVT");
            break;
        case ESP_GATTS_UNREG_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_UNREG_EVT");
            break;
        case ESP_GATTS_DELETE_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DELETE_EVT");
            break;
        case ESP_GATTS_START_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_START_EVT");
            break;
        case ESP_GATTS_STOP_EVT:
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_STOP_EVT");
            break;
        case ESP_GATTS_CONNECT_EVT:{
            global_conn_id = param->connect.conn_id;
            canNotify = true;
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONNECT_EVT");
            /* start security connect with peer device when receive the connect event sent by the master */
            esp_ble_set_encryption(param->connect.remote_bda, ESP_BLE_SEC_ENCRYPT_NO_MITM);
            state = MKC_BLE_STATE_CONNECTED;
            break;
        }    
        case ESP_GATTS_DISCONNECT_EVT:
            global_conn_id = param->disconnect.conn_id;
            canNotify = false;
            ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
            /* start advertising again when missing the connect */
            ble_gap_start_advertising_custom();
            state = MKC_BLE_STATE_DISCONNECTED;
            break;
        case ESP_GATTS_OPEN_EVT:
            break;
        case ESP_GATTS_CANCEL_OPEN_EVT:
            break;
        case ESP_GATTS_CLOSE_EVT:
            global_conn_id = param->close.conn_id;
            break;
        case ESP_GATTS_LISTEN_EVT:
            break;
        case ESP_GATTS_CONGEST_EVT:
            global_conn_id = param->congest.conn_id;
            break;
        case ESP_GATTS_CREAT_ATTR_TAB_EVT: {
            ESP_LOGI(GATTS_TABLE_TAG, "The number handle = %x",param->add_attr_tab.num_handle);
            if (param->create.status == ESP_GATT_OK){
                if(param->add_attr_tab.num_handle == MKC_IDX_NB) {
                    memcpy(cooling_handle_table, param->add_attr_tab.handles,
                    sizeof(cooling_handle_table));
                    esp_ble_gatts_start_service(cooling_handle_table[MKC_IDX_SVC]);
                }else{
                    ESP_LOGE(GATTS_TABLE_TAG, "Create attribute table abnormally, num_handle (%d) doesn't equal to HRS_IDX_NB(%d)",
                         param->add_attr_tab.num_handle, MKC_IDX_NB);
                }
            }else{
                ESP_LOGE(GATTS_TABLE_TAG, " Create attribute table failed, error code = %x", param->create.status);
            }
        break;
    }

        default:
           break;
    }
}


static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param)
{
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            cooling_profile_tab[MKCOOLING_PROFILE_APP_IDX].gatts_if = gatts_if;
        } else {
            ESP_LOGI(GATTS_TABLE_TAG, "Reg app failed, app_id %04x, status %d\n",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    do {
        int idx;
        for (idx = 0; idx < MKCOOLING_PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gatts_if == cooling_profile_tab[idx].gatts_if) {
                if (cooling_profile_tab[idx].gatts_cb) {
                    cooling_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

void mkc_ble_module_init(receive_datas_callback_t receive_callback, compose_response_callback_t response_callback)
{
    if (receive_callback != NULL)
    {
        receive_callback_t = receive_callback;
    }
    if (response_callback != NULL)
    {
        response_callback_t = response_callback;
    }

    esp_err_t ret;

    // check Mac Address
    ESP_LOGE(GATTS_TABLE_TAG, "========== mac address =========");
    esp_read_mac(local_mac_address, ESP_MAC_BT);
    // load MAC address to manufacturer_data
    memcpy(&manufacturer_data[4], local_mac_address, sizeof(local_mac_address));
    // for (size_t i = 0; i < 6; i++)
    // {
    //     manufacturer_data[i + 3] = local_mac_address[i];
    // }

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGE(GATTS_TABLE_TAG, "NVS init");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ESP_LOGI(GATTS_TABLE_TAG, "%s init bluetooth", __func__);
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gap register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gatts_app_register(MKCOOLING_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    /* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND;     //bonding with peer device after authentication
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
    uint8_t key_size = 16;      //the key size should be 7~16 bytes
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    //set static passkey
    uint32_t passkey = 123456;
    uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
    uint8_t oob_support = ESP_BLE_OOB_DISABLE;
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_OOB_SUPPORT, &oob_support, sizeof(uint8_t));
    /* If your BLE device acts as a Slave, the init_key means you hope which types of key of the master should distribute to you,
    and the response key means which key you can distribute to the master;
    If your BLE device acts as a master, the response key means you hope which types of key of the slave should distribute to you,
    and the init key means which key you can distribute to the slave. */
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));

    /* Just show how to clear all the bonded devices
     * Delay 30s, clear all the bonded devices
     *
     * vTaskDelay(30000 / portTICK_PERIOD_MS);
     * remove_all_bonded_devices();
     */
    // remove_all_bonded_devices();
    // mkc_ble_module_reset();
    show_bonded_devices();
}

void mkc_ble_module_reset(){
    user_need_add_device = true;
    // callback_t(MKC_IDX_AUTH_VAL, 0x00);
    remove_all_bonded_devices();
    ble_gap_start_advertising_custom();
}

void mkc_ble_module_deinit(){
    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();
}

enum mkc_ble_state mkc_ble_get_state(){
    return state;
}

void mkc_set_sleep(bool need_sleep){
    if (need_sleep == true){
        esp_bt_sleep_enable();
    }else{
        esp_bt_sleep_disable();
    }
}

void mkc_switch_to_reset_mode(bool is_on){
    switcher = is_on;
}

void auto_kick_handle(){
    // if (auth_val[0] != 0x33){
    //     mkc_ble_module_reset();
    // }else{
    //     ESP_LOGE(GATTS_TABLE_TAG, "Auth success");
    // }
}

void auto_kick_on(){
    esp_timer_create_args_t auto_kick_on_arg = { 
        .callback = &auto_kick_handle, //设置回调函数
        .arg = NULL, //不携带参数
        .name = "ble_kick" //定时器名字
    };
    esp_timer_handle_t handle = (esp_timer_handle_t)auto_kick_handle;
    esp_err_t err = esp_timer_create(&auto_kick_on_arg, &handle);
    err = esp_timer_start_once(handle, 1000 * 1000);
	if (err != ESP_OK) {
        ESP_LOGE(GATTS_TABLE_TAG, "failed to start");
    }
}


