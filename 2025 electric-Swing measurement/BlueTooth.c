#include "stm32f10x.h" 
#include "uart.h"

void USART1_BTSendChar(char c)
{
    while (!(USART1->SR & USART_SR_TXE));
    USART1->DR = c;
}

void USART1_BTSendString(const char* str)
{
    while (*str)
    {
        USART1_SendChar(*str++);
    }
}

