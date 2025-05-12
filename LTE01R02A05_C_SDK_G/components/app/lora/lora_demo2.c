#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "ql_api_osi.h"
#include "ql_api_spi.h"
#include "ql_gpio.h"
#include "radio.h"
#include "project_config.h"
#include "ql_log.h"
#define LOGI(msg, ...)  QL_LOG(QL_LOG_LEVEL_INFO, "lora_demo2", msg, ##__VA_ARGS__)

ql_task_t lora_demo2_task_t = NULL;
static RadioEvents_t LLCC68RadioEvents;
unsigned char * tx_buf = NULL;
unsigned char * tx_mal_buf = NULL;
unsigned char * rx_buf = NULL;
unsigned char * rx_mal_buf = NULL;
unsigned short tx_len = 512;
unsigned short rx_len = 512;

void lora_tx_rx_buf_init(void)
{
    LOGI("enter lora_tx_rx_buf_init\n");
    tx_mal_buf = (unsigned char *)malloc(QL_SPI_DMA_ADDR_ALIN+tx_len);
    rx_mal_buf = (unsigned char *)malloc(QL_SPI_DMA_ADDR_ALIN+rx_len);

    //32对齐
    tx_buf = (unsigned char *)OSI_ALIGN_UP(tx_mal_buf, QL_SPI_DMA_ADDR_ALIN);
    rx_buf = (unsigned char *)OSI_ALIGN_UP(rx_mal_buf, QL_SPI_DMA_ADDR_ALIN);

    //清零
    memset(tx_buf, 0x00, tx_len);
    memset(rx_buf, 0x00, rx_len);
}

void int_callback(void *param)
{

}

void lora_BUSY_nRST_INT_init(void)
{
    /*
    busy 61 GPIO_0
    rst  59 GPIO_2
    int  60 GPIO_3
    设置为gpio功能，busy上拉输入，int中断管，下拉上升沿触发，rst先输出复位，延时100us后再上拉输入
    */
    LOGI("enter lora_BUSY_nRST_INT_init");
    ql_errcode_gpio err = 0;
    err = ql_pin_set_gpio(59);                       if(err!=0) LOGI("set rst gpio err=%d\n",err);
    err = ql_pin_set_gpio(60);                       if(err!=0) LOGI("set int gpio err=%d\n",err);
    err = ql_pin_set_gpio(61);                       if(err!=0) LOGI("set busy gpio err=%d\n",err);
    err = ql_pin_set_func(66,0);                     if(err!=0) LOGI("set gpio66 func err=%d\n",err);//设置成spi功能防止和pin61冲突

    /*busy*/
    err = ql_gpio_set_direction(GPIO_0,GPIO_INPUT);                     if(err!=0) LOGI("set busy direction err=%d\n",err);

    /*int*/
    err = ql_gpio_set_direction(GPIO_3,GPIO_INPUT);                     if(err!=0) LOGI("set int direction err=%d\n",err);
    err = ql_int_register(GPIO_3,EDGE_TRIGGER,DEBOUNCE_EN,EDGE_RISING,PULL_DOWN,int_callback,NULL);
                                                                        if(err!=0) LOGI("set int register err=%d\n",err);
    err = ql_int_enable(GPIO_3);                                        if(err!=0) LOGI("set int enable err=%d\n",err);

    /*rst*/
    err = ql_gpio_set_direction(GPIO_2,GPIO_OUTPUT);                    if(err!=0) LOGI("set rst direction err=%d\n",err);
    err = ql_gpio_set_level(GPIO_2,LVL_HIGH);                            if(err!=0) LOGI("set rst low err=%d\n",err);

    uint8_t func = 3;
    ql_pin_get_func(66,&func);
    LOGI("pin 66 func = %d\n",func);

}

/*return 1 -> sucess,return 0 -> timeout*/
int busy_wait_low(void)
{
    ql_LvlMode busy_level = LVL_HIGH;
    int retry = 10000;
    while(retry --){
        ql_gpio_get_level(GPIO_0, &busy_level);
        if(busy_level==0){
            LOGI("busy is set low in %d times\n",10000-retry);
            return 1;
        }
    }
    LOGI("busy wait tmeout");
    return 0;
}

