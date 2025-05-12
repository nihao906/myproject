/*软件模拟I2C*/
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "ql_gpio.h"
#include "ql_log.h"
//#include "osi_api.h"
#include "MyI2C.h"

#define     I2C_SCL     GPIO_14
#define     I2C_SDA     GPIO_15
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "MyI2C", msg, ##__VA_ARGS__)

ql_errcode_gpio err = 0;


void i2c_delay(void)
{
    int32_t i = 10000;
    while(i--){

    }
}

ql_errcode_gpio i2c_SCL_set_level(ql_LvlMode value)
{
    err = ql_gpio_set_level(I2C_SCL,value);
    i2c_delay();
    return err;
}

ql_errcode_gpio i2c_SDA_set_level(ql_LvlMode value)
{
    err = ql_gpio_set_level(I2C_SDA,value);
    i2c_delay();
    return err;
}

ql_LvlMode i2c_SCL_get_level(void)
{
    ql_LvlMode bit;
    ql_gpio_get_level(I2C_SCL,&bit);
    return bit;
}

ql_LvlMode i2c_SDA_get_level(void)
{
    ql_LvlMode bit;
    ql_gpio_get_level(I2C_SDA,&bit);
    return bit;
}

/*初始化*/
int i2c_init(void)
{
    //SCL和SDA配置为开漏输出高电平
    err = ql_pin_set_func(56,0);
    if(err != 0){
        //LOGI("ql_pin_set_func I2C_SCL failed, err = %d \n",err);
        return 0;
    }
    err = ql_pin_set_func(57,0);
    if(err != 0){
        //LOGI("ql_pin_set_func I2C_SDA failed, err = %d \n",err);
        return 0;
    }
    err = ql_gpio_init(I2C_SCL,GPIO_OUTPUT,PULL_NONE,LVL_HIGH);
    if(err != 0){
        //LOGI("ql_gpio_init I2C_SCL failed, err = %d \n",err);
        return 0;
    }
    err = ql_gpio_init(I2C_SDA,GPIO_OUTPUT,PULL_NONE,LVL_HIGH);
    if(err != 0){
        //LOGI("ql_gpio_init I2C_SDA failed, err = %d \n",err);
        return 0;
    }
    return 1;
}

/*起始*/
int i2c_start(void)
{
    err = i2c_SDA_set_level(LVL_HIGH);
    if(err != 0){
        //LOGI("i2c_SDA_set_level LVL_HIGH failed, err = %d \n",err);
        return 0;
    }
    err = i2c_SCL_set_level(LVL_HIGH);
    if(err != 0){
        //LOGI("i2c_SCL_set_level LVL_HIGH failed, err = %d \n",err);
        return 0;
    }
    err = i2c_SDA_set_level(LVL_LOW);
    if(err != 0){
        //LOGI("i2c_SDA_set_level LVL_LOW failed, err = %d \n",err);
        return 0;
    }
    err = i2c_SCL_set_level(LVL_LOW);
    if(err != 0){
        //LOGI("i2c_SCL_set_level LVL_LOW failed, err = %d \n",err);
        return 0;
    }
    return 1;
}

/*end*/
int i2c_stop(void)
{
    err = i2c_SDA_set_level(LVL_LOW);
    if(err != 0){
        return 0;
    }
    err = i2c_SCL_set_level(LVL_HIGH);
    if(err != 0){
        return 0;
    }
    err = i2c_SDA_set_level(LVL_HIGH);
    if(err != 0){
        return 0;
    }
    return 1;
}

/*send*/
int i2c_send_byte(uint8_t byte)//10111010
{

    for(int i =0; i< 8; i++){
        if((byte & (0x80 >> i)) == 0){
            if(i2c_SDA_set_level(LVL_LOW)!=0){
                LOGI("i2c_SDA_set_level LVL_LOW failed, err = %d \n",err);
                return 0;
            }
        }else{
            if(i2c_SDA_set_level(LVL_HIGH)!=0){
                LOGI("i2c_SDA_set_level LVL_HIGH failed, err = %d \n",err);
                return 0;
            }
        }
        LOGI("SDA send: %d\n",i2c_SDA_get_level());

        if (i2c_SCL_set_level(LVL_HIGH)!=0){
            LOGI("i2c_SCL_set_level LVL_HIGH failed, err = %d \n",err);
            return 0;
        }
        LOGI("SCL send: %d\n",i2c_SCL_get_level());

        if(i2c_SCL_set_level(LVL_LOW)!=0){
            LOGI("i2c_SCL_set_level LVL_LOW failed, err = %d \n",err);
            return 0;
        }
        LOGI("SCL send: %d\n",i2c_SCL_get_level());
    } 

    /*主机释放SDA，从机把应答位放在SDA上*/
    err = i2c_SDA_set_level(LVL_HIGH);
    if(err!=0) LOGI("i2c_SDA_set_level LVL_HIGH failed, err = %d \n",err);

    return 1;  
}

/*receive*/
uint8_t i2c_receive_byte(void)
{
    uint8_t byte = 0x00;
    for(int i =0; i< 8; i++){
        i2c_SDA_set_level(LVL_HIGH);
        i2c_SCL_set_level(LVL_HIGH);
        if(i2c_SDA_get_level() == LVL_HIGH){
            byte |= (0x80 >> i);
        }
        i2c_SCL_set_level(LVL_LOW);
    }
    return byte;
}

/*send_ack or noack*/
void i2c_send_ack(ql_LvlMode ack)
{
    i2c_SDA_set_level(ack);
    i2c_SCL_set_level(LVL_HIGH);
    i2c_SCL_set_level(LVL_LOW);

    /*主机释放SDA，从机把应答位放在SDA上*/
    if(ack == 0){
        err = i2c_SDA_set_level(LVL_HIGH);
        if(err!=0) LOGI("i2c_SDA_set_level LVL_HIGH failed, err = %d \n",err);
    }   
}

/*receive ack*/
ql_LvlMode i2c_receive_ack(void)
{
    /*SCL低电平*/
    ql_LvlMode ack = 2;

    i2c_SDA_set_level(LVL_HIGH);
    //LOGI("SDA receive: %d\n",i2c_SDA_get_level());
    
    /*SDA设置为输入*/
    err = ql_gpio_set_direction(I2C_SDA,GPIO_INPUT);
    if(err != 0) LOGI("ql_gpio_set_direction input fail\n");

    /*从机改变SDA电平*/
    i2c_delay();

    /*SCL高电平，主机读取应答位*/
    err = i2c_SCL_set_level(LVL_HIGH);
    if(err!=0) LOGI("i2c_SCL_set_level LVL_HIGH failed, err = %d \n",err);
    LOGI("SCL receive: %d\n",i2c_SCL_get_level());

    i2c_delay();

    /*SDA读取电平*/
    ack = i2c_SDA_get_level();
    LOGI("SDA receive: %d\n",i2c_SDA_get_level());

    /*SDA设置为输出*/
    err = ql_gpio_set_direction(I2C_SDA,GPIO_OUTPUT);
    if(err != 0) LOGI("ql_gpio_set_direction output fail\n");

    /*SCL低电平，进入下一个时序单元*/
    err = i2c_SCL_set_level(LVL_LOW);
    if(err!=0) LOGI("i2c_SCL_set_level LVL_LOW failed, err = %d \n",err);
    LOGI("SCL receive: %d\n",i2c_SCL_get_level());
    return ack;
}