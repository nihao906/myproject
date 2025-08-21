#include "stm32f4xx.h"                  // Device header

//PA6  TIM3_CH1  right
//PA7  TIM3_CH2  right

void ENCODER_initright(void) 
{		
	GPIO_InitTypeDef 	        GPIO_InitStruct;
	TIM_TimeBaseInitTypeDef	 	TIM_TimeBaseInitStructure;
	TIM_ICInitTypeDef 			  TIM_ICInitStructure;
	NVIC_InitTypeDef			    NVIC_InitStructure;
		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//开启GPIOA的时钟
	
	GPIO_InitStruct.GPIO_Pin  	= GPIO_Pin_6|GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd	=  GPIO_PuPd_UP;
	
	GPIO_Init(GPIOA, &GPIO_InitStruct);                          
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_TIM3);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_TIM3);
	
  //定时器设置-------------------------------------------------------------	
  TIM_TimeBaseInitStructure.TIM_Period = 1320;   				//重装载值 这是两相脉冲总数量
	TIM_TimeBaseInitStructure.TIM_Prescaler = 0x0; 				 	//预分频,在编码器模式下没有任何作用
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 	//向上计数
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 		//时钟分割
	
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);//初始化TIM3

    //编码器模式设置--------------------------------------------------------------			  		

	TIM_EncoderInterfaceConfig(TIM3,TIM_EncoderMode_TI12,TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);//计数模式3
	
	TIM_ICStructInit(&TIM_ICInitStructure); 
  TIM_ICInitStructure.TIM_ICFilter = 10;  //滤波器值
  TIM_ICInit(TIM3, &TIM_ICInitStructure);
    //溢出中断设置--------------------------------------------------------------
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE); //允许TIM2溢出中断
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		= 0x00; 
	NVIC_InitStructure.NVIC_IRQChannelCmd				= ENABLE;
	NVIC_Init(&NVIC_InitStructure);
  
   //Reset counter-----------------------------------------------
   TIM_SetCounter(TIM3,0); //TIM3->CNT=0
   TIM_Cmd(TIM3, ENABLE); 
}

int16_t Encoder_Getright(void)
{
//	uint32_t Temp;
//	Temp = TIM_GetCounter(TIM2);
//	TIM_SetCounter(TIM2, 0);
//	return Temp;
	return (int16_t)TIM_GetCounter(TIM3);
}
