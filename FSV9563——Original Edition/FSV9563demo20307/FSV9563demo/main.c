#include <stdio.h>
#include "stm32f10x.h"
#include "FSV9563.h"
#include "stm32f10x_it.h"
u8 Status_INT=0;
u8 Tx_Hander = 0;
u8 Tx_Reg = 0;
u16 TestChip;
void System_Init(void);
int main()
{

	u8 chipIndex;
	u8 bValueI,bValueQ;
	
  System_Init();
	delay_ms(20);
	FSV9563_Init();
	printf("reset!!!!\n");

//	uint16_t reload = 13560;
//	FSV9563_WriteReg(rRegT3ReloadHi,  (reload >> 8) & 0xFF);
//	FSV9563_WriteReg(rRegT3ReloadLo,  reload & 0xFF);

//	FSV9563_WriteReg(rRegT3CounterValHi, (reload >> 8) & 0xFF);
//	FSV9563_WriteReg(rRegT3CounterValLo, reload & 0xFF);

//	FSV9563_WriteReg(rRegT3Control, 0x83); 

//	FSV9563_WriteReg(rRegIRQ1En, 0x48);
	
	while(1)
	{ 
		//FSV9563_TypeA();
	  //FSV9563_TypeB();
		//FSV9563_Felica();
		FSV9563_ISO15693();
//		if(Status_INT)
//		{
//			LED_1;
//		}
//		else
//		
//			LED_0;
//		}
		delay_ms(200);

	}
}

//void EXTI15_10_IRQHandler(void)
//{
//    if (EXTI_GetITStatus(EXTI_Line11) != RESET)
//    {
//        
//        LED_1;
//        // 清除中断标志位
//        EXTI_ClearITPendingBit(EXTI_Line11);
//    }
//}


int fputc(int ch, FILE *f)
{
	USART_SendData(USART1, (uint8_t) ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	return ch;
}

int fgetc(FILE *f)
{
  while(USART_GetFlagStatus(USART1, USART_FLAG_RXNE==RESET));
	return (int)USART_ReceiveData(USART1);
}

#if 0
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
	/* Whatever you require here. If the only file you are using is */ 
	/* standard output using printf() for debugging, no file handling */ 
	/* is required. */ 
}; 
/* FILE is typedef’ d in stdio.h. */ 
FILE __stdout;     
FILE __stdin; 
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
	USART1->DR = (u8) ch;      
	return ch;
}
#endif 

void System_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div2);
	RCC_PCLK2Config(RCC_HCLK_Div1); 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO | RCC_APB2Periph_ADC1 , ENABLE);
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2,  ENABLE );
	
	////////////////////////////////////////////////////////////////////////////////UART1
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitStructure.USART_BaudRate = 115200;  		
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART1, &USART_InitStructure);
	//USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);			//Enable UART1 receive interrupt
	USART_Cmd(USART1, ENABLE);
	USART_ClearFlag(USART1, USART_FLAG_TC); 
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_5|GPIO_Pin_4|GPIO_Pin_3;				//flag pb10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	FLAG_0;
	FLAG_1;	
	GPIO_ResetBits(GPIOB,GPIO_Pin_4);
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);
	
	
	GPIO_InitStructure.GPIO_Pin = LED;				//led pb3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	LED_0;
	delay_init(72);
}
