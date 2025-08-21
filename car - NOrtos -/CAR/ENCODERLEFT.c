#include "stm32f4xx.h"                  // Device header

//PA0  TIM2_CH1  Left
//PA1  TIM2_CH2  Left

void ENCODER_initleft(void) 
{		
	GPIO_InitTypeDef 	        GPIO_InitStruct;
	TIM_TimeBaseInitTypeDef	 	TIM_TimeBaseInitStructure;
	TIM_ICInitTypeDef 			  TIM_ICInitStructure;
	NVIC_InitTypeDef			    NVIC_InitStructure;
		
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);//开启GPIOA的时钟
	
	GPIO_InitStruct.GPIO_Pin  	= GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed	= GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_OType	= GPIO_OType_OD;
	GPIO_InitStruct.GPIO_PuPd	=  GPIO_PuPd_UP;
	
	GPIO_Init(GPIOA, &GPIO_InitStruct);                          
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource0,GPIO_AF_TIM2);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM2);
	
  //定时器设置-------------------------------------------------------------	
  TIM_TimeBaseInitStructure.TIM_Period = 1320;   				//重装载值 这是两相脉冲总数量
	TIM_TimeBaseInitStructure.TIM_Prescaler = 0x0; 				 	//预分频,在编码器模式下没有任何作用
	TIM_TimeBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up; 	//向上计数
	TIM_TimeBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1; 		//时钟分割
	
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);//初始化TIM2

    //编码器模式设置--------------------------------------------------------------			  		

	TIM_EncoderInterfaceConfig(TIM2,TIM_EncoderMode_TI12,TIM_ICPolarity_BothEdge, TIM_ICPolarity_BothEdge);//计数模式3
	TIM_ICStructInit(&TIM_ICInitStructure); 
  TIM_ICInitStructure.TIM_ICFilter = 10;  //滤波器值
  TIM_ICInit(TIM2, &TIM_ICInitStructure);
    //溢出中断设置--------------------------------------------------------------
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE); //允许TIM2溢出中断
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority		= 0x00; 
	NVIC_InitStructure.NVIC_IRQChannelCmd				= ENABLE;
	NVIC_Init(&NVIC_InitStructure);
  
   //Reset counter-----------------------------------------------
   TIM_SetCounter(TIM2,0); //TIM2->CNT=0
   TIM_Cmd(TIM2, ENABLE); 
}

int16_t Encoder_Getleft(void)
{
//	uint32_t Temp;
//	Temp = TIM_GetCounter(TIM2);
//	TIM_SetCounter(TIM2, 0);
//	return Temp;
	return (int16_t)TIM_GetCounter(TIM2);
}

//int16_t Encoder_Getleft(void)
//{
//    return (int16_t)TIM_GetCounter(TIM9); 
//}

void Encoder_Resetleft(void)
{
    TIM_SetCounter(TIM2, 0);
}

