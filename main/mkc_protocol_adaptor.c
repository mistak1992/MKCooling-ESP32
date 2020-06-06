#include "include/mkc_protocol_adaptor.h"
#include "esp_system.h"
#include <string.h>

#define MKC_PROTOCOL_ADAPTOR_IDX_DATA 3
#define MKC_PROTOCOL_ADAPTOR_IDX_TEMP_IR_O_INT 3
#define MKC_PROTOCOL_ADAPTOR_IDX_TEMP_IR_O_DEC 4
#define MKC_PROTOCOL_ADAPTOR_IDX_TEMP_IR_A_INT 5
#define MKC_PROTOCOL_ADAPTOR_IDX_TEMP_IR_A_DEC 6
#define MKC_PROTOCOL_ADAPTOR_IDX_FAN_DUTY 7
#define MKC_PROTOCOL_ADAPTOR_IDX_FAN_RPM 8
#define MKC_PROTOCOL_ADAPTOR_IDX_DELAY 10

#define MKC_PROTOCOL_ADAPTOR_LEN_FETCHDATAS 8
#define MKC_PROTOCOL_ADAPTOR_LEN_TOKEN 4

mkc_protocol_model_t mkc_protocol_data_to_model(uint8_t *data){
    uint8_t *rawData = data;
    // new model 
    mkc_protocol_model_t model;
    // hdr
    model.hdr = rawData[0];
    // typ
    model.typ = rawData[1];
    // len
    model.len = rawData[2];
    // data
    int currentIdx = 3;
    switch (model.typ)
    {
        case 3:{
            model.fan_duty = rawData[3];
            currentIdx++;
            break;
        }
        case 4:{
            model.delay = rawData[3];
            currentIdx++;
            break;
        }
        default:

            break;
    }
    // token
    uint8_t token[4];

    for (size_t i = 0; i < 16; i++)
    {
        printf("%02x", data[i]);
        if (i == 15)
        {
            printf("\n");
        }else{
            printf(":");
        }
    }
    return model;
}

void mkc_protocol_model_to_data(uint8_t *data, mkc_protocol_model_t model){
    // incase of overflow
    model.len = (model.len > 8 ? 8 : model.len);
    int currentIdx = 0;
    // new rawData
    uint8_t *rawData = calloc(sizeof(uint8_t), 16);
    // hdr
    rawData[0] = model.hdr;
    currentIdx++;
    // typ
    rawData[1] = model.typ;
    currentIdx++;
    // switch typ
    switch (model.typ){
        case 2:{
            rawData[2] = MKC_PROTOCOL_ADAPTOR_LEN_FETCHDATAS;
            currentIdx++;
            rawData[MKC_PROTOCOL_ADAPTOR_IDX_TEMP_IR_O_INT] = model.temp_ir_o_data.temp_int;
            rawData[MKC_PROTOCOL_ADAPTOR_IDX_TEMP_IR_O_DEC] = model.temp_ir_o_data.temp_dec;
            rawData[MKC_PROTOCOL_ADAPTOR_IDX_TEMP_IR_A_INT] = model.temp_ir_a_data.temp_int;
            rawData[MKC_PROTOCOL_ADAPTOR_IDX_TEMP_IR_A_DEC] = model.temp_ir_a_data.temp_dec;
            rawData[MKC_PROTOCOL_ADAPTOR_IDX_FAN_DUTY] = model.fan_duty;
            rawData[MKC_PROTOCOL_ADAPTOR_IDX_FAN_RPM] = model.fan_rpm >> 8;
            rawData[MKC_PROTOCOL_ADAPTOR_IDX_FAN_RPM + 1] = model.fan_rpm & 0x00ff;
            rawData[MKC_PROTOCOL_ADAPTOR_IDX_DELAY] = model.delay;
            currentIdx += MKC_PROTOCOL_ADAPTOR_LEN_FETCHDATAS;
            break;
        }
        default:{
            rawData[2] = model.len;
            currentIdx++;
            memcpy(&rawData[MKC_PROTOCOL_ADAPTOR_IDX_DATA], model.data, model.len * sizeof(uint8_t));
            currentIdx += model.len;
            // token
            memcpy(&rawData[MKC_PROTOCOL_ADAPTOR_IDX_DATA + model.len], model.token, MKC_PROTOCOL_ADAPTOR_LEN_TOKEN * sizeof(uint8_t));
            currentIdx += MKC_PROTOCOL_ADAPTOR_LEN_TOKEN;
            // crc
            rawData[MKC_PROTOCOL_ADAPTOR_IDX_DATA + model.len + MKC_PROTOCOL_ADAPTOR_LEN_TOKEN] = model.crc;
            currentIdx++;
            break;
        }
    }
    // fill with ramdom
    for (size_t i = currentIdx; i < 16; i++)
    {
        rawData[i] = esp_random() & 0x000000ff;//random_number(0, 0xfe);
    }
    // debug print
    for (size_t i = 0; i < 16; i++)
    {
        printf("%02x", rawData[i]);
        if (i == 15)
        {
            printf("\n");
        }else{
            // printf(":");
        }
    }
    memcpy(data, rawData, 16 * sizeof(uint8_t));
    free(rawData);
}