#include "stm32f4xx.h"                  // Device header
//PF8
void BEEP_init(void)
{
 //时钟使能GPIOF的时钟
 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
	
  //设置相应脚位功能
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_8;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	//使配置功能脚位生效
	GPIO_Init(GPIOF,&GPIO_INSTRUCT);
}

void BEEP_on(void)
{
	GPIO_SetBits(GPIOF,GPIO_Pin_8);
}

void BEEP_off(void)
{
  GPIO_ResetBits(GPIOF,GPIO_Pin_8);
}
