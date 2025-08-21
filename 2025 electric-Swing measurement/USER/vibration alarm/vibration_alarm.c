#include "stm32f10x.h"	/*导入单片机资源*/

GPIO_InitTypeDef GPIO_InitStructure;

void Buzzer_init(void)         //蜂鸣器
{
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	/*打开对应功能的电源*/
	
	/* Configure PD0 and PD2 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;			/*6号引脚*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/*切换速度*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	/*输出推挽*/	
	GPIO_Init(GPIOA, &GPIO_InitStructure);				/*配置a组功能*/
}

void LED_init(void)				//报警灯						
{
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	/*打开对应功能的电源*/
	
	/* Configure PD0 and PD2 in output pushpull mode */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;			/*8号引脚*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/*切换速度*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	/*输出推挽*/	
	GPIO_Init(GPIOA, &GPIO_InitStructure);				/*配置a组功能*/
}


void vibration_init(void)										//震动传感
{	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	/*打开对应功能的电源*/
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;			/*5号引脚*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/*切换速度*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;		/*下拉输入*/	
	GPIO_Init(GPIOA, &GPIO_InitStructure);				/*配置a组功能*/
}

void KeyA0_init(void)										//关闭安保
{	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	/*打开对应功能的电源*/
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;			/*0号引脚*/
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	/*切换速度*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;		/*下拉输入*/	
	GPIO_Init(GPIOA, &GPIO_InitStructure);				/*配置a组功能*/
}


