#include "stm32f4xx.h"                  // Device header
#include "BITBAND.h"
#include <stdio.h>
//USART3
//PB10  TX
//PB11  RX
//STM32F103C8T6
//STM32F407ZET6	
void USART3_init(void)
{
	//1、对应使能GPIO时钟
 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);
	
	//2、设脚位功能
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_AF;//复用模式
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;//推挽输出
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_10 |GPIO_Pin_11;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;//浮空
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;//输出速率

	//3、初始化脚位设置
	GPIO_Init(GPIOB,&GPIO_INSTRUCT);
	
	//4、使能串口的时钟
	 RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);
	
	//5、复用GPIO为USART
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3);
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);
	
	//6、串口参数配置
	USART_InitTypeDef USART_INSTRUCT;
	USART_INSTRUCT.USART_BaudRate = 9600;//波特率
	USART_INSTRUCT.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//不使用硬件流控
	USART_INSTRUCT.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;//收发模式
	USART_INSTRUCT.USART_Parity = USART_Parity_No;//不校验
	USART_INSTRUCT.USART_StopBits = USART_StopBits_1;//1位停止位
	USART_INSTRUCT.USART_WordLength = USART_WordLength_8b;//8bit
	
	//7、串口初始化
	USART_Init(USART3,&USART_INSTRUCT);
	
	//8、开启串口接收的中断
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	//9、NVIC配置
	NVIC_InitTypeDef NVIC_INSTRUCT;
	NVIC_INSTRUCT.NVIC_IRQChannel = USART3_IRQn;
	NVIC_INSTRUCT.NVIC_IRQChannelCmd = ENABLE;
	NVIC_INSTRUCT.NVIC_IRQChannelPreemptionPriority = 0X06;
	NVIC_INSTRUCT.NVIC_IRQChannelSubPriority = 0X00;
	
	//10、NVIC初始化
	NVIC_Init(&NVIC_INSTRUCT);
	
	//11、启动串口接收
	USART_Cmd(USART3,ENABLE);
}

//12、提供串口的中断服务程序
void USART3_IRQHandler(void)
{
  if(USART_GetITStatus(USART3,USART_IT_RXNE) ==SET)
	{
	  uint8_t data =  USART_ReceiveData(USART3);
		if(data == 0x55)
		{
			USART_SendData(USART3,0X05);
		}
		if(data == 0xff)
		{
			USART_SendData(USART3,0XFF);
		}
		USART_ClearITPendingBit(USART3,USART_IT_RXNE);
	}
}

void USART3_senddata(uint8_t data)
{
	USART_SendData(USART1,data);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) == RESET);
}

void USART3_sendstring(char*string)
{
  uint16_t i;
	for(i = 0;i !='\0';i++)
	{
	  USART3_senddata(string[i]);
	}
}
