#ifndef __DHT11__H_
#define __DHT11__H_
#include "stm32f4xx.h"                  // Device header
void DHT11_init(void);//初始化
int8_t DHT11_getdata(uint8_t *pbuf);//5个字节
#endif
