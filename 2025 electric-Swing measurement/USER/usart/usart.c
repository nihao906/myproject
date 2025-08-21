#include "usart.h"

char USART1_RX_BUF[USART1_REC_LEN];     	// 接收缓冲,最大USART_REC_LEN个字节.
u16 USART1_RX_STA=0;       					// 接收状态标记	
u16 USART1_RX_num=0;						// 接收计数偏移

void USART1_Init(u32 bound)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;			// 引脚配置结构体变量
	USART_InitTypeDef 	USART_InitStructure;		// 串口配置结构体变量
	NVIC_InitTypeDef 	NVIC_InitStructure;			// 中断配置结构体变量
	/* config USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);	 // 时钟
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	  //打开时钟
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;		//USART1 Tx (PA.09)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		//USART1 Rx (PA.10)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = bound;						// 波特率
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;		// 数据位
	USART_InitStructure.USART_StopBits = USART_StopBits_1;			// 停止位
	USART_InitStructure.USART_Parity = USART_Parity_No ;			// 效验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		// 流控
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	// 中断模式
	USART_Init(USART1, &USART_InitStructure); 						// 初始化
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);					// 允许中断
	USART_Cmd(USART1, ENABLE);										// 使能串口
	
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;				// 串口 1  中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;		// 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;				// 响应优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					// 中断使能
	NVIC_Init(&NVIC_InitStructure);									// 初始化中断向量表
}


void Usart1_Send_Byte(u8 data)//值传递
{	//发送一个字节函数
	while(USART_GetFlagStatus( USART1, USART_FLAG_TXE) == 0);		// 判断发送标志位
	//USART1->DR = data;
	USART_SendData(USART1,data);									// 发送 字符
}
void USART1_Send_Str(char *p)//址传递
{	//发送字符串函数
	while(*p)
	{	// 循环发送字符串中的每一个字符
		Usart1_Send_Byte(*p);	// 发送字符
		p++;					// 指针偏移
	}
}
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}


//

//void USART1_IRQHandler(void)     							// 串口 1 中断服务函数           
//{	
//	u32 i;													// 中间变量		
//	
//	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  	// 接收标志位
//	{
//		i=USART_ReceiveData(USART1);						// 读取一个字节
//		USART_SendData(USART1,i);							// 串口发送一个字节
////		while(USART_GetFlagStatus(USART1,USART_FLAG_TC) != SET);	// 等待发送结束
//		USART_ClearFlag(USART1,USART_FLAG_TC);				// 清空中断标志位
//	} 
//}
// i 是一个字节   表示 char 类型  
// 判断  i 保存的 一个字节 控制LED灯状态	比如 i=A  LED点亮   i=B LED熄灭

//   贞头	数据1 数据2	贞尾
//   0x01  	0x03  0x05  0x10
void USART1_IRQHandler(void)                						// 串口1中断服务程序 接收字符串
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  			// 接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		USART1_RX_BUF[USART1_RX_num++] = USART_ReceiveData(USART1);	// 读取接收到的数据		
		
		if(USART1_RX_BUF[USART1_RX_num-1] == '\n' || USART1_RX_num >= USART1_REC_LEN) 	//不是换行就继续接收
		{
			USART1_RX_STA = 1;										// 遇到换行表示接收到一整个字符串 中断标志位置 1
			USART1_RX_num = 0;										// 接收偏移重新计数
			//		while(USART_GetFlagStatus(USART1,USART_FLAG_TC) != SET);	// 等待发送结束
		}
		USART_ClearFlag(USART1,USART_FLAG_TC);						// 清空中断标志位
	}
}
