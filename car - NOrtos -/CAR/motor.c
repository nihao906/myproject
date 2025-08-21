#include "stm32f4xx.h"                  // Device header
#include "BITBAND.h"

//IN1 PB6
//IN2 PB7

void Motor_LeftStop(void);
void Motor_RightStop(void);

void Motor_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
	
		GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
		GPIO_Init(GPIOD, &GPIO_InitStruct);

    Motor_LeftStop(); 
		Motor_RightStop();
}

void Motor_LeftForward(void)
{
	  PBout(6) = 1;
		PBout(7) = 0; 
}

void Motor_LeftBackward(void)
{
	  PBout(6) = 0;
		PBout(7) = 1; 
}

void Motor_LeftStop(void)
{
    PBout(6) = 0;
		PBout(7) = 0; 
}

void Motor_RightForward(void)
{
	  PDout(0) = 0;
		PDout(1) = 1; 
}

void Motor_RightBackward(void)
{
	  PDout(0) = 1;
		PDout(1) = 0;
}

void Motor_RightStop(void)
{
    PDout(0) = 0;
		PDout(1) = 0; 
}

void PWM_setmotor(float PWM)
{
	if(PWM > 0)
	{
		Motor_LeftBackward();
		Motor_RightBackward();
	}
	else
	{
		Motor_LeftForward();
		Motor_RightForward();
		PWM = -PWM;
	}
	TIM_SetCompare1(TIM13, PWM);
	TIM_SetCompare1(TIM14, PWM);
}
