#include "stm32f4xx.h"                  // Device header

void WWDG_init(void)
{
 //1、使能APB1的WWGD外设时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG,ENABLE);
 
	//2、设置WWDG预分频数值
	WWDG_SetPrescaler(WWDG_Prescaler_8);//42MHZ/4096/8=780us
	
	//3、设置WWGD窗口上限
	WWDG_SetWindowValue(80);
	
	//4、开启WWDG
	WWDG_Enable(127);
	
}


