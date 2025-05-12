#include <stdlib.h>
#include <unistd.h>
#include "lora_user.h"

#include "ql_api_spi.h"
#include "ql_api_osi.h"
#include "ql_gpio.h"

// /************************************************************************************/
#include "ql_log.h"
#define LOGI(msg, ...)  QL_LOG(QL_LOG_LEVEL_INFO, "lora_user", msg, ##__VA_ARGS__)

// #define     LORA_nCS    SPI_1_nCS
// #define     LORA_MOSI   SPI_1_MOSI
// #define     LORA_MISO   SPI_1_MISO
// #define     LORA_SCLK   SPI_1_SCLK
// #define     LORA_BUSY   SPI_2_CLK
// #define     LORA_nINT   SPI_2_MISO
// #define     LORA_RST    SPI_2_MOSI

// #define     LORA_nCS_PIN    4
// #define     LORA_MOSI_PIN   3
// #define     LORA_MISO_PIN   2
// #define     LORA_SCLK_PIN   1
// #define     LORA_BUSY_PIN   61
// #define     LORA_nINT_PIN   60
// #define     LORA_RST_PIN    59  
// /************************************************************************************/

// /*
// 1.gpio口的功能配置
// 2.spi的功能配置
// 3.
// */


// /************************************************************************************/

// /*lora模块gpio初始化*/
// void lora_gpio_init(void)
// {
//     LOGI("enter lora_gpio_init\n");
//     ql_errcode_gpio gpio_err = 0;

//     //设置为spi通信功能
//     ql_pin_set_func(LORA_SCLK_PIN,1);
//     ql_pin_set_func(LORA_MOSI_PIN,1);
//     ql_pin_set_func(LORA_MISO_PIN,1);
//     ql_pin_set_func(LORA_nCS_PIN,0);

//     //设置为gpio口
//     ql_pin_set_func(LORA_BUSY_PIN,0);
//     ql_pin_set_func(LORA_nINT_PIN,0);
//     ql_pin_set_func(LORA_RST_PIN,0);
// }

// /*lora模块spi初始化*/
// void lora_spi_init(void)
// {
//     LOGI("enter lora_spi_init\n");
//     ql_errcode_spi_e spi_err = 0;
//     ql_spi_config_s lora_spi_config = {
//         .input_mode = QL_SPI_INPUT_TRUE,            //SPI允许输入（读取）
//         .port = QL_SPI_PORT1,                       //spi_1 总线
//         .framesize = 16,                            //每个数据帧的位数
//         .spiclk = QL_SPI_CLK_1_5625MHZ,             //时钟频率
//         .cs_polarity0 = QL_SPI_CS_ACTIVE_LOW,       //片选信号极性1，spi总线操作时为低
//         .cs_polarity1 = QL_SPI_CS_ACTIVE_LOW,       //片选信号极性2，spi总线操作时为低
//         .cpol = QL_SPI_CPOL_LOW,                    //SPI未使能时，CLK线为低电平，第一个边沿是上升沿 
//         .cpha = QL_SPI_CPHA_1Edge,                  //MOSI延时一个边沿,CLK和MISO延时两个边沿，即发送的数据先准备好，才有CLK
//         .input_sel = QL_SPI_DI_1,                   //选择输入引脚
//         .transmode = QL_SPI_DIRECT_POLLING,         //传输模式
//         .cs = QL_SPI_CS0,                           //选择片选引脚
//         .cs_gpio = QL_SPI_CS0,                      //片选gpio口选择
//         .clk_delay = QL_SPI_CLK_DELAY_0,            //时钟延时选择
//         .release_flag = QL_SPI_NOT_RELEASE,         //使用完SPI总线不释放SPI总线
//     };

//     spi_err = ql_spi_init_ext(lora_spi_config);
//     if(spi_err != QL_SPI_SUCCESS) LOGI("ql_spi_init_ext fail, err = %d\n",spi_err);
// }

// /*lora模块初始化*/
// void lora_init(void)
// {
//     LOGI("enter lora_init\n");
//     lora_gpio_init();
//     lora_spi_init();
// }

// /*lora发送数据*/
// void lora_send(void)
// {
//     LOGI("enter lora_send\n");
// }

// /*lora接收数据*/
// void lora_receive(void)
// {
//     LOGI("enter lora_receive\n");
// }

// /*lora收发测试*/
// void lora_test(void)
// {
//     LOGI("enter lora_test\n");
//     lora_send();
//     lora_receive();
// }


//#include "LLCC68_example_recive.h"
#include "radio.h"
#include "project_config.h"
//#include "stdio.h"
//#include "stm32f10x_it.h"
//#include "delay.h"
#include "string.h"

/*!
 * Radio events function pointer
 * 这个是传参进入其他函数中了，所以用全局变量(局部变量使用完了内存释放可能导致异常)
 */
