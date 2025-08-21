#include "stm32f10x.h"                  // Device header
#include "stm32f10x_i2c.h"
#include "MPU6050_Reg.h"
#include "MPU6050.h"
#include <math.h>
#include "Kalman.h"

void Kalman_Init(Kalman_t *kalman)
{
    kalman->Q_angle = 0.001f;
    kalman->Q_bias = 0.003f;
    kalman->R_measure = 0.03f;

    kalman->angle = 0.0f;
    kalman->bias = 0.0f;

    kalman->P[0][0] = 0.0f;
    kalman->P[0][1] = 0.0f;
    kalman->P[1][0] = 0.0f;
    kalman->P[1][1] = 0.0f;
}

float Kalman_GetAngle(Kalman_t *kalman, float newAngle, float newRate, float dt)
{
    kalman->rate = newRate - kalman->bias;
    kalman->angle += dt * kalman->rate;

    kalman->P[0][0] += dt * (dt*kalman->P[1][1] - kalman->P[0][1] - kalman->P[1][0] + kalman->Q_angle);
    kalman->P[0][1] -= dt * kalman->P[1][1];
    kalman->P[1][0] -= dt * kalman->P[1][1];
    kalman->P[1][1] += kalman->Q_bias * dt;

    float S = kalman->P[0][0] + kalman->R_measure;
    float K0 = kalman->P[0][0] / S;
    float K1 = kalman->P[1][0] / S;

    float y = newAngle - kalman->angle;
    kalman->angle += K0 * y;
    kalman->bias += K1 * y;

    float P00_temp = kalman->P[0][0];
    float P01_temp = kalman->P[0][1];

    kalman->P[0][0] -= K0 * P00_temp;
    kalman->P[0][1] -= K0 * P01_temp;
    kalman->P[1][0] -= K1 * P00_temp;
    kalman->P[1][1] -= K1 * P01_temp;

    return kalman->angle;
}

static Kalman_t kalmanPitch, kalmanRoll;
static float lastYaw = 0.0f;

static float GetPitch(int16_t ax, int16_t ay, int16_t az)
{
    return atan2f((float)ay, sqrtf(ax*ax + az*az)) * 57.3f;
}

static float GetRoll(int16_t ax, int16_t az)
{
    return atan2f((float)-ax, (float)az) * 57.3f;
}

void Attitude_Init(void)
{
    Kalman_Init(&kalmanPitch);
    Kalman_Init(&kalmanRoll);
    lastYaw = 0.0f;
}

void Attitude_Update(float dt, float *pitch, float *roll, float *yaw)
{
    int16_t ax, ay, az, gx, gy, gz;
    MPU6050_GetData(&ax, &ay, &az, &gx, &gy, &gz);

    float accPitch = GetPitch(ax, ay, az);
    float accRoll = GetRoll(ax, az);

    float gyroX = (float)gx / 131.0f;
    float gyroY = (float)gy / 131.0f;
    float gyroZ = (float)gz / 131.0f;

    *pitch = Kalman_GetAngle(&kalmanPitch, accPitch, gyroY, dt);
    *roll  = Kalman_GetAngle(&kalmanRoll, accRoll,  gyroX, dt);
    *yaw  = lastYaw + gyroZ * dt;
    lastYaw = *yaw;
}

///* main.c Ê¾Àý */

//int main(void)
//{
//    float pitch, roll, yaw;
//    float dt = 0.005f; // 5ms

//    MPU6050_Init();
//    Attitude_Init();

//    while (1)
//    {
//        Attitude_Update(dt, &pitch, &roll, &yaw);
//        printf("Pitch: %.2f, Roll: %.2f, Yaw: %.2f\r\n", pitch, roll, yaw);
//        Delay_ms(5);
//    }
//}
