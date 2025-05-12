/*!
 * \file      sx1262mbxcas-board.c
 *
 * \brief     Target board SX1262MBXCAS shield driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <stdlib.h>
#include "project_config.h"

#include "radio.h"
#include "llcc68_board.h"
#include "ql_api_spi.h"
#include "ql_gpio.h"

#include "ql_api_osi.h"
//#include "stdio.h"

#include "ql_log.h"
#define LOGI(msg, ...)  QL_LOG(QL_LOG_LEVEL_INFO, "llcc68_ec600u_board", msg, ##__VA_ARGS__)

typedef struct __softTimer_s{
	uint8_t isRun;		//0停止，1运行
	uint32_t delayMs;	//定时器需要多长时间后唤醒
	uint32_t startMs;	//定时器开始时间
}SoftTimer_t;

static DioIrqHandler *dio1IrqCallback=NULL;	//记录DIO1的回调函数句柄
static uint32_t timer_count=0;
static SoftTimer_t txTimerHandle,rxTimerHandle;
ql_errcode_gpio err = 1;
ql_errcode_spi_e spi_err = 0;
/*初始化spi*/
uint8_t Spi1Init(void)
{
	LOGI("enter Spi1Init\n");
	// GPIO_InitTypeDef  GPIO_InitStructure;
	// SPI_InitTypeDef  SPI_InitStructure;

	// //初始化SPI
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,ENABLE);//SPI时钟使能
	// //初始化MISO,MOSI,SCK为推挽外设模式
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	    //使能指定端口时钟
	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO口速度为50MHz，这里不用传参，直接写死用最大速度50MHZ
	// GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;	//片上外设推挽模式
	// GPIO_InitStructure.GPIO_Pin = RADIO_SCK_PIN|RADIO_MISO_PIN|RADIO_MOSI_PIN;
	// GPIO_Init(RADIO_SCK_PORT, &GPIO_InitStructure);	//初始化GPIO

	// //配置SPI
	// SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	// SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	// SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	// SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		//设置时钟极性(串行同步时钟的空闲状态为高电平还是低电平)
	// SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	//设置相位(串行同步时钟的第几个跳变沿（上升或下降）数据被采样)
	// SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:设置为软件控制(SSI控制)
	// SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;		//定义波特率预分频的值:波特率预分频值为256(256是最低,可以设置完成之后调整速度,如果速度过快导致通信失败再调小)
	// SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	// SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式(CRC校验相关)
	// SPI_Init(SPI1,&SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

	// SPI_Cmd(SPI1,ENABLE); //使能SPI外设
	
	// //初始化NSS脚为输出高电平 RADIO_NSS_PIN		RADIO_NSS_PORT
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	    //使能指定端口时钟
	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO口速度为50MHz，这里不用传参，直接写死用最大速度50MHZ
	// GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;	//推挽输出
	// GPIO_InitStructure.GPIO_Pin = RADIO_NSS_PIN;
	// GPIO_Init(RADIO_NSS_PORT, &GPIO_InitStructure);	//初始化GPIO
	// //GPIO_ResetBits(gpioObj->portIndex,gpioObj->pinIndex);	//输出低电平
	// GPIO_SetBits(RADIO_NSS_PORT,RADIO_NSS_PIN);	//输出高电平
	
	// return 0;
	ql_spi_config_s lora_spi_config = {0};
    lora_spi_config.input_mode =        QL_SPI_INPUT_TRUE;
    lora_spi_config.port =              QL_SPI_PORT1;
    lora_spi_config.framesize =         8;
    lora_spi_config.spiclk =            QL_SPI_CLK_6_25MHZ;
    lora_spi_config.cs_polarity0 =      QL_SPI_CS_ACTIVE_LOW;
    lora_spi_config.cs_polarity1 =      QL_SPI_CS_ACTIVE_LOW;
    lora_spi_config.cpol =              QL_SPI_CPOL_LOW;
    lora_spi_config.cpha =              QL_SPI_CPHA_1Edge;
    lora_spi_config.input_sel =         QL_SPI_DI_1;//?
    lora_spi_config.transmode =         QL_SPI_DMA_IRQ;
    lora_spi_config.cs =                QL_SPI_CS0;
    
    lora_spi_config.clk_delay =         QL_SPI_CLK_DELAY_0;
    lora_spi_config.release_flag =      QL_SPI_NOT_RELEASE;

    ql_spi_init_ext(lora_spi_config);
    ql_spi_cs_auto(QL_SPI_PORT1);
    ql_spi_request_sys_clk(QL_SPI_PORT1);

	return 1;
}


