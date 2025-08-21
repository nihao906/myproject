#include "stm32f4xx.h"                  // Device header
//定时溢出是1s
//LSI 32KHZ
void IWDG_init(void)
{
 //1、开启看门狗时钟
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	
 //2、独立看门狗分频设置
	IWDG_SetPrescaler(IWDG_Prescaler_32);//32KHZ /32 = 1KHZ = 1MS
	
 //3、设置重装值
	IWDG_SetReload(1000);//0-4096
	
 //4、重载计数器（喂狗）
	IWDG_ReloadCounter();
	
 //5、使能看门狗
	IWDG_Enable();
}