int reset(void)
{
    ql_errcode_gpio err = 0;
    err = ql_gpio_set_level(GPIO_2,LVL_LOW);   
    if(err!=0){
        LOGI("set rst low err=%d\n",err);
        return 0;
    };

    ql_delay_us(1000);
    
    err = ql_gpio_set_level(GPIO_2,LVL_HIGH);   
    if(err!=0){
        LOGI("set rst high err=%d\n",err);
        return 0;
    } 
 
    if(busy_wait_low() == 0){
        return 0;
    }
    return 1;
}

static void LLCC68OnTxDone( void )
{
	LOGI("TxDone\r\n");
	Radio.Standby();
}

static void LLCC68OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr )
{
	uint32_t reciveNumber=0;
	Radio.Standby();
	LOGI("RxDone\r\nsize:%d\r\nrssi:%d\r\nsnr:%d\r\n",size,rssi,snr);
	Radio.Rx( 0 );
	if(size > 0 ){
		
		if(size==4){
			memcpy(&reciveNumber,payload,4);
			LOGI("recive u32 data=%d\r\n",reciveNumber);
		}else{
			LOGI("recive data: %s\r\n",(char*)payload);
		}
	}else {
		LOGI("recive size !=4 is error\r\n");
	}
}

static void LLCC68OnTxTimeout( void )
{
	LOGI("TxTimeout\r\n");
}

static void LLCC68OnRxTimeout( void )
{
	Radio.Standby();
	LOGI("RxTimeout retry recive\r\n");
	Radio.Rx( LORA_RX_TIMEOUT_VALUE ); 
}

static void LLCC68OnRxError( void )
{
	Radio.Standby();
	LOGI("RxError retry recive\r\n");
	Radio.Rx(LORA_RX_TIMEOUT_VALUE); 
}

void printf_info(void)
{
    // ql_LvlMode cs_level = LVL_LOW;
    // ql_GpioDir cs_dir = GPIO_INPUT;
    // ql_gpio_get_direction(GPIO_10, &cs_dir);
    // ql_gpio_get_level(GPIO_10, &cs_level);
    // LOGI("nCS info: level = %d, dir = %d\n",cs_level,cs_dir); //片选引脚是输出高

    // ql_LvlMode busy_level = 0;
    // ql_GpioDir busy_dir = GPIO_INPUT;
    // ql_PullMode busy_pull = PULL_NONE;
    // ql_gpio_get_pull(GPIO_0,&busy_pull);
    // ql_gpio_get_level(GPIO_0, &busy_level);  
    // ql_gpio_get_direction(GPIO_0, &busy_dir);
    // LOGI("busy info: level = %d, dir = %d, pull = %d\n",busy_level,busy_dir,busy_pull); //busy引脚低电平上拉输入

    // ql_LvlMode VCC33_level = 0;
    // uint8_t VCC33_func = 100;
    // ql_pin_get_func(16,&VCC33_func);
    // ql_gpio_get_level(GPIO_19, &VCC33_level);
    // LOGI("VCC33 info: func= %d, level = %d\n",VCC33_func,VCC33_level); //4,0

    int pin_func = 100;
    ql_pin_get_func(2,&pin_func);
    LOGI("miso func = %d\n",pin_func);
    ql_pin_get_func(100,&pin_func);
    LOGI("pin 100 func = %d\n",pin_func);

    ql_GpioDir miso_dir = GPIO_OUTPUT;
    ql_gpio_get_direction(GPIO_12,&miso_dir);
    LOGI("miso dir = %d\n",miso_dir);
}

