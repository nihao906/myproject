#include "stm32f4xx.h"                  // Device header
#include "pid.h"

PID_PositionTypedef PIDP;
void PID_PositionInit(PID_PositionTypedef *pid, float p, float i, float d)
{
	pid->p = p;
	pid->i = i;
	pid->d = d;
	pid->Error = 0;
	pid->Last_Error = 0;
	pid->Integral = 0;
	pid->Differential = 0;
	pid->Output = 0;
	pid->Output_MaxLimit = 8000;
	pid->Output_MinLimit = -8000;
}

PID_SpeedTypedef PIDS;
void PID_SpeedInit(PID_SpeedTypedef *pid, float Target_speed, float p, float i, float d)
{
	pid->p = p;
	pid->i = i;
	pid->d = d;
	pid->Target_Speed = Target_speed;
	pid->Error = 0;
	pid->Lowout = 0;
	pid->Last_Error = 0;
	pid->Integral = 0;
	pid->Differential = 0;
	pid->Output = 0;
	pid->Output_MaxLimit = 8000;
	pid->Output_MinLimit = -8000;
}

float PID_PositionControl(PID_PositionTypedef *pid, float Target, float Current, float Gyro)
{
	pid->Error = Target - Current;
	pid->Integral += pid->Error;
	pid->Differential = pid->Error - pid->Last_Error;
	//pid->Output = pid->Error * pid->p + pid->Integral * pid->i + pid->Differential * pid->d;
	pid->Output = pid->Error * pid->p + pid->Integral * pid->i + Gyro * pid->d;
	
	if(pid->Output > pid->Output_MaxLimit)
	{
		pid->Output = pid->Output_MaxLimit;
	}
	else if(pid->Output < pid->Output_MinLimit)
	{
		pid->Output = pid->Output_MinLimit;
	}
	
	pid->Last_Error = pid->Error;
	
	return pid->Output;
}

float PID_SpeedControl(PID_SpeedTypedef *pid, float Left, float Right)
{
	pid->Error = (Left+Right) - pid->Target_Speed;
//	pid->Error *= 0.3f;
//	pid->Last_Error *= 0.7f;
//	
//	pid->Lowout = pid->Error + pid->Last_Error;
//	pid->Last_Error = pid->Lowout;
//	
//	pid->Integral += pid->Lowout;
//	if(pid->Integral > 8000)//20000
//		pid->Integral = 8000;
//	else if(pid->Integral < -8000)
//		pid->Integral = -8000;
	
	pid->Error *= 0.15f;
	pid->Last_Error *= 0.85f;
	
	pid->Lowout = pid->Error + pid->Last_Error;
	pid->Last_Error = pid->Lowout;
	
	pid->Integral += pid->Lowout;
	if(pid->Integral > 10000)
		pid->Integral = 10000;
	else if(pid->Integral < -10000)
		pid->Integral = -10000;
	
	pid->Output = pid->Lowout * pid->p + pid->Integral * pid->i;
	
	if(pid->Output > pid->Output_MaxLimit)
	{
		pid->Output = pid->Output_MaxLimit;
	}
	else if(pid->Output < pid->Output_MinLimit)
	{
		pid->Output = pid->Output_MinLimit;
	}
	
	return pid->Output;
	
//  ÆÕÍ¨Ð´·¨
//	pid->Error = pid->Target_Speed - (Left + Right) / 2;
//	pid->Integral += pid->Error;
//	if(pid->Integral > 2)
//	{
//		pid->Integral = 2;
//	}
//	else if(pid->Integral < -2)
//	{
//		pid->Integral = -2;
//	}
//	pid->Differential = pid->Error - pid->Last_Error;
//	pid->Output = pid->Error * pid->p + pid->Integral * pid->i + pid->Differential * pid->d;
//	
//	pid->Last_Error = pid->Error;
	
//	return pid->Output;
}

