#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "ql_gpio.h"
#include "ql_api_spi.h"
#include "ql_api_osi.h"
#include "ql_log.h"

#include "./inc/user.h"

#define LOGI(msg, ...)  QL_LOG(QL_LOG_LEVEL_INFO, "user", msg, ##__VA_ARGS__)

/*全局变量*/
ql_errcode_gpio gpio_err;
ql_errcode_spi_e spi_err;
ql_LvlMode busy_level = LVL_HIGH;
unsigned char * write_buf = NULL;
unsigned char * write_mal_buf = NULL;
unsigned char * read_buf = NULL;
unsigned char * read_mal_buf = NULL;
unsigned short write_len = 512;
unsigned short read_len = 512;

/*函数*/
int wait_busy_low(void)
{
    int retry = 10000;
    while(retry--){
        ql_gpio_get_level(SPI_BUSY_IO,&busy_level);
        if(busy_level == LVL_LOW) return 1;
    }
    return 0;
}

int lora_wrtie_register(uint16_t add,uint8_t *data,uint8_t len)
{
    wait_busy_low();
    write_buf[0] = 0x0D;
    write_buf[1] = ((add & 0xff00) >> 8);
    write_buf[2] = (add & 0xff);
    for(int i =0;i<len;i++){
        write_buf[i+3] = data[i];
    }
    ql_spi_write_read(QL_SPI_PORT1,read_buf,write_buf,len+3);
    return wait_busy_low();
}

int lora_read_register(uint16_t add, uint8_t len)
{
    wait_busy_low();
    write_buf[0] = 0x1D;
    write_buf[1] = ((add & 0xff00) >> 8);
    write_buf[2] = (add & 0xff);
    for(int i = 0; i <= len; i++){
        write_buf[i+1+len] = 0;
    }
    ql_spi_write_read(QL_SPI_PORT1,read_buf,write_buf,len+4);
    
    return wait_busy_low();
}

int lora_write_cmd(uint8_t cmd, uint8_t *data, uint8_t len)
{
    if(wait_busy_low() == 0) return 0;
    
    write_buf[0] = cmd;
    for(int i = 0; i < len; i++){
        write_buf[i+1] = data[i];
    }
    spi_err = ql_spi_write_read(QL_SPI_PORT1,read_buf,write_buf,len+1);
    if(spi_err !=0) return 0;

    //if(wait_busy_low() == 0) return 0;
    return wait_busy_low();
}

int lora_read_cmd(uint8_t cmd, uint8_t len)
{
    wait_busy_low();
    write_buf[0] = cmd;
    for(int i = 0; i < len; i++){
        write_buf[i+1] = 0;
    }
    ql_spi_write_read(QL_SPI_PORT1,read_buf,write_buf,len+1);
    return wait_busy_low();
}

void lora_write_buffer(uint8_t *data, uint8_t len)
{
    wait_busy_low();
    write_buf[0] = 0x0E;
    write_buf[1] = 0x00;
    for(int i = 0; i < len; i++){
        write_buf[i+2] = data[i];
    }
    ql_spi_write_read(QL_SPI_PORT1,read_buf,write_buf,len+2);
    wait_busy_low();
}

void lora_read_buffer(uint8_t offset, uint8_t len)
{
    wait_busy_low();
    write_buf[0] = 0x1E;
    write_buf[1] = offset;
    for(int i = 0; i < len; i++){
        write_buf[i+2] = 0;
    }
    ql_spi_write_read(QL_SPI_PORT1,read_buf,write_buf,len+2);
    wait_busy_low();
}

void transfer_buf_init(void)
{
    write_mal_buf = (unsigned char *)malloc(QL_SPI_DMA_ADDR_ALIN+write_len);
    read_mal_buf = (unsigned char *)malloc(QL_SPI_DMA_ADDR_ALIN+read_len);

    //32对齐
    write_buf = (unsigned char *)OSI_ALIGN_UP(write_mal_buf, QL_SPI_DMA_ADDR_ALIN);
    read_buf = (unsigned char *)OSI_ALIGN_UP(read_mal_buf, QL_SPI_DMA_ADDR_ALIN);

    //清零
    memset(write_buf, 0x00, write_len);
    memset(read_buf, 0x00, read_len);
}

void lora_reset(void)
{
    ql_gpio_set_level(SPI_RST_IO,LVL_LOW);
    ql_delay_us(1000);
    ql_gpio_set_level(SPI_RST_IO,LVL_HIGH);
    wait_busy_low();
}

