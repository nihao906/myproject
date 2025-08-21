#ifndef __ADC__H_
#define __ADC__H_
#include "stm32f4xx.h"                  // Device header
void ADCRES_init(void);
uint16_t ADC_getresult(void);
#endif