/* 初始化LLCC68需要用到的GPIO初始化，将 BUSY 引脚设置为输入模式
 */
int LLCC68IoInit( void )
{
	LOGI("enter LLCC68IoInit\n");
	//uint8_t u8_ret=255;
	
	//GPIO_InitTypeDef  GPIO_InitStructure;

	//初始化BUSY为上拉输入模式
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	    //使能指定端口时钟
	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO口速度为50MHz，这里不用传参，直接写死用最大速度50MHZ
	// GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;//上拉输入
	// GPIO_InitStructure.GPIO_Pin = RADIO_DIO4_BUSY_PIN;
	// GPIO_Init(RADIO_DIO4_BUSY_PORT, &GPIO_InitStructure);	//初始化GPIO

	//下面这几个引脚没用用到，设置为浮空输入模式
	// GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;//浮空输入
	// GPIO_InitStructure.GPIO_Pin = RADIO_DIO0_TXEN_PIN;
	// GPIO_Init(RADIO_DIO0_TXEN_PORT, &GPIO_InitStructure);	//初始化GPIO
	// GPIO_InitStructure.GPIO_Pin = RADIO_DIO2_PIN;
	// GPIO_Init(RADIO_DIO2_PORT, &GPIO_InitStructure);	//初始化GPIO
	// GPIO_InitStructure.GPIO_Pin = RADIO_DIO3_PIN;
	// GPIO_Init(RADIO_DIO3_PORT, &GPIO_InitStructure);	//初始化GPIO
	// GPIO_InitStructure.GPIO_Pin = RADIO_DIO5_RXEN_PIN;
	// GPIO_Init(RADIO_DIO5_RXEN_PORT, &GPIO_InitStructure);	//初始化GPIO

	// err = ql_gpio_init(LORA_BUSY,GPIO_INPUT,PULL_UP,LVL_LOW);
	// if(err != 0){
	// 	return -1;
	// 	LOGI("ql_gpio_init LORA_BUSY fail\n");
	// }
	
	/*初始化spi1总线引脚*/
	ql_pin_set_func(1,0x01);//clk
    ql_pin_set_func(2,0x01);//miso di_1
    ql_pin_set_func(3,0x01);//mosi dio_0
    ql_pin_set_func(4,0x01); //ncs_pin

	/*将miso的io口设置为输入,其余为输出*/
	
	ql_gpio_set_direction(GPIO_12,GPIO_INPUT);
	
	ql_gpio_set_direction(GPIO_11,GPIO_OUTPUT);
	ql_gpio_set_direction(GPIO_10,GPIO_OUTPUT);
	ql_gpio_set_level(GPIO_10,LVL_HIGH);
	ql_gpio_set_direction(GPIO_9,GPIO_OUTPUT);


    // /*将busy，int，rst引脚设置为gpio功能*/
    // ql_pin_set_gpio(59);//rst_pin
    // ql_pin_set_gpio(60);//int_pin
    // err = ql_pin_set_gpio(61);//busy_pin
	// if(err !=0 ){
	// 	//LOGI("busy set gpio fail,err = %d",err);
	// 	QL_LOG(QL_LOG_LEVEL_INFO,"LLCC68IoInit","busy set gpio fail,err = %d",err);
	// }


    // //busy设置为上拉输入模式
    // err = ql_gpio_set_direction(GPIO_0,GPIO_INPUT);//busy_io_num
	// if(err != 0){
	// 	//LOGI("busy set input fail,err = %d",err);
	// 	QL_LOG(QL_LOG_LEVEL_INFO,"LLCC68IoInit","busy set input fail,err = %d",err);
	// }
    // err = ql_gpio_set_pull(GPIO_0,PULL_UP);
	// if(err != 0){
	// 	//LOGI("busy set pull up fail,err = %d",err);
	// 	QL_LOG(QL_LOG_LEVEL_INFO,"LLCC68IoInit","busy set pull up fail,err = %d",err);
	// }


    //int设置为中断引脚,下拉上升沿触发，启用防抖动
    // ql_gpio_set_direction(GPIO_3,GPIO_INPUT);//int_io_num
    // ql_gpio_set_pull(GPIO_3,PULL_DOWN);
    // ql_int_register(GPIO_3,EDGE_TRIGGER,DEBOUNCE_EN,EDGE_RISING,PULL_DOWN,dio1IrqCallback,NULL);//int_io_num

    //cs设置为输出高电平
    // ql_gpio_set_direction(GPIO_10,GPIO_OUTPUT);//ncs_io_num
	
	
	Spi1Init();

	return 1;
}

