#include "stm32f4xx.h"                  // Device header
#include "BITBAND.h"
//基本定时器
//16位向上计数器
//16分频器

//int16_t encoder_speed = (int16_t)(TIMx->CNT - last_cntleft);
//last_cntleft = TIMx->CNT;


void TIMER6_init(void)
{
  //1、使能定时器6的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
	
	//2、定时器的功能设置（计数模式...）
	TIM_TimeBaseInitTypeDef TIM_TIMBASEINSTRUCT;
	TIM_TIMBASEINSTRUCT.TIM_ClockDivision = TIM_CKD_DIV1;//二级分频，407没有
	TIM_TIMBASEINSTRUCT.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
	TIM_TIMBASEINSTRUCT.TIM_Period = 840 -1;			//10ms
	TIM_TIMBASEINSTRUCT.TIM_Prescaler = 1000 -1;//
	
	//3、定时器的初始化
	TIM_TimeBaseInit(TIM6,&TIM_TIMBASEINSTRUCT);
	
	//4、开启定时器的中断
	TIM_ITConfig(TIM6,TIM_IT_Update, ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	//5、配置NVIC
	NVIC_InitTypeDef NVIC_INSTRUCT;
	NVIC_INSTRUCT.NVIC_IRQChannel = TIM6_DAC_IRQn;//参考stm32f4xx.h
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 0X04;
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 0X00;
	
	//6、NVIC初始化
	NVIC_Init(&NVIC_INSTRUCT);
	
	//7、开启定时器  
	TIM_Cmd(TIM6,ENABLE);
}