static RadioEvents_t LLCC68RadioEvents;

static void LLCC68OnTxDone( void );
static void LLCC68OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );
static void LLCC68OnTxTimeout( void );
static void LLCC68OnRxTimeout( void );
static void LLCC68OnRxError( void );

//开启一个定时发送任务，每隔1S发送一条数据
void ExampleLLCC68ReciveDemo(void){
	uint32_t u32_count=0;
	uint8_t OCP_Value = 0;
	LOGI("enter ExampleLLCC68ReciveDemo");
	
	LLCC68RadioEvents.TxDone = LLCC68OnTxDone;
	LLCC68RadioEvents.RxDone = LLCC68OnRxDone;
	LLCC68RadioEvents.TxTimeout = LLCC68OnTxTimeout;
	LLCC68RadioEvents.RxTimeout = LLCC68OnRxTimeout;
	LLCC68RadioEvents.RxError = LLCC68OnRxError;
	LOGI("set LLCC68RadioEvents end\n");

	/*初始化无线电*/
	int flag = Radio.Init( &LLCC68RadioEvents );
	if(flag == -1){
		LOGI("Radio.Init fail\n");
	}else{
		LOGI("Radio.Init sucess\n");
	}

#if 1
	/*设置信道*/
	Radio.SetChannel(LORA_FRE);
	LOGI("radio set channel end\n");

	/*配置发送参数*/
	Radio.SetTxConfig( MODEM_LORA, LORA_TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                     LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                     LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                     true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
	LOGI("radio set tx config end\n");
	//参数：lora模式,发射功率,fsk用的lora设置为0就可以，带宽，纠错编码率，前导码长度，
	//固定长度数据包(一般是不固定的所以选false)，crc校验，0表示关闭跳频，
	//跳频之间的符号数(关闭跳频这个参数没有意义)，这个应该是表示是否要翻转中断电平的，超时时间

	/*获取过流保护配置参数*/
	OCP_Value = Radio.Read(REG_OCP);
	LOGI("OCP_Value is %d\n", OCP_Value);
	
	/*配置接收参数*/
	Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
                     LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
                     LORA_LLCC68_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
                     0, true, 0, 0, LORA_IQ_INVERSION_ON, false );
	LOGI("radio set rx config end\n");

	//进入接收模式
	LOGI("start enter rx mode\n");
	Radio.Rx( 0 );	
	LOGI("all setting\n");
	LOGI("freq: %d\r\n Tx power: %d\r\n band width: %d\r\n FS: %d\r\n CODINGRATE: %d\r\n PREAMBLE_LENGTH: %d\r\n",LORA_FRE,LORA_TX_OUTPUT_POWER,LORA_BANDWIDTH,LORA_SPREADING_FACTOR,LORA_CODINGRATE,LORA_PREAMBLE_LENGTH);
	while(1){

		LOGI("start while recycle\n");
		//处理中断请求
		Radio.IrqProcess( ); // Process Radio IRQ
		u32_count++;
		//delay_ms(1);
        ql_rtos_task_sleep_ms(1);
	}
#endif
}

static void LLCC68OnTxDone( void )
{
	// LOGI("TxDone\r\n");
	Radio.Standby();
	
	// //发送完成闪烁一下led提示
	// GPIO_ResetBits(GPIOB,GPIO_Pin_12);
	// delay_ms(100);
	// GPIO_SetBits(GPIOB,GPIO_Pin_12);
}

static void LLCC68OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	uint32_t reciveNumber=0;
	Radio.Standby();
	//LOGI("RxDone\r\nsize:%d\r\nrssi:%d\r\nsnr:%d\r\n",size,rssi,snr);
	Radio.Rx( 0 );
	if(size > 0 ){
		
		if(size==4){
			memcpy(&reciveNumber,payload,4);
			//LOGI("recive u32 data=%d\r\n",reciveNumber);
		}else{
			//LOGI("recive data: %s\r\n",(char*)payload);
		}
		//接收成功闪烁一下led提示
		//GPIO_ResetBits(GPIOB,GPIO_Pin_12);
		//delay_ms(100);
		//GPIO_SetBits(GPIOB,GPIO_Pin_12);
	}else {
		//LOGI("recive size !=4 is error\r\n");
	}
}

static void LLCC68OnTxTimeout( void )
{
	//LOGI("TxTimeout\r\n");
}

static void LLCC68OnRxTimeout( void )
{
	Radio.Standby();
	//LOGI("RxTimeout retry recive\r\n");
	Radio.Rx( LORA_RX_TIMEOUT_VALUE ); 
}

static void LLCC68OnRxError( void )
{
	Radio.Standby();
	//LOGI("RxError retry recive\r\n");
	Radio.Rx(LORA_RX_TIMEOUT_VALUE); 
}