/* 初始化 DIO1
 * 将DIO1设置为外部中断(上升沿触发)，并且回调函数为 dioIrq 函数原型 void RadioOnDioIrq( void* context )
 */
void LLCC68IoIrqInit( DioIrqHandler dioIrq )
{
	// GPIO_InitTypeDef  GPIO_InitStructure;
	// EXTI_InitTypeDef EXTI_InitStructure;
 	// NVIC_InitTypeDef NVIC_InitStructure;
	
	dio1IrqCallback=dioIrq;
	
	// //gpio(DIO1)初始化为下拉输入模式
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	    //使能指定端口时钟
	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO口速度为50MHz，这里不用传参，直接写死用最大速度50MHZ
	// GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPD;//下拉输入
	// GPIO_InitStructure.GPIO_Pin = RADIO_DIO1_PIN;
	// GPIO_Init(RADIO_DIO1_PORT, &GPIO_InitStructure);	//初始化GPIO
	
	// //中断配置
  	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟
	// //GPIOE.2 中断线以及中断初始化配置   上升沿触发
  	// GPIO_EXTILineConfig(GPIO_PortSourceGPIOB,GPIO_PinSource11);
	// EXTI_InitStructure.EXTI_Line=EXTI_Line11;
	// EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
	// EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;	//上升沿触发
	// EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	// EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

	// //设置中断优先级
	// NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//使能按键WK_UP所在的外部中断通道
	// NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
	// NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//子优先级3
	// NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
	// NVIC_Init(&NVIC_InitStructure);
	// EXTI_ClearITPendingBit(EXTI_Line11);  //清除LINE11上的中断标志位 

	//ql_int_register(LORA_nINT,EDGE_TRIGGER,DEBOUNCE_EN,EDGE_RISING,PULL_DOWN,dio1IrqCallback,NULL);
	//ql_int_enable(LORA_nINT);
}

//在中断回调函数中回调 void LLCC68IoIrqInit( DioIrqHandler dioIrq ) 中注册的 dioIrq 回调
void EXTI15_10_IRQHandler(void){
	//printf("enter irq\r\n");
	// if(GPIO_ReadInputDataBit(RADIO_DIO1_PORT,RADIO_DIO1_PIN)){
	// 	LOGI("DIO1 irq\r\n");
	// 	if(NULL!=dio1IrqCallback){
	// 		dio1IrqCallback(NULL);
	// 	}
	// 	EXTI_ClearITPendingBit(EXTI_Line11);//清除中断标志位
	// }
	ql_LvlMode level = 0;
	ql_gpio_get_level(LORA_nINT,&level);
	if(level){
		//LOGI("DIO1 irq\r\n");
		if(dio1IrqCallback != NULL){
			dio1IrqCallback(NULL);
		}
		//ql好像没有清除中断标志位的选项
	}
}

void LLCC68IoDeInit( void )
{
    //GPIO去初始化代码
}

//复位按键功能
void LLCC68Reset( void )
{
	// GPIO_InitTypeDef  GPIO_InitStructure;

	// LLCC68DelayMs( 10 );
	
	// //将RST输出低电平
	// RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	    //使能指定端口时钟
	// GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //IO口速度为50MHz，这里不用传参，直接写死用最大速度50MHZ
	// GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;	//推挽输出
	// GPIO_InitStructure.GPIO_Pin = RADIO_nRESET_PIN;
	// GPIO_Init(RADIO_nRESET_PORT, &GPIO_InitStructure);	//初始化GPIO
	// GPIO_WriteBit( RADIO_nRESET_PORT, RADIO_nRESET_PIN,Bit_RESET);	//RST设为低电平输出
	
	// LLCC68DelayMs( 20 );
	
	// //将RST设置为上拉输入模式
	// GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	// GPIO_Init(RADIO_nRESET_PORT, &GPIO_InitStructure);	//初始化GPIO
	
	// LLCC68DelayMs( 10 );
	//ql_rtos_task_sleep_ms(10);

	// int RST_level = 2;
	// err = ql_gpio_set_direction(GPIO_2,GPIO_OUTPUT);
	// if(err != 0){
	// 	QL_LOG(QL_LOG_LEVEL_INFO,"RESET","GPIO2 dir set fail");
	// }

	// ql_gpio_get_level(GPIO_2,&RST_level);
	// QL_LOG(QL_LOG_LEVEL_INFO,"RESET","GPIO2 level start %d\n",RST_level); //初始为低电平,现在改为高低高

	// ql_gpio_set_level(GPIO_2,LVL_HIGH);
	// ql_gpio_set_level(GPIO_2,LVL_LOW);
	// ql_delay_us(200);
	// ql_gpio_set_level(GPIO_2,LVL_HIGH);

	// ql_gpio_get_level(GPIO_2,&RST_level);
	// QL_LOG(QL_LOG_LEVEL_INFO,"RESET","GPIO2 level end %d\n",RST_level);


	// //ql_rtos_task_sleep_ms(10);
	// err = ql_gpio_set_pull(GPIO_2,PULL_UP);
	// if(err !=0){
	// 	QL_LOG(QL_LOG_LEVEL_INFO,"RESET","GPIO2 pull set fail,err = %d",err); //设置为上拉失败,无效参数错误
	// }
	// err = ql_gpio_set_direction(GPIO_2,GPIO_INPUT);
	// if(err !=0){
	// 	QL_LOG(QL_LOG_LEVEL_INFO,"RESET","GPIO2 dir set fail");
	// }
	
}

