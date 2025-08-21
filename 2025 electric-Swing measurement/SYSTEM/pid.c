#include "stm32f10x.h"
#include "pid.h"

void PID_Init(PID_t *pid, float kp, float ki, float kd, float limit)
{
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0;
    pid->last_error = 0;
    pid->output_limit = limit;
}

float PID_Compute(PID_t *pid, float target, float measured)
{
    float error = target - measured;
    pid->integral += error;
    float derivative = error - pid->last_error;
    pid->last_error = error;

    float output = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;

    // ÏŞ·ù
    if (output > pid->output_limit) output = pid->output_limit;
    else if (output < -pid->output_limit) output = -pid->output_limit;

    return output;
}

