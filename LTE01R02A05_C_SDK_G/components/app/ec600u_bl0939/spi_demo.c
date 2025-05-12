/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------

=================================================================*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"

#include "ql_api_spi.h"

#include "ql_log.h"

#include "ql_gpio.h"
#include "ql_power.h"
#define SPI_DEMO_TASK_PRIO           12
#define SPI_DEMO_TASK_STACK_SIZE     8*1024
#define SPI_DEMO_TASK_EVENT_CNT      4

#define QL_SPI_DEMO_LOG_LEVEL       	        QL_LOG_LEVEL_INFO
#define QL_SPI_DEMO_LOG(msg, ...)			    QL_LOG(QL_SPI_DEMO_LOG_LEVEL, "ql_SPI_DEMO", msg, ##__VA_ARGS__)
#define QL_SPI_DEMO_LOG_PUSH(msg, ...)	        QL_LOG_PUSH("ql_SPI_DEMO", msg, ##__VA_ARGS__)

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "spi_demo", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "spi_demo", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "spi_demo", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "spi_demo", msg, ##__VA_ARGS__)

/**
 *  使用SPI DMA注意事项：
 *  8910:
 *  1. SPI DMA POLLING和SPI DMA IRQ只支持8bit和16bit的数据传输，不支持32bit数据传输
 *  2. 在使用16bit传输数据时，DMA实际使用的是32bit位宽，需要对输出数据插入一些无效数据，对输入数据去除无效数据，
 *     因此新增16bit dma api用于16bit情况下的读写，api包含ql_spi_write_16bit_dma、ql_spi_read_16bit_dma、
 *     ql_spi_write_read_16bit_dma，这部分代码在demo中开源，客户可自行优化，或直接使用
 *  3. QL_SPI_16BIT_DMA置为1表示使用16bit DMA demo
 *  8850:
 *  1. SPI DMA POLLING和SPI DMA IRQ支持8bits、16bits和32bits的数据传输,但是传输的数据大小需要framesize对齐。
 */
#define QL_SPI_16BIT_DMA                   0        //16bit DMA demo
#define QL_SPI_DEMO_LOW_POWER_USE          0        //0-not run in lower power mode；1-run in lower power mode

ql_task_t spi_demo_task = NULL;
ql_sem_t  spi_demo_write;
ql_sem_t  spi_demo_read;
int spi_power_lock = 0;


#define QL_SPI_DEMO_WAIT_NONE              0
#define QL_SPI_DEMO_WAIT_WRITE             1
#define QL_SPI_DEMO_WAIT_READ              2

unsigned char spi_demo_wait_write_read = QL_SPI_DEMO_WAIT_NONE;

#if 0
#define QL_CUR_SPI_PORT             QL_SPI_PORT1
#define QL_CUR_SPI_CS_PIN           QL_CUR_SPI1_CS_PIN
#define QL_CUR_SPI_CS_FUNC          QL_CUR_SPI1_CS_FUNC
#define QL_CUR_SPI_CLK_PIN          QL_CUR_SPI1_CLK_PIN
#define QL_CUR_SPI_CLK_FUNC         QL_CUR_SPI1_CLK_FUNC
#define QL_CUR_SPI_DO_PIN           QL_CUR_SPI1_DO_PIN
#define QL_CUR_SPI_DO_FUNC          QL_CUR_SPI1_DO_FUNC
#define QL_CUR_SPI_DI_PIN           QL_CUR_SPI1_DI_PIN
#define QL_CUR_SPI_DI_FUNC          QL_CUR_SPI1_DI_FUNC
#else
#define QL_CUR_SPI_PORT             QL_SPI_PORT2
#define QL_CUR_SPI_CS_PIN           QL_CUR_SPI2_CS_PIN
#define QL_CUR_SPI_CS_FUNC          QL_CUR_SPI2_CS_FUNC
#define QL_CUR_SPI_CLK_PIN          QL_CUR_SPI2_CLK_PIN
#define QL_CUR_SPI_CLK_FUNC         QL_CUR_SPI2_CLK_FUNC
#define QL_CUR_SPI_DO_PIN           QL_CUR_SPI2_DO_PIN
#define QL_CUR_SPI_DO_FUNC          QL_CUR_SPI2_DO_FUNC
#define QL_CUR_SPI_DI_PIN           QL_CUR_SPI2_DI_PIN
#define QL_CUR_SPI_DI_FUNC          QL_CUR_SPI2_DI_FUNC
#endif
#define QL_TYPE_SHIFT_8             8

