#include "stm32f4xx.h"                  // Device header
//PF9   LED0
//PF10  LED1
//PE13  LED2
//PE14  LED3
void LED_init(void)
{
 //时钟使能GPIOF的时钟
 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE);
 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);
	
	 //设置相应脚位功能
	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_9 |GPIO_Pin_10;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;//1HZ 1S  1KS  1MS 1MHZ 1US  
	//使配置功能脚位生效
	GPIO_Init(GPIOF,&GPIO_INSTRUCT);
	
	
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_13 |GPIO_Pin_14;
	//使配置功能脚位生效
	GPIO_Init(GPIOE,&GPIO_INSTRUCT);
   
	GPIO_SetBits(GPIOF,GPIO_Pin_9);
	GPIO_SetBits(GPIOF,GPIO_Pin_10);
	GPIO_SetBits(GPIOE,GPIO_Pin_13);
	GPIO_SetBits(GPIOE,GPIO_Pin_14);
	
}

void LED0_on(void)
{
 GPIO_ResetBits(GPIOF,GPIO_Pin_9);
} 

void LED0_off(void)
{
 GPIO_SetBits(GPIOF,GPIO_Pin_9);
} 

void LED1_on(void)
{
GPIO_ResetBits(GPIOF,GPIO_Pin_10);
}


void LED1_off(void)
{
GPIO_SetBits(GPIOF,GPIO_Pin_10);
}

void LED2_on(void)
{
GPIO_ResetBits(GPIOE,GPIO_Pin_13);
}

void LED2_off(void)
{
GPIO_SetBits(GPIOE,GPIO_Pin_13);
}

void LED3_on(void)
{
GPIO_ResetBits(GPIOE,GPIO_Pin_14);
}

void LED3_off(void)
{
GPIO_SetBits(GPIOE,GPIO_Pin_14);
}


