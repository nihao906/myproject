#ifndef __PROJECT_CONFIG_H__
#define __PROJECT_CONFIG_H__

//基于semtech官网驱动移植
//官网驱动网址 https://github.com/Lora-net/LoRaMac-node/tree/master/src/radio  下载日期 2021/2/3
#define SOFT_VERSION	"LLCC68 driver for stm32f103 V0.0.0"

//--------------------------------------------- 测试默认配置 ---------------------------------------------
#define LORA_FRE									868500000	// 收发频率
#define LORA_TX_OUTPUT_POWER                        20        // 测试默认使用的发射功率，126x发射功率0~22dbm，127x发射功率2~20dbm
#define LORA_BANDWIDTH                              1         // [0: 125 kHz,	测试默认使用的带宽，LLCC68：[0: 125 kHz,1: 250 kHz,2: 500 kHz,3: Reserved]
#define LORA_SPREADING_FACTOR                       9         // 测试默认使用的扩频因子范围7~12
#define LORA_CODINGRATE                             1         // 测试默认使用的纠错编码率[1: 4/5,2: 4/6,3: 4/7,4: 4/8]
#define LORA_PREAMBLE_LENGTH                        8         // 前导码长度
#define LORA_LLCC68_SYMBOL_TIMEOUT                  0         // Symbols(LLCC68用到的是0,127x用到的是5)
#define LORA_FIX_LENGTH_PAYLOAD_ON                  false			// 是否为固定长度包(暂时只是LLCC68用到了)
#define LORA_IQ_INVERSION_ON                        false			// 这个应该是设置是否翻转中断电平的(暂时只是LLCC68用到了)
#define LORA_RX_TIMEOUT_VALUE                       5000

/*!
 * Board MCU pins definitions
 */
//SPI
#define RADIO_NSS_PIN       GPIO_Pin_4
#define RADIO_NSS_PORT      GPIOA
#define RADIO_MOSI_PIN      GPIO_Pin_7
#define RADIO_MOSI_PORT     GPIOA
#define RADIO_MISO_PIN      GPIO_Pin_6
#define RADIO_MISO_PORT     GPIOA
#define RADIO_SCK_PIN       GPIO_Pin_5
#define RADIO_SCK_PORT      GPIOA
//RST复位脚
#define RADIO_nRESET_PIN    GPIO_Pin_1
#define RADIO_nRESET_PORT   GPIOB
//DIO1 引脚 中断
#define RADIO_DIO1_PIN      GPIO_Pin_11
#define RADIO_DIO1_PORT     GPIOB
//BUSY 引脚
#define RADIO_DIO4_BUSY_PIN      GPIO_Pin_0
#define RADIO_DIO4_BUSY_PORT     GPIOA


//下面这几个引脚没用用到，设置为浮空输入模式
//TXEN
#define RADIO_DIO0_TXEN_PIN      GPIO_Pin_10
#define RADIO_DIO0_TXEN_PORT     GPIOB
//DIO2
#define RADIO_DIO2_PIN      GPIO_Pin_8
#define RADIO_DIO2_PORT     GPIOB
//DIO3
#define RADIO_DIO3_PIN      GPIO_Pin_9
#define RADIO_DIO3_PORT     GPIOB
//RXEN
#define RADIO_DIO5_RXEN_PIN      GPIO_Pin_1
#define RADIO_DIO5_RXEN_PORT     GPIOA

#endif // __PROJECT_CONFIG_H__


/*EC600U*/


#define     SPI_1_CLK       GPIO_9
#define     SPI_1_nCS       GPIO_10
#define     SPI_1_MOSI      GPIO_11
#define     SPI_1_MISO      GPIO_12

#define     SPI_2_CLK       GPIO_0
#define     SPI_2_nCS       GPIO_1
#define     SPI_2_MOSI      GPIO_2
#define     SPI_2_MISO      GPIO_3


#define     LORA_nCS    SPI_1_nCS
#define     LORA_MOSI   SPI_1_MOSI
#define     LORA_MISO   SPI_1_MISO
#define     LORA_SCLK   SPI_1_CLK
#define     LORA_BUSY   SPI_2_CLK
#define     LORA_nINT   SPI_2_MISO
#define     LORA_RST    SPI_2_MOSI

#define     LORA_nCS_PIN    4
#define     LORA_MOSI_PIN   3
#define     LORA_MISO_PIN   2
#define     LORA_SCLK_PIN   1
#define     LORA_BUSY_PIN   61
#define     LORA_nINT_PIN   60
#define     LORA_RST_PIN    59 