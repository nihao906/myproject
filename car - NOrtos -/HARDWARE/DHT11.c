#include "stm32f4xx.h"                  // Device header
#include "BITBAND.h"
#include "DELAY.h"
//PG9  DHT11

static void DQ_setmode(GPIOMode_TypeDef mode)//模式设置
{
	//2）GPIO配置
  GPIO_InitTypeDef GPIO_INSTRUCT;
	GPIO_INSTRUCT.GPIO_Mode = mode;
	GPIO_INSTRUCT.GPIO_OType = GPIO_OType_PP;
	GPIO_INSTRUCT.GPIO_Pin = GPIO_Pin_9;
	GPIO_INSTRUCT.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_INSTRUCT.GPIO_Speed = GPIO_Speed_100MHz;
	//3）GPIO初始化
	GPIO_Init(GPIOG,&GPIO_INSTRUCT);
}

void DHT11_init(void)//初始化
{
	//1）使能GPIOG的时钟
 	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG,ENABLE);
	//4）提供GPIO模式切换的功能函数
	DQ_setmode(GPIO_Mode_OUT);
	PGout(9) = 1;
}

//5）提供用于发送开始信号并获取响应的功能函数
static int8_t DHT11_start(void)//开始信号
{
  DQ_setmode(GPIO_Mode_OUT);
	PGout(9) = 0;
	DELAY_ms(20);
	PGout(9) = 1;
	DElAY_us(30);
	
	DQ_setmode(GPIO_Mode_IN);
	
	uint32_t t = 0;
	while(PGin(9) == 1)
	{
	  DElAY_us(1);
		if(++t > 100000)
		{
		  return -1;
		}
	}
	t = 0;
	while(PGin(9) == 0)
	{
	  DElAY_us(1);
		if(++t >90)
		{
		 return -2;
		}
	}
	t = 0;
	while(PGin(9) == 1)
	{
		DElAY_us(1);
		if(++t >90)
		{
		 return -3;
		}
	}
	t = 0;
	return 0;
}	

//6）提供从总线获取1Byte数据的功能函数
static uint8_t DHT11_readbyte(void)//获取一个字节
{
  uint8_t byte = 0;//0000 0000
	uint8_t i = 0;
	for(i = 0;i < 8;i++)
	{
	  while(PGin(9) == 0) ;
		DElAY_us(35);
		if(PGin(9) == 1)
		 byte |= 1<<(7 - i);
		while(PGin(9) == 1);
	}
	return byte;
}

int8_t DHT11_getdata(uint8_t *pbuf)//5个字节
{
	int8_t ret = 0;
	uint8_t i = 0;
	uint8_t cheaksum = 0;
	ret = DHT11_start();// -1、-2、-3
	if(ret < 0)
  return ret;
	for(i = 0;i < 5;i++)
	{
	 pbuf[i] =  DHT11_readbyte();
	}
	
  for(i = 0;i < 4;i++)
	{
	  cheaksum += pbuf[i];
	}
	if(cheaksum != pbuf[4])
	{
	 return -4;
	}
	return 0;
}
