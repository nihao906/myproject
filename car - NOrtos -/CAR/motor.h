#ifndef __MOTOR__H_
#define __MOTOR__H_

void Motor_Init(void);

void Motor_LeftForward(void);
void Motor_LeftBackward(void);
void Motor_LeftStop(void);

void Motor_RightForward(void);
void Motor_RightBackward(void);
void Motor_RightStop(void);

void PWM_setmotor(float PWM);
#endif
