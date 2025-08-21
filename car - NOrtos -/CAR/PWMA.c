#include "stm32f4xx.h"                  // Device header

//PF8/TIM13_CH1
//1S = 1HZ 1MS = 1KHZ 1US = 1MHZ 
//PWM资源紧缺情况下请定时器中断手动反转GPIO，模拟PWD
void PWMA_init(void)
{
 //1、使能GPIO时钟 PF8
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	
	//2、GPIO功能配置（复用功能）
	GPIO_InitTypeDef GPIO_INSTRUCT;
  GPIO_INSTRUCT.GPIO_Mode  = GPIO_Mode_AF;//不用GPIO功能，用其他功能
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_8;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	
	//3、GPIO初始化
  GPIO_Init(GPIOF,&GPIO_INSTRUCT);
	
	//4、将GPIO复用为定时器  
	GPIO_PinAFConfig(GPIOF,GPIO_PinSource8,GPIO_AF_TIM13);//将PF8复用为TIM13
	
	//5、使能定时器时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13,ENABLE);
	
	//6、定时器的基础功能配置（计数模式）
	TIM_TimeBaseInitTypeDef  TIM_TIMINSTRUCT;
	TIM_TIMINSTRUCT.TIM_ClockDivision = TIM_CKD_DIV1;//没有二级分频
	TIM_TIMINSTRUCT.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
	TIM_TIMINSTRUCT.TIM_Period = 8000 - 1;//
	TIM_TIMINSTRUCT.TIM_Prescaler = 1 - 1;//10.5kHZ

	//7、定时器初始化
	TIM_TimeBaseInit(TIM13,&TIM_TIMINSTRUCT);
	
	//8、PWM功能配置
	TIM_OCInitTypeDef TIM_OCINITSTRUCT;
	TIM_OCINITSTRUCT.TIM_OCMode = TIM_OCMode_PWM1;//PWM1 = CNT<CCR 为有效，否则为无效
	TIM_OCINITSTRUCT.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCINITSTRUCT.TIM_Pulse = 100;//CCR有效值占比空
	TIM_OCINITSTRUCT.TIM_OCPolarity = TIM_OCPolarity_High;

  //9、PWM初始化
	TIM_OC1Init(TIM13,&TIM_OCINITSTRUCT);
  TIM_OC1PreloadConfig(TIM13, TIM_OCPreload_Enable);//使能定时器通道1预加载，主要是用来产生连续的PWM
  TIM_ARRPreloadConfig(TIM13, ENABLE);//使能ARR寄存器预加载

  //10、定时器输出
  TIM_Cmd(TIM13, ENABLE);
}


