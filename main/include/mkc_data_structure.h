#ifndef _MKC_DATA_STRUCTURE_H_
#define _MKC_DATA_STRUCTURE_H_
#include <stdio.h>
#include <stdlib.h>

typedef struct 
{
    uint8_t temp_int;
    uint8_t temp_dec;
} mkc_temp_ir_data_t;

typedef struct
{   
    mkc_temp_ir_data_t temp_ir_a_data;
    mkc_temp_ir_data_t temp_ir_o_data;
    uint8_t hdr;
    uint8_t typ;
    uint8_t len;
    void *data;
    void *token;// 4 bytes
    uint8_t crc;
    uint8_t fan_duty;
    uint16_t fan_rpm;
    uint8_t delay;
} mkc_protocol_model_t;

typedef struct
{   
    mkc_temp_ir_data_t temp_ir_a_data;
    mkc_temp_ir_data_t temp_ir_o_data;
    uint8_t fan_duty;
    uint16_t fan_rpm;
    uint16_t delay;
} mkc_persist_model_t;

#endif