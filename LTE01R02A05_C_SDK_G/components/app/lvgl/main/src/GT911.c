#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "GT911.h"
#include "ql_log.h"
#include "ql_gpio.h"
#include "ql_i2c.h"
#include "osi_api.h"

#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "GT911", msg, ##__VA_ARGS__)
#define nTP_INT         GPIO_5      //pin:62
#define TP_nRST         GPIO_4      //pin:137
#define TP_I2C_SDA      56
#define TP_I2C_SCL      57
#define I2C_CHANNEL     i2c_2

user_touch_struct user_touch_info = {0};
static uint8_t GT911_Config[] = {
		0x81, 0x00, 0x04, 0x58, 0x02, 0x0A, 0x0C, 0x20, 0x01, 0x08, 0x28, 0x05, 0x50, // 0x8047 - 0x8053
		0x3C, 0x0F, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8054 - 0x8060
		0x00, 0x89, 0x2A, 0x0B, 0x2D, 0x2B, 0x0F, 0x0A, 0x00, 0x00, 0x01, 0xA9, 0x03, // 0x8061 - 0x806D
		0x2D, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, // 0x806E - 0x807A
		0x59, 0x94, 0xC5, 0x02, 0x07, 0x00, 0x00, 0x04, 0x93, 0x24, 0x00, 0x7D, 0x2C, // 0x807B - 0x8087
		0x00, 0x6B, 0x36, 0x00, 0x5D, 0x42, 0x00, 0x53, 0x50, 0x00, 0x53, 0x00, 0x00, // 0x8088	- 0x8094
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x8095 - 0x80A1
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80A2 - 0x80AD
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, // 0x80AE - 0x80BA
		0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // 0x80BB - 0x80C7
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80C8 - 0x80D4
		0x02, 0x04, 0x06, 0x08, 0x0A, 0x0F, 0x10, 0x12, 0x16, 0x18, 0x1C, 0x1D, 0x1E, // 0x80D5 - 0x80E1
		0x1F, 0x20, 0x21, 0x22, 0x24, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, // 0x80E2 - 0x80EE
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80EF - 0x80FB
		0x00, 0x00}; // 0x80FC - 0x80FD

/*函数*/
ql_errcode_i2c_e GT911_Reg_Write(uint16_t reg_address, uint8_t *tx_buf, uint32_t tx_length)
{
    return ql_I2cWrite_16bit_addr(I2C_CHANNEL, GT911_ADDRESS, reg_address, tx_buf, tx_length);
}

ql_errcode_i2c_e GT911_Reg_Read(uint16_t reg_address, uint8_t *rx_buf, uint32_t rx_length)
{
    return ql_I2cRead_16bit_addr(I2C_CHANNEL, GT911_ADDRESS, reg_address, rx_buf, rx_length);
}

/*软件复位，1代表开始复位，0代表结束复位*/
/*
命令寄存器（0x8040），0：读坐标状态 1：差值原始值 2：软件复位
3：基准更新（内部测试） 4：基准校准（内部测试） 5:关屏
*/
ql_errcode_i2c_e GT911_software_reset(int reset_type)//QL_I2C_WRITE_ERR
{
    uint16_t reset_cmd = 0;
    if(reset_type){
        reset_cmd = 2;
    }else{
        reset_cmd = 0;    
    }
    return GT911_Reg_Write(GT911_CTRL_ADDRESS,&reset_cmd,2);
}

/*配置GT911从机地址*/
int GT911_set_address(void)
{
    LOGI("enter GT911_set_address");
    ql_errcode_gpio err = 0;

    /*设置引脚功能*/
    ql_pin_set_func(62,1);
    ql_pin_set_func(137,1);
    err = ql_gpio_init(nTP_INT,GPIO_OUTPUT,PULL_UP,LVL_LOW); 
    if(err != 0){
        LOGI("ql_gpio_init nTP_INT failed, err = %d\n",err);
        return 0;
    }

    err = ql_gpio_init(TP_nRST,GPIO_OUTPUT,PULL_UP,LVL_LOW);
    if(err != 0){
        LOGI("ql_gpio_init TP_nRST failed, err = %d\n",err);
        return 0;
    }

    /*退出睡眠模式*/
    ql_gpio_set_level(GPIO_5,1);
    osiThreadSleep(5);
    ql_gpio_set_level(GPIO_5,0);
    osiThreadSleep(5);

    /*设置从机地址为0xBA*/
    ql_gpio_set_level(GPIO_4,0);
    ql_gpio_set_level(GPIO_5,0); 
    ql_gpio_set_level(GPIO_4,1);
    osiThreadSleep(20);

    /*设置INT引脚为浮空输入低电平*/
    ql_gpio_init(GPIO_5,GPIO_INPUT,PULL_NONE,LVL_LOW);

    LOGI("GT911_set_address end\n");
    return 1;
}

