#include "stm32f10x.h"

//GPIO初始化函数
void GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 开启 GPIOB 和 GPIOC 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);

    // 配置 GPIOB 引脚为推挽输出，用于控制数码管的段（A-G）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6;  // A-G 段
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置 GPIOC 引脚为推挽输出，用于控制数码管的位选（1-8）
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0; // 位选
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// 配置 PA0 为外部中断输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;               // 配置 PA0
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;    // 设置为浮空输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        // 设置输入速度
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}


// 配置 GPIO
void GPIO_Motor_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    // 方向控制引脚 PA4, PA5
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 按键 PB0
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

