#ifndef __TIME__H
#define __TIME__H

#include "stm32f10x.h"

void Encoder_Init(void);
void Timer_Config(void);
void NVIC_Config(void);
void Timer_Stop(void);
void Timer_Start(void);
int16_t Encoder_Get(void);

#endif
