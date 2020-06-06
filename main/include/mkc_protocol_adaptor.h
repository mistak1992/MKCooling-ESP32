#ifndef _MKC_PROTOCOL_ADAPTOR_H_
#define _MKC_PROTOCOL_ADAPTOR_H_
#include "mkc_data_structure.h"
#include <stdio.h>

void mkc_protocol_data_to_model(uint8_t *data, mkc_protocol_model_t *model);
void mkc_protocol_model_to_data(uint8_t *data, mkc_protocol_model_t *model);

#endif