uint32_t g_inbuf[QL_SPI_DMA_IRQ_SIZE/4] OSI_CACHE_LINE_ALIGNED;
uint32_t g_outbuf[QL_SPI_DMA_IRQ_SIZE/4] OSI_CACHE_LINE_ALIGNED;
static uint8_t spi_txbuf[36] OSI_CACHE_LINE_ALIGNED;
static uint8_t spi_rxbuf[36] OSI_CACHE_LINE_ALIGNED;
static int spi_wait_write_read = 0;
#define QL_SPI_WAIT_NONE 0
#define QL_SPI_WAIT_WRITE 1
#define QL_SPI_WAIT_READ 2
static int spi_write(int fd, uint8_t *inbuf, int len)
{
    int ret;
    spi_wait_write_read = QL_SPI_WAIT_WRITE;
    ql_spi_request_sys_clk(fd);
    ret = ql_spi_write(fd, inbuf, len);
    ql_rtos_semaphore_wait(spi_demo_write, 10);
    QL_SPI_DEMO_LOG("spi_write inbuf=%p,len=%d,ret=%d", inbuf, len, ret);
    return ret;
}

static int spi_write_read(int fd, uint8_t *outbuf, uint8_t *inbuf, int len)
{
    int ret;
    spi_wait_write_read = QL_SPI_WAIT_READ;
    ret = ql_spi_write_read(fd, inbuf, outbuf, len);
    ql_rtos_semaphore_wait(spi_demo_read, 10);
    QL_SPI_DEMO_LOG("spi_write_read inbuf=%p,outbuf=%p,len=%d,ret=%d", inbuf, outbuf, len, ret);
    return ret;
}

void ql_spi_read_data_transform(unsigned char *buf, unsigned int len)
{
    if(len%2 != 0 || len > QL_SPI_DMA_IRQ_SIZE/2)
    {
        QL_SPI_DEMO_LOG("invalid parm");
        return;
    }

    for(int i = 0; i < len/2; i++)
    {
        buf[i*2] = (g_inbuf[i] >> QL_TYPE_SHIFT_8) & 0xFF;
        buf[i*2+1] = g_inbuf[i] & 0xFF;
    }
}

ql_errcode_spi_e ql_spi_write_16bit_dma(ql_spi_port_e port, unsigned char *buf, unsigned int len)
{
    if(len%2 != 0 || len > QL_SPI_DMA_IRQ_SIZE/2)
    {
        QL_SPI_DEMO_LOG("invalid parm");
        return QL_SPI_PARAM_DATA_ERROR;
    }
    
    unsigned short out_temp = 0;

    for(int i = 0; i < len/2; i++)
    {
        out_temp = buf[i*2];
        g_outbuf[i] = (out_temp << QL_TYPE_SHIFT_8) + buf[i*2+1];
    }

    return ql_spi_write(port, (unsigned char*)g_outbuf, len*2);
}

ql_errcode_spi_e ql_spi_read_16bit_dma(ql_spi_port_e port, unsigned char *buf, unsigned int len)
{
    if(len%2 != 0 || len > QL_SPI_DMA_IRQ_SIZE/2)
    {
        QL_SPI_DEMO_LOG("invalid parm");
        return QL_SPI_PARAM_DATA_ERROR;
    }

    return ql_spi_read(port, (unsigned char*)g_inbuf, len*2);
}

