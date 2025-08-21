#ifndef __MOTOR__H
#define __MOTOR__H

#include "stm32f10x.h"

void PWM_Config(void);
void Motor_SetSpeed(uint16_t speed);
void Motor_SetDirection(uint8_t direction);

#endif
