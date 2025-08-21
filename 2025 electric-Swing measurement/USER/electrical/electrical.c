#include "stm32f10x.h"

void Reversible_Init(void)       //正反转输出初始化
{
		GPIO_InitTypeDef GPIO_InitStructure;
	    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;       // 推挽输出
		GPIO_Init(GPIOA, &GPIO_InitStructure);
}
