#include "stm32f10x.h"
#include "lcd_init.h"
#include "lcd.h"

void control_number_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能C端口时钟
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;	 
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//速度50MHz
 	GPIO_Init(GPIOC, &GPIO_InitStructure);	  //初始化GPIOC
 	GPIO_SetBits(GPIOC,GPIO_Pin_8);
}

int8_t count_number(u8 number)
{
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)==0)
		{
			number += 5;
			LCD_ShowIntNum(40,104,number,2,0x0f00,0xffff,16);
			LCD_ShowIntNum(0,84,number,2,0x0f00,0xffff,16);
			while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)==0);
		}
		
		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)==0)
		{
			number -= 5;
			LCD_ShowIntNum(40,104,number,2,0x0f00,0xffff,16);
			LCD_ShowIntNum(0,84,number,2,0x0f00,0xffff,16);
			while(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)==0);
		}
		
		return number;
}
