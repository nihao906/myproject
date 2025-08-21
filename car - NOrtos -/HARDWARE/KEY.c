#include "stm32f4xx.h"                  // Device header
#include "KEY.h"
//KEY0 PA0
//KEY1 PE2
//KEY2 PE3
//KEY3 PE4

void KEY_init(void)
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE,ENABLE);

	GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = GPIO_Mode_IN;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_0;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;

	GPIO_Init(GPIOA,&GPIO_INSTRUCT);
	
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_2| GPIO_Pin_3 |GPIO_Pin_4;
	GPIO_Init(GPIOE,&GPIO_INSTRUCT);
}

uint8_t GETKEY_flag(void)
{
  if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) == 0)//PA0
	{
	 return 1;
	}
   else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2) == 0)//PE2
	{
	 return 2;
	}
   else	if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3) == 0)//PE3
	{
	 return 3;
	}
	 else if(GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4) == 0)//PE4
	{
		return 4;
	}
	else 
	{
    return 0;
	}
}