int lora_spi_init(void)
{
    ql_spi_config_s lora_spi_config = {0};
    lora_spi_config.input_mode =        QL_SPI_INPUT_TRUE;
    lora_spi_config.port =              QL_SPI_PORT1;
    lora_spi_config.framesize =         8;
    lora_spi_config.spiclk =            QL_SPI_CLK_6_25MHZ;
    lora_spi_config.cs_polarity0 =      QL_SPI_CS_ACTIVE_LOW;
    lora_spi_config.cs_polarity1 =      QL_SPI_CS_ACTIVE_LOW;
    lora_spi_config.cpol =              QL_SPI_CPOL_LOW;
    lora_spi_config.cpha =              QL_SPI_CPHA_2Edge;
    lora_spi_config.input_sel =         QL_SPI_DI_1;
    lora_spi_config.transmode =         QL_SPI_DMA_IRQ;
    lora_spi_config.cs =                QL_SPI_CS0;
    
    lora_spi_config.clk_delay =         QL_SPI_CLK_DELAY_0;
    lora_spi_config.release_flag =      QL_SPI_NOT_RELEASE;

    spi_err = ql_spi_init_ext(lora_spi_config);         if(spi_err != 0) return 2;
    spi_err = ql_spi_cs_auto(QL_SPI_PORT1);             if(spi_err != 0) return 3;
    spi_err = ql_spi_request_sys_clk(QL_SPI_PORT1);     if(spi_err != 0) return 4;

    return 1;
}

int lora_gpio_init(void)
{
    /*将引脚设置为spi功能*/
    gpio_err = ql_pin_set_func(SPI_CS_PIN,1);                       if(gpio_err != 0) return 2;
    gpio_err = ql_pin_set_func(SPI_CLK_PIN,1);                      if(gpio_err != 0) return 3;
    gpio_err = ql_pin_set_func(SPI_MOSI_PIN,1);                     if(gpio_err != 0) return 4;
    gpio_err = ql_pin_set_func(SPI_MISO_PIN,1);                     if(gpio_err != 0) return 5;

    /*将busy，int，rst引脚设置为gpio功能*/
    gpio_err = ql_pin_set_gpio(SPI_BUSY_PIN);                       if(gpio_err != 0) return 6;
    gpio_err = ql_pin_set_gpio(SPI_RST_PIN);                        if(gpio_err != 0) return 7;
    gpio_err = ql_pin_set_gpio(SPI_INT_PIN);                        if(gpio_err != 0) return 8;

    /*将busy引脚同一io口的引脚设置为非gpio功能，防止冲突*/
    gpio_err = ql_pin_set_func(66,0);                               if(gpio_err != 0) return 9;

    /*设置busy，int，rst的gpio*/
    gpio_err = ql_gpio_set_direction(SPI_BUSY_IO,GPIO_INPUT);       if(gpio_err != 0) return 10;
    gpio_err = ql_gpio_set_direction(SPI_INT_IO,GPIO_INPUT);        if(gpio_err != 0) return 11;
    gpio_err = ql_gpio_set_direction(SPI_RST_IO,GPIO_OUTPUT);       if(gpio_err != 0) return 12;
    gpio_err = ql_gpio_set_level(SPI_RST_IO,LVL_HIGH);              if(gpio_err != 0) return 13;

    /*拉低电平转换器开关电平，使能B端电压*/
    gpio_err = ql_pin_set_gpio(16);                                 if(gpio_err != 0) return 14;
    gpio_err = ql_gpio_set_direction(GPIO_19,GPIO_OUTPUT);          if(gpio_err != 0) return 15;
    gpio_err = ql_gpio_set_level(GPIO_19,LVL_LOW);                  if(gpio_err != 0) return 16;


    /*将miso口设置为输入，其他口设置为输出*/
    // gpio_err = ql_gpio_set_direction(SPI_CS_IO,GPIO_OUTPUT);        if(gpio_err != 0) return 14;
    // gpio_err = ql_gpio_set_direction(SPI_CLK_IO,GPIO_OUTPUT);       if(gpio_err != 0) return 15;
    // gpio_err = ql_gpio_set_direction(SPI_MOSI_IO,GPIO_OUTPUT);      if(gpio_err != 0) return 16;
    // gpio_err = ql_gpio_set_direction(SPI_MISO_IO,GPIO_INPUT);       if(gpio_err != 0) return 17;

    return 1;
}