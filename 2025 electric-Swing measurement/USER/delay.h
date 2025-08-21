#ifndef __DELAY__H
#define __DELAY__H

#include "stm32f10x.h"

void Delay_Init(uint8_t SYSCLK);
void Delay_us(uint32_t nus);
void Delay_ms(uint32_t nms);


#endif
