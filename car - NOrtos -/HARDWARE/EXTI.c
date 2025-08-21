#include "stm32f4xx.h"                  // Device header
//KEY0 PA0
//KEY1 PE2
//KEY2 PE3
//KEY3 PE4
//uint8_t flag = 0;
void EXTI_init(void)
{
 //1、使能相关的IO口时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE,ENABLE);
 //2、GPIO功能设置（模式、脚位）
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_IN;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_0;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
 //3、GPIO初始化
	GPIO_Init(GPIOA,&GPIO_INSTRUCT);
	
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 |GPIO_Pin_4;
	GPIO_Init(GPIOE,&GPIO_INSTRUCT);
	
	//4、使能SYSCGF的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG,ENABLE);
	
	//5、利用SYSCFG寄存器将GPIO与EXTI连接起来
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA,EXTI_PinSource0);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource2);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource3);
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE,EXTI_PinSource4);
	
	//6、EXTI外部中断配置（中断触发方式）
	EXTI_InitTypeDef  EXTI_INSTRUCT;
	EXTI_INSTRUCT.EXTI_Line = EXTI_Line0 | EXTI_Line2 | EXTI_Line3 | EXTI_Line4;
	EXTI_INSTRUCT.EXTI_LineCmd = ENABLE;
	EXTI_INSTRUCT.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_INSTRUCT.EXTI_Trigger = EXTI_Trigger_Falling;
	
	//7、EXTI初始化（让配置生效）
	EXTI_Init(&EXTI_INSTRUCT);
	
	//8、NVIC配置（嵌套中断向量控制器，中断优先级、中断通道...）
	NVIC_InitTypeDef NVIC_INSTRUCT;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_INSTRUCT.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 0X02;//抢占优先级 0-3
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority =  0X00;//响应优先级 0-3
	NVIC_Init(&NVIC_INSTRUCT);
	
  NVIC_INSTRUCT.NVIC_IRQChannel = EXTI2_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 0X02;//抢占优先级0-3
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority =  0X01;//响应优先级0-3
	NVIC_Init(&NVIC_INSTRUCT);
	
		
  NVIC_INSTRUCT.NVIC_IRQChannel = EXTI3_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 0X02;//抢占优先级0-3
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority =  0X02;//响应优先级0-3
	NVIC_Init(&NVIC_INSTRUCT);
	
		
  NVIC_INSTRUCT.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 0X02;//抢占优先级0-3
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority =  0X03;//响应优先级0-3
	//9、NVIC初始化（让配置生效）
	NVIC_Init(&NVIC_INSTRUCT);
}
	

 //10、中断服务程序函数
void EXTI0_IRQHandler(void)
{
	//1）获取中断的标志位，硬件置位
 if(EXTI_GetITStatus(EXTI_Line0) == SET)
 {
	 if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0)
	 //2）业务代码
   GPIO_ToggleBits(GPIOF,GPIO_Pin_9);
	 //3）清除中断标志位（本次中断处理完毕，可以进行下一次中断）
	 EXTI_ClearITPendingBit(EXTI_Line0);
 }
}

void EXTI2_IRQHandler(void)
{
	//1）获取中断的标志位，硬件置位
 if(EXTI_GetITStatus(EXTI_Line2) == SET)
 {
	 if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) == 0)
	 //2）业务代码
   GPIO_ToggleBits(GPIOF,GPIO_Pin_10);
	 //3）清除中断标志位（本次中断处理完毕，可以进行下一次中断）
	 EXTI_ClearITPendingBit(EXTI_Line2);
 }
}

void EXTI3_IRQHandler(void)
{
 if(EXTI_GetITStatus(EXTI_Line2) == SET)
 {
	 EXTI_ClearITPendingBit(EXTI_Line2);
 }
}

void EXTI4_IRQHandler(void)
{
 if(EXTI_GetITStatus(EXTI_Line2) == SET)
 {

	 EXTI_ClearITPendingBit(EXTI_Line2);
 }
}
