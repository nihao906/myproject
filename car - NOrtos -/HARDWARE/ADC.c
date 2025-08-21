#include "stm32f4xx.h"                  // Device header
//PA5接ADC电路
void ADCRES_init(void)
{
 //1、使能GPIOA时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	
	//2、GPIO功能配置（复用功能）
	GPIO_InitTypeDef GPIO_INSTRUCT;
  GPIO_INSTRUCT.GPIO_Mode  = GPIO_Mode_AN;//因为用到ADC，因此配置模拟输入模式，模拟是唯一一个在ADC中用
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_5;
	
	//3、GPIO初始化
  GPIO_Init(GPIOA,&GPIO_INSTRUCT);
	
	//4、使能ADC的时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	
	//5、ADC的通用配置
	ADC_CommonInitTypeDef ADC_COMMONINSTRUCT;
	ADC_COMMONINSTRUCT.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;//禁用DMA访问
	ADC_COMMONINSTRUCT.ADC_Mode = ADC_Mode_Independent;//只使用了一个ADC，因此选择独立模式
	ADC_COMMONINSTRUCT.ADC_Prescaler = ADC_Prescaler_Div2;//设置ADC频率，APB2/2= 42MHZ
	ADC_COMMONINSTRUCT.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//设置采样相位延时
	
	//6、ADC的通用配置初始化
	ADC_CommonInit(&ADC_COMMONINSTRUCT);
	
	//7、ADC的配置
	ADC_InitTypeDef ADC_INSTRUCT;
	ADC_INSTRUCT.ADC_ContinuousConvMode = ENABLE;
	ADC_INSTRUCT.ADC_DataAlign = ADC_DataAlign_Right;//数据右对齐
	ADC_INSTRUCT.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//禁用外部触发
	ADC_INSTRUCT.ADC_NbrOfConversion = 1;
	ADC_INSTRUCT.ADC_Resolution = ADC_Resolution_12b;//ADC转换精度是12BIT
	ADC_INSTRUCT.ADC_ScanConvMode = DISABLE;//禁用多个ADC对通道进行采样
	
	//8、ADC的配置初始化
	 ADC_Init(ADC1, &ADC_INSTRUCT);
	 
	//9、ADC转换通道选择
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 1, ADC_SampleTime_3Cycles);
	
	//10、开启ADC
	 ADC_Cmd(ADC1, ENABLE);
}
//定时器触发（外部触发）
//软件触发（手动触发）
uint16_t ADC_getresult(void)
{
  ADC_SoftwareStartConv(ADC1);
  while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) !=SET);
	return ADC_GetConversionValue(ADC1);
}

