#include "dht11.h"

//GPIO_InitTypeDef  GPIO_InitStructure;

void dht11_inputmode(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	/* Configure PA1 in input pushpull mode */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);							
}

void dht11_outputmode(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	/* Configure PA1 in output pushpull mode */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);	
}

//发送开始信号，并接收DHT11的相应信号
//该函数的出错代码为负数，而且每一个阶段的出错代码都不一样
int32_t dht11_start(void)
{
	//t是超时信号，目的在于防止我们DHT11启动失败会卡死我们的程序
	uint32_t t=0;
	
	//配置数据引脚为输出模式
	dht11_outputmode();
	
	//发送开始信号
	PAout(1) = 0;	
	delay_ms(20);	//延时至少18ms
	
	//拉高
	PAout(1) = 1;
	delay_us(30);	//拉高20~40us
	
	//配置数据引脚为输入模式
	dht11_inputmode();
	
	//等待dht11拉低数据引脚
	t=0;
	while(PAin(1) == 1)
	{
		delay_us(1);
		t++;
		
		//超时处理，最多等待1ms
		if(t > 1000)
			return 1;	//返回错误代码-1
	}
	
	//等待低电平持续完毕
	t=0;
	while(PAin(1) == 0)
	{
		delay_us(1);
		t++;
		
		//超时处理，最多等待150us
		if(t > 150)
			return 2;
	}	
	
	//等待高电平持续完毕
	t=0;
	while(PAin(1) == 1)
	{
		delay_us(1);
		t++;
		
		//超时处理，最多等待150us
		if(t>150)
			return 3;
	}			
	
	return 0;	//成功返回0，失败返回相应的错误代码
}

//接收DHT11中的一字节数据
uint8_t dht11_read_byte(void)
{
	uint32_t i=0;
	uint8_t data=0; //0x00
	
	//接收一字节数据
	for(i=0; i<8; i++)
	{
		//等待低电平持续完毕
		while(PAin(1) == 0);	//没有进行超时处理，程序是由几率在这卡死的
		delay_us(40);//延时40us
		
		
		//判断当前PA15引脚电平的状态，判断出是数据0和数据1
		if(PAin(1) == 1)
		{
			//先传输高位数据
			data |= 1 << (7-i);
			
			//等待高电平持续完毕
			while(PAin(1) == 1);
		}
	}	
	return data;
}

//读取DHT11的温湿度数据
int32_t dht11_read_data(uint8_t *pbuf) //uint8_t *pbuf = uchar pbuf[5]
{
	int32_t rt=5;	//接收函数返回值，即错误代码或成功0	
	uint32_t i=0;
	uint8_t check_sum=0;	//计算数据校验和
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	//发送开始信号，并检查dht11是否有响应
	rt = dht11_start();
	if(rt > 0)	//rt小于0，接收响应信号出现错误，返回错误代码
		return rt;
	
	//读取5个字节  8bit湿度整数数据+8bit湿度小数数据+8bi温度整数数据+8bit温度小数数据
	for(i=0; i<5; i++)
	{
		pbuf[i] = dht11_read_byte();//pbuf[4]为数据校验和
	}
	
	//判断所得到的温度和数据是否是正确
	//计算校验和
	check_sum = pbuf[0]+pbuf[1]+pbuf[2]+pbuf[3];
	if(check_sum != pbuf[4])	//接收数据出现异常
		return 4;

//	//忽略结束信号
	dht11_outputmode();
	PAout(1) = 1;	
	return 0;
}
