#ifndef __FTOC__H_
#define __FTOC__H_
#include "stm32f4xx.h"                  // Device header

void float_to_string(float number, char *buffer, int decimal_places);
void int16_to_str(int16_t num, char *str);

#endif
