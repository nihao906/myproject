
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


/*获取同步字*/
uint16_t lora_get_syncword(void);

/*设置standby模式*/
void lora_set_standby(uint8_t data);

/*lora模块初始化函数*/
int lora_init(void);

/*启动lora通讯*/
int lora_start(void);

/*设置radio参数*/
int lora_radioConfig(void);

/*获取中断状态*/
uint16_t lora_get_irq_status(void);

/*清除中断状态*/
void lora_clear_irq_status(uint16_t irq_status);