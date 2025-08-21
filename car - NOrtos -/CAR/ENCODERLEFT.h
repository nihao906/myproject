#ifndef __ENCODERLEFT__H_
#define __ENCODERLEFT__H_
#include "stm32f4xx.h"                  // Device header

void ENCODER_initleft(void);
int16_t Encoder_Getleft(void);
void Encoder_Resetleft(void);

#endif
