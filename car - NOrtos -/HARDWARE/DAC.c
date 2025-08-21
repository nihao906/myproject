#include "stm32f4xx.h"                  // Device header
//DAC PA4--DCMI_HREF(照相机的5号脚)

void DAC_init(void)
{
	//1、使能GPIOA时钟--DAC(PA4,PA5(ADC电位器))
 RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	
	//2、GPIO配置（模拟模拟）
	GPIO_InitTypeDef GPIO_INSTRUCT;
  GPIO_INSTRUCT.GPIO_Mode  = GPIO_Mode_AN;//因为用到ADC，因此配置模拟输入模式，模拟是唯一一个在ADC中用
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_4;
	
  //3、GPIO初始化
  GPIO_Init(GPIOA,&GPIO_INSTRUCT);
	
	//4、使能DAC时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC,ENABLE);
	
	//5、DAC的配置
	DAC_InitTypeDef DAC_INSTRUCT;
	DAC_INSTRUCT.DAC_OutputBuffer = DAC_OutputBuffer_Enable;//启用转换缓冲
	DAC_INSTRUCT.DAC_Trigger = DAC_Trigger_None;//不使用外部触发DAC
	DAC_INSTRUCT.DAC_WaveGeneration = DAC_WaveGeneration_None;//不产生输出波形
	
	//6、DAC的初始化
	DAC_Init(DAC_Channel_1,&DAC_INSTRUCT);
	
	//7、开启DAC
	DAC_Cmd(DAC_Channel_1,ENABLE);
}