void send_test(void)
{
    rx_buf[0] = 0;
    rx_buf[1] = 0;
    
    /*set standby RC*/
    tx_buf[0] = 0x80; //10000000
    tx_buf[1] = 0x00;
#if 0
    /*read syncword*/
    tx_buf[0] = 0x1D; //00011101
    tx_buf[1] = 0x07; //00000111
    tx_buf[2] = 0x40; //01000000
    tx_buf[3] = 0x00;
    tx_buf[4] = 0x00;
    tx_buf[5] = 0x00;

    /*set dio2 asrf switch ctrl*/
    tx_buf[0] = 0x9D; //1001 1101
    tx_buf[1] = 0x01; //0000 0001

    /*set over current protection*/
    tx_buf[0] = 0x0D; //0000 1101
    tx_buf[1] = 0x08; //0000 1000
    tx_buf[2] = 0xE7; //1110 0111
    tx_buf[3] = 0x18; //0001 1000

    /*set syncword*/
    uint8_t syncWordBuf[] = { 0xC1, 0x94, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00 };
    tx_buf[0] = 0x0D;
    tx_buf[1] = 0x06;
    tx_buf[2] = 0xC0;
    tx_buf[3] = 0xC1;
    tx_buf[4] = 0x94;
    tx_buf[5] = 0xC1;
    tx_buf[6] = 0x00;
    tx_buf[7] = 0x00;
    tx_buf[8] = 0x00;
    tx_buf[9] = 0x00;
    tx_buf[10] = 0x00;

    SetRxBoosted
    tx_buf[0] = 0x0D;
    tx_buf[1] = 0x08;
    tx_buf[2] = 0xAC;
    tx_buf[3] = 0x96;

    /*get status*/
    tx_buf[0] = 0xC0;//1100 0000
    tx_buf[1] = 0;

    /*set packetype*/
    tx_buf[0] = 0x8A; //10001010
    tx_buf[1] = 0x01; //00000001

    /*get packetype*/
    tx_buf[0] = 0x11;
    tx_buf[1] = 0;
#endif
    ql_errcode_spi_e spi_err =  ql_spi_write_read(QL_SPI_PORT1,rx_buf,tx_buf,2);
    // if(spi_err != 0){
    //     LOGI("ql_spi_write_read fail,spi_err = %d\n",spi_err);
    //     return;
    // }
    // LOGI("ql_spi_write_read sucess\n");
    //busy_wait_low();
    ql_delay_us(100);
    LOGI("rx_buf 0 = 0x%02X, rx_buf 1 = 0x%02X\n",rx_buf[0],rx_buf[1]);
    //LOGI("rx_buf 2 = 0x%02X, rx_buf 3 = 0x%02X\n",rx_buf[2],rx_buf[3]);
    //LOGI("rx_buf 4 = 0x%02X, rx_buf 5 = 0x%02X\n",rx_buf[4],rx_buf[5]);
}

void lora_demo2_task(void *param)
{
#if 1
    LOGI("enter lora_demo2_task\n");
    uint8_t OCP_Value = 0;
    ql_pin_set_gpio(16);
    ql_gpio_set_direction(GPIO_19,GPIO_OUTPUT);
    ql_gpio_set_level(GPIO_19,LVL_LOW);

    /*初始化收发内存空间*/
    lora_tx_rx_buf_init();
    LOGI("lora_tx_rx_buf_init end");

    /*初始化busy，rst，int引脚*/
    ql_delay_us(1000);
    lora_BUSY_nRST_INT_init();
    LOGI("lora_BUSY_nRST_INT_init end");

    /*初始化无线电*/
    LLCC68RadioEvents.TxDone = LLCC68OnTxDone;
	LLCC68RadioEvents.RxDone = LLCC68OnRxDone;
	LLCC68RadioEvents.TxTimeout = LLCC68OnTxTimeout;
	LLCC68RadioEvents.RxTimeout = LLCC68OnRxTimeout;
	LLCC68RadioEvents.RxError = LLCC68OnRxError;
    Radio.Init(&LLCC68RadioEvents);
    LOGI("Radio init end\n");

    /*从机复位*//*理论上通过rst输出引脚进行电平复位后，从机将busy引脚拉低*/
    if(reset()) LOGI("reset success\n");
    else LOGI("reset fail\n");
#endif   
#if 0


    /*设置信道*/
	Radio.SetChannel(LORA_FRE);
    LOGI("SetTxConfig end\n");

    /*配置发送参数*/
    Radio.SetTxConfig(  MODEM_LORA, LORA_TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                        LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                        LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                        true, 0, 0, LORA_IQ_INVERSION_ON, 3000 );
    LOGI("SetTxConfig end\n");

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
	Radio.Rx(0);
    
#endif
    printf_info();
    int count = 1;
    while(1){
        LOGI("enter while %d\n",count);
        count++;
        send_test();        
        ql_rtos_task_sleep_ms(1);        
        //处理中断请求
		//Radio.IrqProcess( ); // Process Radio IRQ
        //LOGI("IrqProcess end\n");       
    }
}

void lora_demo2_thread_init(void)
{
    QlOSStatus err = ql_rtos_task_create(&lora_demo2_task_t,4096,APP_PRIORITY_NORMAL,"lora",lora_demo2_task,NULL,1);
    if(err != 0){
        LOGI("create lora_demo_task failed\n");
    }
}

