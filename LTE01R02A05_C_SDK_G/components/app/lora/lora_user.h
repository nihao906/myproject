#include <stdlib.h>
#include <unistd.h>
#include "ql_gpio.h"

#define     SPI_1_CLK       GPIO_9
#define     SPI_1_nCS       GPIO_10
#define     SPI_1_MOSI      GPIO_11
#define     SPI_1_MISO      GPIO_12
#define     SPI_2_nCS       GPIO_1
#define     SPI_2_MOSI      GPIO_2
#define     SPI_2_MISO      GPIO_3
#define     SPI_2_CLK       GPIO_4


void lora_init(void);
void lora_send(void);
void lora_receive(void);
void lora_test(void);
void ExampleLLCC68ReciveDemo(void);