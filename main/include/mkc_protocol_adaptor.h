#ifndef _MKC_PROTOCOL_ADAPTOR_H_
#define _MKC_PROTOCOL_ADAPTOR_H_
#include "mkc_data_structure.h"
#include <stdio.h>

mkc_protocol_model_t mkc_protocol_data_to_model(void *data);
void mkc_protocol_model_to_data(void *data, mkc_protocol_model_t model);

#endif