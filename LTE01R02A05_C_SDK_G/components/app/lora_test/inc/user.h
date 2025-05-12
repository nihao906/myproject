#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "ql_gpio.h"


#define     SPI_CLK_PIN     1
#define     SPI_CLK_IO      GPIO_9
#define     SPI_MISO_PIN    2
#define     SPI_MISO_IO     GPIO_12
#define     SPI_MOSI_PIN    3
#define     SPI_MOSI_IO     GPIO_11
#define     SPI_CS_PIN      4
#define     SPI_CS_IO       GPIO_10

#define     SPI_RST_PIN     59                      //spi2-mosi
#define     SPI_RST_IO      GPIO_2
#define     SPI_INT_PIN     60                      //spi2-miso
#define     SPI_INT_IO      GPIO_3
#define     SPI_BUSY_PIN    61                      //spi2-clk
#define     SPI_BUSY_IO     GPIO_0

#define     LORA_SYNCWORD_1   0x3444
#define     LORA_SYNCWORD_2   0x1444

/*cmd*/
#define     LORA_SET_REGULATORMODE  0x96
#define     LORA_SET_RFSWITCHMODE   0x9D
#define     LORA_SET_CALIBRATE      0x89
#define     LORA_SET_BUFFERBASEADDRESS  0x8F
#define     LORA_SET_PACONFIG  0x95
#define     LORA_SET_TX_PARAMS  0x8E
#define     LORA_SET_CALIBRATEIMAGE 0x98
#define     LORA_SET_RFFREQ 0x86
#define     LORA_SET_STOPRXTIMERONPREAMBLE  0x9F
#define     LORA_SET_SYMBNUMTIMEOUT 0xA0
#define     LORA_SET_PACKETTYPE 0x8A
#define     LORA_SET_MODULATIONPARAMS   0x8B
#define     LORA_SET_PACKETPARAMS   0x8C
#define     LORA_SET_DIO_IRQ_CONFIG 0x08
#define     LORA_SET_RX 0x82    //将芯片设置为接收模式
#define     LORA_GET_STATUS 0xC0 //返回设备当前状态
#define     LORA_GET_IRQSTATUS 0x12 //获取中断状态
#define     LORA_CLEAR_IRQSTATUS 0x02 //清除中断状态

/*register add*/
#define     LORA_SET_OCP    0x08E7
#define     LORA_SYNCWORD   0x0740

/*参数宏*/
//CALIBRATE
#define CALIBRATE_IMAGE_OFF                    0b00000000  //  6     6     image calibration: disabled
#define CALIBRATE_IMAGE_ON                     0b01000000  //  6     6                        enabled
#define CALIBRATE_ADC_BULK_P_OFF               0b00000000  //  5     5     ADC bulk P calibration: disabled
#define CALIBRATE_ADC_BULK_P_ON                0b00100000  //  5     5                             enabled
#define CALIBRATE_ADC_BULK_N_OFF               0b00000000  //  4     4     ADC bulk N calibration: disabled
#define CALIBRATE_ADC_BULK_N_ON                0b00010000  //  4     4                             enabled
#define CALIBRATE_ADC_PULSE_OFF                0b00000000  //  3     3     ADC pulse calibration: disabled
#define CALIBRATE_ADC_PULSE_ON                 0b00001000  //  3     3                            enabled
#define CALIBRATE_PLL_OFF                      0b00000000  //  2     2     PLL calibration: disabled
#define CALIBRATE_PLL_ON                       0b00000100  //  2     2                      enabled
#define CALIBRATE_RC13M_OFF                    0b00000000  //  1     1     13 MHz RC osc. calibration: disabled
#define CALIBRATE_RC13M_ON                     0b00000010  //  1     1                                 enabled
#define CALIBRATE_RC64K_OFF                    0b00000000  //  0     0     64 kHz RC osc. calibration: disabled
#define CALIBRATE_RC64K_ON                     0b00000001  //  0     0                                 enabled
//standby
#define     LORA_STANDBY_RC         0x00
#define     LORA_STANDBY_XOSC       0x01
//regulator
#define     REGULATOR_DCDC          0x01
#define     REGULATOR_LDO           0x00
//freq
#define     XTAL_FREQ               (double)(32000000)
#define     FREQ_DIV                (double)(33554432) //2的25次方
#define     FREQ_STEP               (double)(XTAL_FREQ/FREQ_DIV)
#define     RF_FREQ                 (uint32_t)470000000
//irq   
typedef enum
{
    IRQ_RADIO_NONE                          = 0x0000,
    IRQ_TX_DONE                             = 0x0001,
    IRQ_RX_DONE                             = 0x0002,
    IRQ_PREAMBLE_DETECTED                   = 0x0004,
    IRQ_SYNCWORD_VALID                      = 0x0008,
    IRQ_HEADER_VALID                        = 0x0010,
    IRQ_HEADER_ERROR                        = 0x0020,
    IRQ_CRC_ERROR                           = 0x0040,
    IRQ_CAD_DONE                            = 0x0080,
    IRQ_CAD_ACTIVITY_DETECTED               = 0x0100,
    IRQ_RX_TX_TIMEOUT                       = 0x0200,
    IRQ_RADIO_ALL                           = 0xFFFF,
}RadioIrqMasks_t;

/*初始化spi总线*/
int lora_spi_init(void);

/*初始化lora所需的gpio口*/
int lora_gpio_init(void);

/*初始化收发数组内存，进行32字节对齐*/
void transfer_buf_init(void);

/*复位lora模块*/
void lora_reset(void);

/*等待busy被拉低*/
int wait_busy_low(void);

/*写命令*/
int lora_write_cmd(uint8_t cmd, uint8_t *data, uint8_t len);

/*读命令*/
int lora_read_cmd(uint8_t cmd, uint8_t len);

/*写寄存器*/
int lora_wrtie_register(uint16_t add,uint8_t *data,uint8_t len);

/*读寄存器*/
int lora_read_register(uint16_t add, uint8_t size);

/*写buffer*/
void lora_write_buffer(uint8_t *data, uint8_t len);

/*读buffer*/
void lora_read_buffer(uint8_t offset, uint8_t len);