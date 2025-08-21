#include "stm32f10x.h" 
#include <stdlib.h>
/**
 * @brief  初始化 USART1
 */
void USART1_Init(void)
{
    // 1. 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);

    // 2. 配置 PA9 (TX) 为复用推挽输出
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;  
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 3. 配置 PA10 (RX) 为浮空输入
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 4. 配置 USART1
    USART_InitTypeDef USART_InitStruct;
    USART_InitStruct.USART_BaudRate = 9600;                         // 波特率
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;         // 8位数据
    USART_InitStruct.USART_StopBits = USART_StopBits_1;              // 1位停止位
    USART_InitStruct.USART_Parity = USART_Parity_No;                 // 无奇偶校验
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;     // 使能发送和接收
    USART_Init(USART1, &USART_InitStruct);

	// 4. 开启接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    // 5. 使能 USART1
    USART_Cmd(USART1, ENABLE);
	
	// 6. NVIC 配置中断优先级
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief  发送单个字符
 */
void USART1_SendChar(char ch)
{
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);  // 等待发送缓冲区空
    USART_SendData(USART1, ch);  // 发送数据
}

/**
 * @brief  发送字符串
 */
void USART1_SendString(char *str)
{
    while (*str)
    {
        USART1_SendChar(*str++);
    }
}

void my_itoa(int16_t num, char* str)
{
    char temp[8];
    int i = 0, j = 0;
    int isNegative = 0;

    if (num < 0)
    {
        isNegative = 1;
        num = -num;
    }

    // 数字转字符（倒序）
    do {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    } while (num);

    if (isNegative)
        str[j++] = '-';

    // 反转字符串
    while (i > 0)
        str[j++] = temp[--i];

    str[j] = '\0';
}


void USART1_SendInt(int16_t value)
{
    char buf[16];
    my_itoa(value, buf);
    USART1_SendString(buf);
}

