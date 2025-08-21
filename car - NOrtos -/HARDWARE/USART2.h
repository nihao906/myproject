#ifndef __USART2__H_
#define __USART2__H_
#include "stm32f4xx.h"                  // Device header

void USART2_init(void);
void USART2_senddata(uint8_t data);
void USART2_sendstring(char*string);
#endif
