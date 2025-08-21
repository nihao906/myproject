#include "stm32f4xx.h"                  // Device header
#include "BITBAND.h"
//基本定时器
//16位向上计数器
//16分频器
void TIMER6_init(void)
{
  //1、使能定时器6的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
	
	//2、定时器的功能设置（计数模式...）
	TIM_TimeBaseInitTypeDef TIM_TIMBASEINSTRUCT;
	TIM_TIMBASEINSTRUCT.TIM_ClockDivision = TIM_CKD_DIV1;//二级分频，407没有
	TIM_TIMBASEINSTRUCT.TIM_CounterMode = TIM_CounterMode_Up;//向上计数模式
	TIM_TIMBASEINSTRUCT.TIM_Period = 8400 -1;//0.1MS *10000 = 1000MS = 1S
	TIM_TIMBASEINSTRUCT.TIM_Prescaler = 10000 -1;//10KHZ = 0.1MS 1KHZ = 1MS 
	
	//3、定时器的初始化
	TIM_TimeBaseInit(TIM6,&TIM_TIMBASEINSTRUCT);
	
	//4、开启定时器的中断
	 TIM_ITConfig(TIM6,TIM_IT_Update, ENABLE);
	
	//5、配置NVIC
	NVIC_InitTypeDef NVIC_INSTRUCT;
	NVIC_INSTRUCT.NVIC_IRQChannel = TIM6_DAC_IRQn;//参考stm32f4xx.h
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 0X01;
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 0X03;
	
	//6、NVIC初始化
	NVIC_Init(&NVIC_INSTRUCT);
	
	//7、开启定时器  
	TIM_Cmd(TIM6,ENABLE);
}
//8、提供中断服务程序
void TIM6_DAC_IRQHandler(void)//定时1s
{
 if(TIM_GetITStatus(TIM6,TIM_IT_Update) == SET)
 {
 
	 TIM_ClearITPendingBit(TIM6,TIM_IT_Update);
 }
}