//读取Busy引脚电平状态，等到busy引脚变为低电平
int LLCC68WaitOnBusy( void )
{
	QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68WaitOnBusy","LLCC68WaitOnBusy start");
	//uint32_t u32_count=0;
	//int flag = -1;
	// while( GPIO_ReadInputDataBit(RADIO_DIO4_BUSY_PORT,RADIO_DIO4_BUSY_PIN) == 1 ){
	// 	if(u32_count++>1000){
	// 		printf("wait busy pin timeout\r\n");
	// 		u32_count=0;
	// 	}
	// 	LLCC68DelayMs(1);
	// }

	ql_LvlMode busy_level = 2;
	// while(busy_level){
	// 	err = ql_gpio_get_level(LORA_BUSY,&busy_level);
	// 	if(err != 0) {
	// 		QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68WaitOnBusy","ql_gpio_get_level fail");
	// 		return -1;
	// 	}
	// 	if(busy_level==LVL_LOW){
	// 		break;
	// 	}else{
	// 		u32_count++;
	// 		if(u32_count>1000){
	// 			//LOGI("wait busy pin timeout\n");
	// 			u32_count=0;
	// 			flag = -1;
	// 			//LOGI("LLCC68WaitOnBusy timeout\n");
	// 			QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68WaitOnBusy","LLCC68WaitOnBusy timeout");
	// 			break;
	// 		}
	// 	}
	// 	ql_rtos_task_sleep_ms(1);
	// }

	int count = 1000;
	while(count--){
		err = ql_gpio_get_level(LORA_BUSY,&busy_level);
	
		if(busy_level==LVL_LOW){
			//flag = 1;
			
			return 1;
		}
		
	}
	//QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68WaitOnBusy","busy_level is %d\n",busy_level);
	//if(busy_level != 0){
		//flag = -1;
		//QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68WaitOnBusy","LLCC68WaitOnBusy timeout");
	//}
	//QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68WaitOnBusy","LLCC68WaitOnBusy end, flag = %d\n",flag);

	return -1;
}

//设置SPI的NSS引脚电平，0低电平，非零高电平
//返回值为0 true，
int LLCC68SetNss(uint8_t lev ){
	// if(lev){
	// 	GPIO_SetBits(RADIO_NSS_PORT,RADIO_NSS_PIN);	//输出高电平
	// }else{
	// 	GPIO_ResetBits(RADIO_NSS_PORT,RADIO_NSS_PIN);	//输出低电平
	// }
	int flag = 1;
	if(lev){
		err = ql_gpio_set_level(LORA_nCS,LVL_HIGH);
		if(err != 0 ){
			LOGI("ql_gpio_set_level LORA_nCS LVL_HIGH fail\n");
			flag = -1;
		}
	}else{
		err = ql_gpio_set_level(LORA_nCS,LVL_LOW);
		if(err != 0 ){
			LOGI("ql_gpio_set_level LORA_nCS LVL_LOW fail\n");
			flag = -1;
		}
	}
	return flag;
}

//spi传输一个字节的数据
int8_t LLCC68SpiInOut(uint8_t data){
	//return HALSpi1InOut(data);
	uint8_t rx_buf = 0;
	spi_err = ql_spi_read_follow_write(QL_SPI_PORT1,&data,sizeof(data),&rx_buf,sizeof(rx_buf));
	if(spi_err != 0){
		LOGI("LLCC68SpiInOut fail\n");
		return -1;
	}
	return rx_buf;
}

//检查频率是否符合要求，如果不需要判断则可以直接返回true
bool LLCC68CheckRfFrequency( uint32_t frequency )
{
    // Implement check. Currently all frequencies are supported
    return true;
}

