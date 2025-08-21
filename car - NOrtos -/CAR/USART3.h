#ifndef __USART3__H_
#define __USART3__H_
#include "stm32f4xx.h"                  // Device header

void USART3_init(void);
void USART3_senddata(uint8_t data);
void USART3_sendstring(char*string);
#endif
