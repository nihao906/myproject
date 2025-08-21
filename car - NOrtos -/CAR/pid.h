#ifndef __PID__H_
#define __PID__H_

typedef struct{
	float p;
	float i;
	float d;
	float Error;
	float Last_Error;
	float Integral;
	float Differential;
	float Output;
	float Output_MaxLimit;
	float Output_MinLimit;
}PID_PositionTypedef;
extern PID_PositionTypedef PIDP;

typedef struct{
	float p;
	float i;
	float d;
	
	float Target_Speed;
	float Error;
	float Lowout;
	float Last_Error;
	float Integral;
	float Differential;
	float Output;
	float Output_MaxLimit;
	float Output_MinLimit;
}PID_SpeedTypedef;
extern PID_SpeedTypedef PIDS;

void PID_PositionInit(PID_PositionTypedef *pid, float p, float i, float d);
void PID_SpeedInit(PID_SpeedTypedef *pid, float Target_speed, float p, float i, float d);
float PID_PositionControl(PID_PositionTypedef *pid, float Target, float Current, float Gyro);
float PID_SpeedControl(PID_SpeedTypedef *pid, float Left, float Right);

#endif