ql_errcode_spi_e ql_spi_write_read_16bit_dma(ql_spi_port_e port, unsigned char *inbuf, unsigned char *outbuf, unsigned int len)
{
    if(len%2 != 0 || len > QL_SPI_DMA_IRQ_SIZE/2)
    {
        QL_SPI_DEMO_LOG("invalid parm");
        return QL_SPI_PARAM_DATA_ERROR;
    }
    unsigned short out_temp = 0;
    
    for(int i = 0; i < len/2; i++)
    {
        out_temp = outbuf[i*2];
        g_outbuf[i] = (out_temp << QL_TYPE_SHIFT_8) + outbuf[i*2+1];
    }
    
    return ql_spi_write_read(port, (unsigned char*)g_inbuf, (unsigned char*)g_outbuf, len*2);
}

void ql_spi_flash_data_printf(unsigned char *data, int len)
{
    int i = 0;
    int count_len = 256;
    int count = 0;
    int exit = 0;
	int write_len = 0;
	int pos = 0;
    
    unsigned char *str_data = (unsigned char *)malloc(len*2+64);
    if (str_data == NULL)
    {
        QL_SPI_DEMO_LOG("malloc err");
        return ; 
    }

    QL_SPI_DEMO_LOG("read len=%d", len);
    
    while ((exit == 0))
    {
        if (len - count > 256)
        {
            count_len = 256;
        }
        else 
        {
            count_len = len - count;
            exit = 1;
        }
        
        memset(str_data, 0, len*2+64);
        for (i=count; i<count+count_len; i++)
        {
            //sprintf((char *)str_data,"%s%02x", str_data, data[i]);
            write_len += snprintf((char *)(str_data + write_len),(len*2+64 - write_len), "%02x", data[i]);
        }
        QL_SPI_DEMO_LOG("data[%d-%d]=%s", count, count+count_len, (char *)(str_data+pos));
        pos += write_len;
        count += count_len;
    }

    free(str_data);
}

void ql_spi_demo_cb_handler(ql_spi_irq_s cause)
{
    if (cause.tx_dma_done == 1 && spi_demo_wait_write_read == QL_SPI_DEMO_WAIT_WRITE)
    {
        spi_demo_wait_write_read = QL_SPI_DEMO_WAIT_NONE;
        ql_rtos_semaphore_release(spi_demo_write);
    }

    if (cause.rx_dma_done == 1 && spi_demo_wait_write_read == QL_SPI_DEMO_WAIT_READ)
    {
        spi_demo_wait_write_read = QL_SPI_DEMO_WAIT_NONE;
        ql_rtos_semaphore_release(spi_demo_read);
    }

    QL_SPI_DEMO_LOG("cause.tx_dma_done=%d", cause.tx_dma_done);
    QL_SPI_DEMO_LOG("cause.rx_dma_done=%d", cause.rx_dma_done);

}

