#include "stm32f10x.h"  // 包含 STM32 标准外设库头文件
#include "stm32f10x_tim.h"

//// 编码器参数
//#define ENCODER_PPR 1000  // 每转脉冲数 (PPR)
//#define ENCODER_MULTIPLIER 4  // 4 倍频 (A 相和 B 相上升沿和下降沿都计数)
//#define SAMPLE_TIME_MS 100  // 采样间隔 100ms
//#define SAMPLE_TIME_S (SAMPLE_TIME_MS / 1000.0f)  // 采样间隔（秒）

// 初始化 TIM1 为编码器模式 (PA8: TIM1_CH1, PA9: TIM1_CH2)
void Encoder_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_ICInitTypeDef TIM_ICInitStructure;

    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);

    // 配置 PA8 和 PA9 为浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 TIM1 基本参数
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF;  // 最大计数值 (16 位定时器)
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // 配置 TIM1 为编码器模式
    TIM_EncoderInterfaceConfig(TIM1, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising);
    
    // 配置通道 1 和 2
    TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    TIM_ICInitStructure.TIM_ICFilter = 0x0;
    TIM_ICInit(TIM1, &TIM_ICInitStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
    TIM_ICInit(TIM1, &TIM_ICInitStructure);

    // 清除计数器
    TIM_SetCounter(TIM1, 0);
    TIM_Cmd(TIM1, ENABLE);
}

void Timer_Config(void) 
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    // 使能 TIM2 时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 配置 TIM2 定时器
//  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_TimeBaseStructure.TIM_Period = 999;   // 自动重载值，0.1秒
    TIM_TimeBaseStructure.TIM_Prescaler = 7199; // 预分频器，计数频率为 10KHz
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  // 时钟分割
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    // 使能定时器中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);
}

// 中断优先级配置
void NVIC_Config(void) 
{
    // 配置 NVIC 中断优先级
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  // 使用 TIM2 的中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  // 中断优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;  // 使能中断
    NVIC_Init(&NVIC_InitStructure);
}

void Timer_Stop(void)
{
    // 禁用定时器计数器
    TIM_Cmd(TIM2, DISABLE);

    // 禁用定时器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, DISABLE);  // 禁用 TIM2 时钟

    // 清除中断标志
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	
}

void Timer_Start(void)
{
    // 开启定时器计数器
    TIM_Cmd(TIM2, ENABLE);

    // 开启定时器时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // 开启 TIM2 时钟
	
}

int16_t Encoder_Get(void)
{
	int16_t Temp;
	Temp = TIM_GetCounter(TIM3);
	TIM_SetCounter(TIM3, 0);
	return Temp;
}
