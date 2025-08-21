#include "stm32f4xx.h"                  // Device header
#include "DELAY.h"
void DELAY_ms(uint32_t xms)
{
	while(xms--)
	{
		SysTick->CTRL = 0; 
		SysTick->LOAD = SystemCoreClock /1000; 
		SysTick->VAL = 0; 
		SysTick->CTRL = 1<<0 | 1<<2; 
		while ((SysTick->CTRL &(1<<16))==0);
		SysTick->CTRL = 0; 
	}
}

void DElAY_us(uint32_t xus)
{
	while(xus--)
	{
		SysTick->CTRL = 0; 
		SysTick->LOAD = SystemCoreClock /1000/1000; 
		SysTick->VAL = 0; 
		SysTick->CTRL = 1<<0 | 1<<2; 
		while ((SysTick->CTRL &(1<<16))==0);
		SysTick->CTRL = 0; 
	}
}
