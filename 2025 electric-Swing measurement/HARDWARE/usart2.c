#include "usart2.h"               
#include "jy61p.h"

/**
 * @brief       串口2初始化函数
 * @param       无
 * @retval      无
 */
void usart2_Init(uint32_t bound)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;//TX
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//RX
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	 
	USART_InitStructure.USART_BaudRate = bound;//波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//停止位
	USART_InitStructure.USART_Parity = USART_Parity_No ;//校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//硬件流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//收发模式
	USART_Init(USART2, &USART_InitStructure); 
	
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
	USART_Cmd(USART2, ENABLE);//打开串口
	
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;//中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;//抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;//响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;//使能通道
	NVIC_Init(&NVIC_InitStructure);
}



/**
 * @brief       串口2接收中断服务函数 
 * @param       无
 * @retval      无
 */
void USART2_IRQHandler(void)
{
	uint8_t RxData;
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
	{
		RxData = USART_ReceiveData(USART2);	/*读取接收到的数据*/
		
		jy61p_ReceiveData(RxData);	/*调用数据包处理函数*/
		Ajy61p_ReceiveData(RxData);
		
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
	}
}