/*GT911触摸屏初始化*/
int GT911_init(GT911_Config_t config)
{
    ql_errcode_i2c_e err;

    //配置数组
	GT911_Config[1] = config.X_Resolution & 0x00FF;
	GT911_Config[2] = (config.X_Resolution >> 8) & 0x00FF;
	GT911_Config[3] = config.Y_Resolution & 0x00FF;
	GT911_Config[4] = (config.Y_Resolution >> 8) & 0x00FF;
	GT911_Config[5] = config.Number_Of_Touch_Support;
	GT911_Config[6] = 0;
	GT911_Config[6] |= config.ReverseY << 7;
	GT911_Config[6] |= config.ReverseX << 6;
	GT911_Config[6] |= config.SwithX2Y << 3;
	GT911_Config[6] |= config.SoftwareNoiseReduction << 2;

    //计算校验和
    uint8_t buf[2] = {0,0};
    for(int i = 0; i < sizeof(GT911_Config); i++){
        buf[0] += GT911_Config[i];
    }
    buf[0] = (~buf[0]) + 1;

    //初始化i2c
    err = ql_I2cInit(I2C_CHANNEL,STANDARD_MODE);
    if(err != 0){
        LOGI("ql_I2cInit failed, err = %d\n",err);
        return 0;
    }else{
        LOGI("ql_I2cInit sucess\n");
    }
    ql_pin_set_func(TP_I2C_SDA,1);
    ql_pin_set_func(TP_I2C_SCL,1);

    //配置GT911从机地址
    int GT911_set_address_flag = GT911_set_address();
    if(GT911_set_address_flag == 1){
        LOGI("GT911_set_address 0xBA sucess\n");
    }else{
        LOGI("GT911_set_address 0xBA failed\n");
        return 0;
    }

    //开始复位
    err = GT911_software_reset(1);
    if(err != 0){
        LOGI("GT911_software_reset true failed, err = %d\n",err);//QL_I2C_WRITE_ERR
        return 0;
    }

    osiThreadSleep(10);

    //写入配置数组
    err = GT911_Reg_Write(GT911_CONFIG_ADDRESS,(uint8_t *)GT911_Config,sizeof(GT911_Config));
    if(err != 0){
        LOGI("write config cmd failed, err = %d\n",err);
        return 0;
    }

    //写入校验和，配置更新标记
    err = GT911_Reg_Write(GT911_CHECK_REG,buf,2);
    if(err != 0){
        LOGI("write check cmd failed, err = %d\n",err);
        return 0;
    }

    //结束复位
    osiThreadSleep(10);
    err = GT911_software_reset(0);
    if(err != 0){
        LOGI("GT911_software_reset false failed, err = %d\n",err);
        return 0;
    }

    return 1;
}

/*获取触摸状态*/
GT911_Touch_State GT911_is_touched(void)
{
    uint8_t state;
    GT911_Reg_Read(GT911_STATE_REG,&state,1);
    user_touch_info.touch_state = state;
    user_touch_info.touch_number = (user_touch_info.touch_state & 0x0f);
    user_touch_info.touch_state = (user_touch_info.touch_state & 0x80);

    state = 0;
    GT911_Reg_Write(GT911_STATE_REG,&state,1);
    if(user_touch_info.touch_state == 0x80){
        return IS_TOUCH;
    }else if(user_touch_info.touch_state == 0x00){
        return NO_TOUCH;
    }else{
        return VALUE_ERR;
    }
}

/*获取触摸坐标*/
int GT911_get_XY(int16_t * x, int16_t *y)
{
    uint8_t temp;
    ql_errcode_i2c_e err = 0;
    for(int i = 0; i < user_touch_info.touch_number; i++){

        //x
        err = GT911_Reg_Read((GT911_TPD_Sta + i*8 + X_L), &temp, 1);
        if(err != 0){
            LOGI("read the %d x_low point failed, err = %d\n",i,err);
            return 0;
        }
        user_touch_info.coordinate_info[i].x_point = temp;
        err = GT911_Reg_Read((GT911_TPD_Sta + i*8 + X_H), &temp, 1);
        if(err != 0){
            LOGI("read the %d x_high point failed, err = %d\n",i,err);
            return 0;
        }	
		user_touch_info.coordinate_info[i].x_point |= (temp<<8);

        //y
        err = GT911_Reg_Read((GT911_TPD_Sta + i*8 + Y_L), &temp, 1);
        if(err != 0){
            LOGI("read the %d y_low point failed, err = %d\n",i,err);
            return 0;
        }
        user_touch_info.coordinate_info[i].y_point = temp;
        err = GT911_Reg_Read((GT911_TPD_Sta + i*8 + Y_L), &temp, 1);	
        if(err != 0){
            LOGI("read the %d y_high point failed, err = %d\n",i,err);
            return 0;
        }
		user_touch_info.coordinate_info[i].y_point |= (temp<<8);

        //size
        err = GT911_Reg_Read((GT911_TPD_Sta + i*8 + S_L), &temp, 1);
        if(err != 0){
            LOGI("read the %d size_low point failed, err = %d\n",i,err);
            return 0;
        }
        user_touch_info.coordinate_info[i].size = temp;
        err = GT911_Reg_Read((GT911_TPD_Sta + i*8 + S_L), &temp, 1);	
        if(err != 0){
            LOGI("read the %d size_high point failed, err = %d\n",i,err);
            return 0;
        }
		user_touch_info.coordinate_info[i].size |= (temp<<8);
    }
    *x = user_touch_info.coordinate_info[0].x_point;
    *y = user_touch_info.coordinate_info[0].y_point;
    LOGI("x = %d, y = %d \n", *x, *y);
    return 1;
}