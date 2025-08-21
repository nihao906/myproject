#include "stm32f4xx.h"                  // Device header
#include "motor.h"
#include "DELAY.h"
#include "oled.h"
#include "jy61p.h"
#include "FTOC.h"
#include "USART1.h"
#include <stdio.h>
#include "PWMA.h"
#include "PWMB.h"
#include "pid.h"
#include "ENCODERLEFT.h"
#include "ENCODERRIGHT.h"
#include "FreeRTOS.h"
#include "task.h"
#include "TIMER.h"
#include "queue.h"
#include "semphr.h"

//INA PF8  Left   TIM13
//IN1 PB6  Left
//IN2 PB7	 Left

//INB PF9  Right  TIM14
//IN3 PD0  Right
//IN4 PD1  Right

//JY60 PA9 PA10  U1        01,00interrupt   直立环
//OLED PB8 PB9    

//PA0  TIM2_CH1  Left      02,00interrupt  位置环
//PA1  TIM2_CH2  Left

//PA6  TIM3_CH1  right     03,00interrupt 位置环
//PA7  TIM3_CH2  right   

//TIM6                     04,00interrupt   速度环

//上位机 PA2 PA3  U2        05,00interrupt

//BlueTooth PB10 PB11 U3   06,00interrupt

//平衡点 Pitch 3
int16_t circle_countleft = 0;   //记录电机正反转,位置环
int16_t circle_countright = 0;

int16_t last_circle_countleft = 0;
int16_t last_circle_countright = 0;

int16_t last_cntleft = 0;
int16_t last_cntright = 0;

int16_t cnt_speedleft = 0;
int16_t cnt_speedright = 0;

float PWM = 0,PWM_speed = 0,PWM_position = 0;
char PWM1[7];
float Target_angle = 3.4;


int main(void)
{
	
	OLED_Init();		
	OLED_Clear();
	Motor_Init();
	USART1_init();
	PWMA_init();
	PWMB_init();
	ENCODER_initleft();
	ENCODER_initright();
	TIMER6_init();
	
	//PID_PositionInit(&PIDP, 840, 0, 0);//840 -905
	PID_PositionInit(&PIDP, 504, 0, -543);//504,-543
	//PID_SpeedInit(&PIDS, 0, -20, -0.1, 0);//-14//-20
	PID_SpeedInit(&PIDS, 0, -25, -0.125, 0);//-15
				
	char Pitch1[7];
	OLED_ShowString(1, 2, (u8 *)"Pitch:", 12);
	char Pitch_gyro1[7];
	while(1)
	{
//				float_to_string(Pitch_gyro, Pitch_gyro1, 1);	
//  			OLED_ShowString(50, 2, (u8 *)Pitch_gyro1, 12);
//				float_to_string(Pitch, Pitch1, 1);	
//				OLED_ShowString(50, 4, (u8 *)Pitch1, 12);
//			OLED_ShowNum(40, 6, cnt_speedleft, 5, 12);
//     	OLED_ShowNum(80, 6, cnt_speedright, 5, 12);
//			float_to_string(PWM, PWM1, 1);
//			OLED_ShowString(1, 6, (u8 *)PWM1, 12);
//		
			PWM_speed = PID_SpeedControl(&PIDS , cnt_speedleft, cnt_speedright);
			PWM_position = PID_PositionControl(&PIDP, Target_angle,Pitch,Pitch_gyro);//接下来写pid位置环和速度环的连接，考虑再开一个task
		  PWM = PWM_position + PWM_speed;
			PWM_setmotor(PWM);
	}
	
}

//电机转动一圈产生中断
void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
		if((TIM2->CR1>>4 & 0x01)==0)     //DIR==0  通过寄存器TIMx_CR1第四位判断  0：电机正转
			circle_countleft++;
		else if((TIM2->CR1>>4 & 0x01)==1)//DIR==1  通过寄存器TIMx_CR1第四位判断  1：电机正转
			circle_countleft--;
	}
	TIM_ClearITPendingBit(TIM2,TIM_IT_Update); 

}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)==SET)
	{
		if((TIM3->CR1>>4 & 0x01)==0)     //DIR==0  通过寄存器TIMx_CR1第四位判断  0：电机正转
			circle_countright++;
		else if((TIM3->CR1>>4 & 0x01)==1)//DIR==1  通过寄存器TIMx_CR1第四位判断  1：电机正转
			circle_countright--;
	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update); 

}

void TIM6_DAC_IRQHandler(void)//定时10ms
{
 if(TIM_GetITStatus(TIM6,TIM_IT_Update) == SET)
 {
	 if(circle_countleft == last_circle_countleft)
	 {
			 cnt_speedleft = TIM_GetCounter(TIM2) - last_cntleft;
	 }
	 else if(circle_countleft > last_circle_countleft)
	 {
			 cnt_speedleft = 1320 - last_cntleft + TIM_GetCounter(TIM2) + 1320*(circle_countleft - last_circle_countleft);
	 }
	 else if(circle_countleft < last_circle_countleft)
	 {
			 cnt_speedleft = -1320 + TIM_GetCounter(TIM2) - last_cntleft - 1320*(last_circle_countleft - circle_countleft);
	 }
	 last_cntleft = TIM_GetCounter(TIM2);
	 last_circle_countleft = circle_countleft;
	 
	 if(circle_countright == last_circle_countright)
	 {
			 cnt_speedright = TIM_GetCounter(TIM3) - last_cntright;
	 }
	 else if(circle_countright > last_circle_countright)
	 {
			 cnt_speedright = 1320 - last_cntright + TIM_GetCounter(TIM3) + 1320*(circle_countright - last_circle_countright);
	 }
	 else if(circle_countright < last_circle_countright)
	 {
			 cnt_speedright =  -1320 + TIM_GetCounter(TIM3) - last_cntright - 1320*(last_circle_countright - circle_countright);
	 }
	 last_cntright = TIM_GetCounter(TIM3);
	 last_circle_countright = circle_countright;
	 
	 TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
 }
}