//毫秒延时
void LLCC68DelayMs(uint32_t ms){
	//delay_ms(ms);
	ql_rtos_task_sleep_ms(ms);
}


#if 1
//tx/rx定时器操作

//定时器3中断服务程序
//tx定时结束需要回调 RadioOnTxTimeoutIrq
//rx定时结束需要回调 RadioOnRxTimeoutIrq
void TIM3_IRQHandler(void)   //TIM3中断
{
	// uint32_t diffMs=0;
	
	// if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET){
	// 	timer_count++;
	// 	TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志
		
	// 	//printf("timer_count=%d\r\n",timer_count);
		
	// 	//处理tx定时器
	// 	if(txTimerHandle.isRun){
	// 		if(timer_count>=txTimerHandle.startMs){
	// 			diffMs=timer_count-txTimerHandle.startMs;
	// 		}else{//溢出了
	// 			diffMs=0xffffffff - txTimerHandle.startMs + timer_count;
	// 		}
	// 		if(diffMs>txTimerHandle.delayMs){
	// 			//计时结束
	// 			//printf("----------------- tx timer irq ---------------\r\n");
	// 			LLCC68TxTimerStop();
	// 			txTimerHandle.startMs=timer_count;
	// 			RadioOnTxTimeoutIrq(NULL);	//tx定时结束需要回调 RadioOnTxTimeoutIrq
	// 		}
	// 	}
		
	// 	//处理rx定时器
	// 	if(rxTimerHandle.isRun){
	// 		if(timer_count>=rxTimerHandle.startMs){
	// 			diffMs=timer_count-rxTimerHandle.startMs;
	// 		}else{//溢出了
	// 			diffMs=0xffffffff - rxTimerHandle.startMs + timer_count;
	// 		}
	// 		if(diffMs>rxTimerHandle.delayMs){
	// 			//计时结束
	// 			//printf("----------------- rx timer irq ---------------\r\n");
	// 			LLCC68RxTimerStop();
	// 			rxTimerHandle.startMs=timer_count;
	// 			RadioOnRxTimeoutIrq(NULL);	//rx定时结束需要回调 RadioOnRxTimeoutIrq
	// 		}
	// 	}
	// }
}

//初始化定时器(这里用TIM3定时器模拟出了两个ms定时器)
//tx定时结束需要回调 RadioOnTxTimeoutIrq
//rx定时结束需要回调 RadioOnRxTimeoutIrq
void LLCC68TimerInit(void){
	// TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	// NVIC_InitTypeDef NVIC_InitStructure;

	// //使能TIM3
	// RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); //时钟使能
 
	// //TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	// //中断优先级NVIC设置
	// NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;  //TIM3中断
	// NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	// NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	// NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	// NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器
	
	// //定时器TIM3初始化
	// TIM_TimeBaseStructure.TIM_Period = 10; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	// TIM_TimeBaseStructure.TIM_Prescaler =SystemCoreClock/10000; //设置用来作为TIMx时钟频率除数的预分频值(1s10000次，也就是1次0.1ms)
	// TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	// TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	// TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
	
	// //关闭软件定时器
	// txTimerHandle.isRun=0;
	// rxTimerHandle.isRun=0;
	
	// TIM_Cmd(TIM3, ENABLE);  //使能TIMx
	// TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIMx更新中断标志
	// TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); //使能指定的TIM2中断,允许更新中断
}

void LLCC68SetTxTimerValue(uint32_t nMs){
	//printf("[%s()-%d]set timer out %d ms\r\n",__func__,__LINE__,nMs);
	//txTimerHandle.delayMs=nMs;
}

void LLCC68TxTimerStart(void){
	//printf("[%s()-%d]start timer\r\n",__func__,__LINE__);
	//txTimerHandle.startMs=timer_count;
	//txTimerHandle.isRun=1;
}

void LLCC68TxTimerStop(void){
	//printf("[%s()-%d]stop timer\r\n",__func__,__LINE__);
	txTimerHandle.isRun=0;
}

void LLCC68SetRxTimerValue(uint32_t nMs){
	//printf("[%s()-%d]set timer out %d ms\r\n",__func__,__LINE__,nMs);
	rxTimerHandle.delayMs=nMs;
}

void LLCC68RxTimerStart(void){
	//printf("[%s()-%d]start timer\r\n",__func__,__LINE__);
	rxTimerHandle.startMs=timer_count;
	rxTimerHandle.isRun=1;
}

void LLCC68RxTimerStop(void){
	//printf("[%s()-%d]stop timer\r\n",__func__,__LINE__);
	rxTimerHandle.isRun=0;
}

#endif
