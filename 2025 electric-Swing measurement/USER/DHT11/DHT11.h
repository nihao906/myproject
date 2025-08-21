#ifndef _dht11_H
#define _dht11_H

#include "stm32f10x.h"
#include "delay.h"
#include "sys.h"

int32_t dht11_read_data(uint8_t *pbuf);


#endif
