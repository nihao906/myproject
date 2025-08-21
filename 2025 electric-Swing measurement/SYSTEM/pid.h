#ifndef __PID_H
#define __PID_H

typedef struct {
    float kp;
    float ki;
    float kd;

    float integral;
    float last_error;

    float output_limit;
} PID_t;

void PID_Init(PID_t *pid, float kp, float ki, float kd, float limit);
float PID_Compute(PID_t *pid, float target, float measured);

#endif


///* balance_control.c */
//#include "attitude.h"
//#include "pid.h"
//#include <stdio.h>

//static PID_t anglePID;     // 姿态角度 PID
//static PID_t speedPID;     // 速度 PID

//void BalanceControl_Init(void)
//{
//    PID_Init(&anglePID, 35.0f, 0.0f, 0.8f, 300);  // 角度 PID
//    PID_Init(&speedPID, 0.6f, 0.01f, 0.0f, 200);   // 速度 PID
//}

//int16_t GetMotorSpeed(void); // 编码器读取函数 (需用户实现)
//void SetMotorPWM(int16_t pwm); // 电机PWM设置函数 (需用户实现)

//void BalanceControl_Loop(float dt)
//{
//    float pitch, roll, yaw;
//    Attitude_Update(dt, &pitch, &roll, &yaw); // 获取姿态角

//    float angleTarget = 0.0f; // 期望角度为 0°（直立）
//    float angleOutput = PID_Compute(&anglePID, angleTarget, pitch);

//    int16_t speed = GetMotorSpeed(); // 读取编码器速度
//    float speedOutput = PID_Compute(&speedPID, 0, speed);

//    int16_t pwm = (int16_t)(angleOutput + speedOutput);
//    SetMotorPWM(pwm); // 设置电机速度
//}

///* 示例 main.c 循环调用 */
//#include "balance_control.h"

//int main(void)
//{
//    MPU6050_Init();
//    Attitude_Init();
//    BalanceControl_Init();

//    while (1)
//    {
//        BalanceControl_Loop(0.005f); // 5ms 一次
//        Delay_ms(5);
//    }
//}
