#include "jy61p.h"

static uint8_t RxBuffer[11];/*接收角度数组*/
static volatile uint8_t RxState = 0;/*接收状态标志位*/
static uint8_t RxIndex = 0;/*接受数组索引*/
float Roll,Pitch,Yaw;/*角度信息，如果只需要整数可以改为整数类型*/

static uint8_t RxGyroBuffer[11];/*接收角速度数组*/
static volatile uint8_t RxGyroState = 0;/*接收状态标志位*/
static uint8_t RxGyroIndex = 0;/*接受数组索引*/
float Pitch_gyro;


/**
 * @brief       数据包处理函数
 * @param       串口接收的数据RxData
 * @retval      无
 */
void jy60p_ReceiveData(uint8_t RxData)
{
	uint8_t i,sum=0;
	
	if (RxState == 0)	//等待包头
	{
		if (RxData == 0x55)	//收到包头
		{
			RxBuffer[RxIndex] = RxData;
			RxState = 1;
			RxIndex = 1; //进入下一状态
		}
	}
	
	else if (RxState == 1)
	{
		if (RxData == 0x53)	/*判断数据内容，修改这里可以改变要读的数据内容，0x53为角度输出*/
		{
			RxBuffer[RxIndex] = RxData;
			RxState = 2;
			RxIndex = 2; //进入下一状态
		}
	}
	
	else if (RxState == 2)	//接收数据
	{
		RxBuffer[RxIndex++] = RxData;
		if(RxIndex == 11)	//接收完成
		{
			for(i=0;i<10;i++)
			{
				sum = sum + RxBuffer[i]; //计算校验和
			}
			if(sum == RxBuffer[10])		//校验成功
			{
				/*计算数据，根据数据内容选择对应的计算公式*/
				//Roll = ((int16_t) ((int16_t) RxBuffer[3] << 8 | (int16_t) RxBuffer[2])) / 32768.0f * 180.0f;
				Pitch = ((int16_t) ((int16_t) RxBuffer[5] << 8 | (int16_t) RxBuffer[4])) / 32768.0f * 180.0f;
				//Yaw = ((int16_t) ((int16_t) RxBuffer[7] << 8 | (int16_t) RxBuffer[6])) / 32768.0f * 180.0f;
			}
			RxState = 0;
			RxIndex = 0; //读取完成，回到最初状态，等待包头
		}
	}
}


void jy60p_ReceiveDataGyro(uint8_t RxData)
{
	uint8_t i,sum=0;
	
	if (RxGyroState == 0)	//等待包头
	{
		if (RxData == 0x55)	//收到包头
		{
			RxGyroBuffer[RxGyroIndex] = RxData;
			RxGyroState = 1;
			RxGyroIndex = 1; //进入下一状态
		}
	}
	
	else if (RxGyroState == 1)
	{
		if (RxData == 0x52)	/*判断数据内容，修改这里可以改变要读的数据内容，0x52为角速度输出*/
		{
			RxGyroBuffer[RxGyroIndex] = RxData;
			RxGyroState = 2;
			RxGyroIndex = 2; //进入下一状态
		}
	}
	
	else if (RxGyroState == 2)	//接收数据
	{
		RxGyroBuffer[RxGyroIndex++] = RxData;
		if(RxGyroIndex == 11)	//接收完成
		{
			for(i=0;i<10;i++)
			{
				sum = sum + RxGyroBuffer[i]; //计算校验和
			}
			if(sum == RxGyroBuffer[10])		//校验成功
			{
				/*计算数据，根据数据内容选择对应的计算公式*/
				//Roll = ((int16_t) ((int16_t) RxBuffer[3] << 8 | (int16_t) RxBuffer[2])) / 32768.0f * 180.0f;
				Pitch_gyro = ((int16_t) ((int16_t) RxGyroBuffer[5] << 8 | (int16_t) RxGyroBuffer[4])) / 32768.0f * 180.0f;
				//Yaw = ((int16_t) ((int16_t) RxBuffer[7] << 8 | (int16_t) RxBuffer[6])) / 32768.0f * 180.0f;
			}
			RxGyroState = 0;
			RxGyroIndex = 0; //读取完成，回到最初状态，等待包头
		}
	}
}
