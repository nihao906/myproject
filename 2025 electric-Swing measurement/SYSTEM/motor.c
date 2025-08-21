#include "stm32f10x.h"
#include "stm32f10x_tim.h"

// PWM 频率 & 计数器最大值
#define PWM_PERIOD  99  // PWM 频率 = 72MHz / (72 * (PWM_PERIOD + 1)) = 1kHz

// 按键状态
//volatile uint8_t motor_dir = 0;  // 0: 正转, 1: 反转

//int main(void)
//{
//    // 初始化 GPIO、PWM 和按键
//    GPIO_Config();
//    PWM_Config();
//    KEY_Config();

//    while (1)
//    {
//        // 读取按键状态，切换方向
//        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0)  
//        {
//            for (int i = 0; i < 720000; i++);  // 消抖延时
//            if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0)
//            {
//                motor_dir = !motor_dir;  // 切换方向
//                Motor_SetDirection(motor_dir);
//            }
//            while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0);  // 等待按键松开
//        }

//        Motor_SetSpeed(500);  // 设置电机转速（0~999）
//    }
//}

// 配置 PWM
void PWM_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 开启时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // 配置 PA6 (TIM3_CH1) 为复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 配置 TIM3
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD;
    TIM_TimeBaseStructure.TIM_Prescaler = 35;  // 1MHz 计数时钟
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // 配置 PWM 输出模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 50;  // 初始占空比 50%
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_Cmd(TIM3, ENABLE);
}

// 配置按键（PB0）
//void KEY_Config(void)
//{
//    GPIO_InitTypeDef GPIO_InitStructure;
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  // 上拉输入
//    GPIO_Init(GPIOB, &GPIO_InitStructure);
//}

// 设置电机转速 (0~999)
void Motor_SetSpeed(uint16_t speed)
{
    if (speed > PWM_PERIOD) speed = PWM_PERIOD;
    TIM_SetCompare1(TIM3, speed);  // 设置 PWM 占空比
}

// 设置电机方向
void Motor_SetDirection(uint8_t direction)
{
    if (direction == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
    }
    else
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
    }
}
