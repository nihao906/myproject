#ifndef __Kalman__H
#define __Kalman__H

#include "stm32f10x.h"                  
#include "stm32f10x_i2c.h"
#include "MPU6050_Reg.h"
#include "MPU6050.h"
#include <math.h>

typedef struct {
    float Q_angle;
    float Q_bias;
    float R_measure;

    float angle;
    float bias;
    float rate;

    float P[2][2];
} Kalman_t;

void Kalman_Init(Kalman_t *kalman);
float Kalman_GetAngle(Kalman_t *kalman, float newAngle, float newRate, float dt);

void Attitude_Init(void);
void Attitude_Update(float dt, float *pitch, float *roll, float *yaw);

#endif