static void ql_spi_demo_task_pthread(void *ctx)
{
    QlOSStatus err = 0;
    unsigned char *outdata = "outdata";
    unsigned short outlen;
    unsigned char *indata = NULL;
    unsigned short inlen;
    unsigned char *out_mal_data = NULL;
    unsigned char *in_mal_data = NULL;
    ql_errcode_gpio ret;
    ql_spi_clk_e spiclk;
    ql_spi_transfer_mode_e transmode;
    unsigned int framesize;
    unsigned int tx_free = 0;
    ql_spi_config_s spi_config = {0};
    
    if (QL_CUR_SPI_CS_PIN == QUEC_PIN_NONE || QL_CUR_SPI_CS_PIN == QUEC_PIN_NONE || \
        QL_CUR_SPI_DO_PIN == QUEC_PIN_NONE || QL_CUR_SPI_DI_PIN == QUEC_PIN_NONE)
    {
        QL_SPI_DEMO_LOG("pin err");
    }

    ret = ql_pin_set_func(QL_CUR_SPI_CS_PIN, QL_CUR_SPI_CS_FUNC);
    if (ret != QL_GPIO_SUCCESS)
    {
        QL_SPI_DEMO_LOG("set pin err");
        
    }
    
    ret = ql_pin_set_func(QL_CUR_SPI_CLK_PIN, QL_CUR_SPI_CLK_FUNC);
    if (ret != QL_GPIO_SUCCESS)
    {
        QL_SPI_DEMO_LOG("set pin err");
        
    }
    ret = ql_pin_set_func(QL_CUR_SPI_DO_PIN, QL_CUR_SPI_DO_FUNC);
    if (ret != QL_GPIO_SUCCESS)
    {
        QL_SPI_DEMO_LOG("set pin err");
        
    }
    ret = ql_pin_set_func(QL_CUR_SPI_DI_PIN, QL_CUR_SPI_DI_FUNC);
    if (ret != QL_GPIO_SUCCESS)
    {
        QL_SPI_DEMO_LOG("set pin err");
        
    }

    //If you use the default parameters, you can initialize it with ql_spi_init
  /*transmode = QL_SPI_DMA_IRQ;
    spiclk = QL_SPI_CLK_25MHZ;
    framesize = 8;
    ql_spi_init(QL_CUR_SPI_PORT, transmode, spiclk);
  */

    spi_config.input_mode = QL_SPI_INPUT_TRUE;
    spi_config.port = QL_CUR_SPI_PORT;
    framesize = 8;
    transmode = QL_SPI_DMA_IRQ; //使用QL_SPI_DMA_IRQ模式，传输一次最大的数据量为512个字节
    spiclk = QL_SPI_CLK_1_5625MHZ;
    spi_config.spiclk = spiclk;
    spi_config.framesize = framesize;
    spi_config.cs_polarity0 = QL_SPI_CS_ACTIVE_LOW;
    spi_config.cs_polarity1 = QL_SPI_CS_ACTIVE_LOW;
    spi_config.cpol = QL_SPI_CPOL_LOW;
    spi_config.cpha = QL_SPI_CPHA_1Edge;
    spi_config.input_sel = QL_SPI_DI_1;
    spi_config.transmode = transmode;  
    spi_config.cs = QL_SPI_CS0;
    spi_config.clk_delay = QL_SPI_CLK_DELAY_0;
    ql_rtos_task_sleep_ms(5000);
    ql_spi_init_ext(spi_config);
    ql_spi_cs_auto(QL_CUR_SPI_PORT);


    //使用QL_SPI_DMA_IRQ模式才会使用到信号量
    if(transmode == QL_SPI_DMA_IRQ)
    {
        ql_spi_irq_s mask = {0};
        mask.rx_dma_done = 1;
        mask.tx_dma_done = 1;
        ql_rtos_semaphore_create(&spi_demo_write, 0);
        ql_rtos_semaphore_create(&spi_demo_read, 0);
        ql_spi_set_irq(QL_CUR_SPI_PORT, mask, ql_spi_demo_cb_handler);
    }

    while(1)
    {
        spi_txbuf[0] = 0x01;
        int ret = spi_write_read(QL_CUR_SPI_PORT, spi_txbuf, spi_rxbuf, sizeof(spi_txbuf));
        spi_write(QL_CUR_SPI_PORT, spi_txbuf,sizeof(spi_txbuf));
        QL_SPI_DEMO_LOG("SPI_TXBUF[0] = 0x%x", spi_txbuf[0]);
        ql_rtos_task_sleep_ms(50);
    }   

}

QlOSStatus ql_spi_demo_init(void)
{	
	QlOSStatus err = QL_OSI_SUCCESS;
#if QL_SPI_DEMO_LOW_POWER_USE
    spi_power_lock = ql_lpm_wakelock_create("spi_irq", strlen("spi_irq"));
#endif
	err = ql_rtos_task_create(&spi_demo_task, SPI_DEMO_TASK_STACK_SIZE, SPI_DEMO_TASK_PRIO, "ql_spi_demo", ql_spi_demo_task_pthread, NULL, SPI_DEMO_TASK_EVENT_CNT);
	if(err != QL_OSI_SUCCESS)
	{
		QL_SPI_DEMO_LOG("demo_task created failed");
        return err;
	}
    
    return err;
}

