#ifndef _MKC_DATA_STRUCTURE_H_
#define _MKC_DATA_STRUCTURE_H_
#include <stdio.h>

typedef struct 
{
    uint16_t temp_int;
    uint16_t temp_dec;
} mkc_temp_ir_data_t;

typedef struct
{   
    mkc_temp_ir_data_t temp_ir_a_data;
    mkc_temp_ir_data_t temp_ir_o_data;
    uint8_t hdr;
    uint8_t typ;
    uint8_t len;
    uint8_t *data;
    uint8_t *token;// 4 bytes
    uint8_t crc;
    uint8_t fan_duty;
    uint16_t fan_rpm;
    uint16_t delay;
} mkc_protocol_model_t;

#endif