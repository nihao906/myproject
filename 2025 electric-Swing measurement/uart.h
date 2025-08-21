#ifndef __UART_H
#define __UART_H

void USART1_Init(void);
void USART1_SendChar(char ch);
void USART1_SendString(char *str);
void USART1_SendInt(int16_t value);

#endif
