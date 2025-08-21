#ifndef _usart_H
#define _usart_H
 
#include "stm32f10x.h"
#include "stdio.h"
/*	
使用文件注意事项
	通常情况下：		默认  数据位：8位  停止位：1位  效验位：0
		bound 通信波特率 设置为   9600  或者   115200 
	例：
		USART1_Init(9600);
		USART1_Init(115200);
	
	发送数据使用：
		USART1_Send_Str("hello\n");		// 串口发送 hello
*/


#define USART1_REC_LEN 50		// 接收字符串长度定义
extern char USART1_RX_BUF[USART1_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
extern u16 USART1_RX_STA;       //接收状态标记	



void USART1_Init(u32 bound);	// 初始化

void Usart1_Send_Byte(u8 data);//值传递
void USART1_Send_Str(char *p);//址传递
int fputc(int ch, FILE *f);

#endif
