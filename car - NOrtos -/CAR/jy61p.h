#ifndef __JY61P_H
#define __JY61P_H

#include "stm32f4xx.h"                  // Device header

void jy60p_ReceiveData(uint8_t RxData);
void jy60p_ReceiveDataGyro(uint8_t RxData);
extern float Roll,Pitch,Yaw;
extern float Pitch_gyro;

#endif
