#include "stm32f10x.h" // 根据你的芯片型号选择正确的头文件

// 定义全局变量
static uint32_t fac_us = 72; // 微秒计数因子
static uint32_t fac_ms = 72000; // 毫秒计数因子

/**
 * @brief 初始化延时函数（基于 SysTick）
 * @param SYSCLK 系统时钟频率（单位：MHz）
 */
void Delay_Init(uint8_t SYSCLK) {
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); // SysTick 时钟源选择为 HCLK/8
    fac_us = SYSCLK / 8;  // 1微秒计数因子
    fac_ms = fac_us * 1000; // 1毫秒计数因子
}

/**
 * @brief 微秒级延时
 * @param nus 延时长度（单位：微秒）
 */
void Delay_us(uint32_t nus) {
    uint32_t temp;
    SysTick->LOAD = nus * fac_us; // 设置重装载值
    SysTick->VAL = 0x00;          // 清空当前计数值
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // 开始倒计时
    do {
        temp = SysTick->CTRL;
    } while ((temp & SysTick_CTRL_COUNTFLAG_Msk) == 0); // 等待计数到0
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 关闭计时器
    SysTick->VAL = 0x00; // 清空计数器
}

/**
 * @brief 毫秒级延时
 * @param nms 延时长度（单位：毫秒）
 */
void Delay_ms(uint32_t nms) {
    uint32_t temp;
    SysTick->LOAD = nms * fac_ms; // 设置重装载值
    SysTick->VAL = 0x00;          // 清空当前计数值
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; // 开始倒计时
    do {
        temp = SysTick->CTRL;
    } while ((temp & SysTick_CTRL_COUNTFLAG_Msk) == 0); // 等待计数到0
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; // 关闭计时器
    SysTick->VAL = 0x00; // 清空计数器
}
