#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "ql_api_osi.h"
#include "ql_api_spi.h"
//#include "ql_gpio.h"
#include "ql_log.h"
#include "./inc/lora.h"
#include "./inc/user.h"

/*宏定义*/
#define LOGI(msg, ...)  QL_LOG(QL_LOG_LEVEL_INFO, "lora_test", msg, ##__VA_ARGS__)
#define USE_TEST 0

/*全局变量*/
int flag;
ql_task_t lora_demo_test_task_t = NULL;
extern unsigned char *write_buf;
extern unsigned char *read_buf;
int count = 1;

/*函数*/
#if USE_TEST
void lora_send_test(void)
{
    //uint16_t syncword = lora_get_syncword(); //00011101 00000111 01000000
    //LOGI("syncword = 0x%04X\n",syncword);
    //lora_set_standby(0x00);

    wait_busy_low();
    write_buf[0] = 0x1D;
    write_buf[1] = 0x07;
    write_buf[2] = 0x40;
    ql_spi_write_read(QL_SPI_PORT1, read_buf,write_buf, 5);

    LOGI("read_buf 0 = 0x%02X, read_buf 1 = 0x%02X\n",read_buf[0],read_buf[1]);
    LOGI("read_buf 2 = 0x%02X, read_buf 3 = 0x%02X\n",read_buf[2],read_buf[3]);
    LOGI("read_buf 4 = 0x%02X, read_buf 5 = 0x%02X\n",read_buf[4],read_buf[5]);
}
#endif

void lora_receive_mode(void)
{
    /*获取中断状态*/
    uint16_t irq_status = lora_get_irq_status();

    /*清除中断状态*/
    lora_clear_irq_status(irq_status);

    /*根据中断状态进行操作*/
    if((irq_status & IRQ_TX_DONE) == IRQ_TX_DONE){

    }
    if( ( irq_status & IRQ_RX_DONE ) == IRQ_RX_DONE ){

    }
    if( ( irq_status & IRQ_CAD_DONE ) == IRQ_CAD_DONE ){

    }
    if( ( irq_status & IRQ_RX_TX_TIMEOUT ) == IRQ_RX_TX_TIMEOUT ){

    }
    if( ( irq_status & IRQ_PREAMBLE_DETECTED ) == IRQ_PREAMBLE_DETECTED ){

    }
    if( ( irq_status & IRQ_SYNCWORD_VALID ) == IRQ_SYNCWORD_VALID ){

    }
    if( ( irq_status & IRQ_HEADER_VALID ) == IRQ_HEADER_VALID ){

    }
    if( ( irq_status & IRQ_HEADER_ERROR ) == IRQ_HEADER_ERROR ){

    }
}

void lora_demo_test_task(void *param)
{
    lora_init();
#if !USE_TEST
    lora_start();
    lora_radioConfig();
#endif

    while(1){
        LOGI("enter while in %d times\n",count);
        count++;

#if USE_TEST
        lora_send_test();
#else
        /*接收数据*/
        lora_receive_mode();
#endif

        ql_rtos_task_sleep_ms(1);
    }
}

void lora_demo_test_thread_init1(void)
{
    QlOSStatus err = ql_rtos_task_create(&lora_demo_test_task_t,4096,APP_PRIORITY_NORMAL,"lora",lora_demo_test_task,NULL,1);
    if(err != 0){
        LOGI("create lora_demo_task failed\n");
    }
}