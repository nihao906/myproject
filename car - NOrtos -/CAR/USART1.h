#ifndef __USART1__H_
#define __USART1__H_
#include "stm32f4xx.h"                  // Device header

void USART1_init(void);
void USART1_senddata(uint8_t data);
void USART1_sendstring(char*string);
#endif
