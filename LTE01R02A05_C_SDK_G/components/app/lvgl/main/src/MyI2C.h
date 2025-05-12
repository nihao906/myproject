#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "ql_gpio.h"

/*初始化*/
int i2c_init(void);


/*起始*/
int i2c_start(void);


/*end*/
int i2c_stop(void);


/*send*/
int i2c_send_byte(uint8_t byte);


/*receive*/
uint8_t i2c_receive_byte(void);

/*send_ack*/
void i2c_send_ack(ql_LvlMode ack);

/*receive ack*/
ql_LvlMode i2c_receive_ack(void);
