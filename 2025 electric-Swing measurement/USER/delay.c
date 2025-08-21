#include "stm32f10x.h"

void delay_us(u32 nus)  
{
	u32 temp;
	SysTick->LOAD = 9*nus;  //LOAD寄存器是24位，倒数最大为2^24，若为9000*nus,则nus<(2^24/9000),否则出错;
	SysTick->VAL = 0;
	SysTick->CTRL |= 1;
	do
	{
		temp = SysTick->CTRL;
	}
	while(temp&0x01 && !(temp&(1<<16)));
	SysTick->CTRL |= 0;
	SysTick->VAL = 0;
	
}
void delay_ms(u32 mus)
{
	while(mus--)
	{
		delay_us(1000);
	}
}
