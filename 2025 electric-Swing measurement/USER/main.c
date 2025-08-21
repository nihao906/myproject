#include "stm32f10x.h"  // ���� STM32 ��׼�����ͷ�ļ�
#include "stm32f10x_tim.h"
#include "stm32f10x_exti.h"
#include "delay.h"
#include "BlueTooth.h"
#include "jy61p.h"
#include "usart2.h"
#include "uart.h"
#include "math.h"
#include "string.h"
#include "stdio.h"

#define RX_BUFFER_SIZE 100
char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_index = 0;

int buffer_to_int(const volatile char *buffer);
void float_to_string(float number, char *buffer, int decimal_places);

int main(void) 
{
	u32 num;
	
	char Initial_Roll1[32];
	char Initial_Pitch1[32];
	char Initial_Yaw1[32];
	
	float Initial_Roll;
	float Initial_Pitch;
	float Initial_Yaw;
	
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
	usart2_Init(9600);/*串口2波特率设置，与陀螺仪一致*/
	
	USART1_Init();  
	USART1_SendString("Hello from STM32 via Bluetooth!\r\n");
	USART1_SendString("你好世界\r\n");
	
	while (1)
    {
		num = buffer_to_int(rx_buffer);
		rx_index = 0;
		memset(rx_buffer, 0, sizeof(rx_buffer));
		
		switch(num)
		{
			case 1:
				USART1_SendString("蓝牙已连接\r\n");
				USART1_SendString("开始任务\r\n");
				break;
			case 2:
				//校准
				Initial_Roll = Roll;
				Initial_Pitch = Pitch;
				Initial_Yaw = Yaw;
			
				float_to_string(Initial_Roll, Initial_Roll1, 3);
				float_to_string(Initial_Pitch, Initial_Pitch1, 3);
				float_to_string(Initial_Yaw, Initial_Yaw1, 3);
				USART1_SendString(Initial_Roll1);
				USART1_SendString(Initial_Pitch1);
				USART1_SendString(Initial_Yaw1);
				USART1_SendString("启动基础题2\n");
//				while(1)
//				{
//					D=Cx*1000+Cy*100+Cz*10+Cw;
//					if( (Roll > 77 && Roll < 83))
//					{
//						LED_ON;
//						Delay_ms(500);
//						LED_OFF;
//						l = (70-D/10.0-11.52);
//						USART1_SendInt(l);
//						USART1_SendString("cm");
//					}
//					num = buffer_to_int(rx_buffer);
//					rx_index = 0;
//					memset(rx_buffer, 0, sizeof(rx_buffer));
//					if(num == 21)
//					{
//						USART1_SendString("退出基础题2\n");
//						break;
//					}
//				}
				break;
			}
    }

}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		char received = USART_ReceiveData(USART1);  

		if (rx_index < RX_BUFFER_SIZE - 1)
		{
			rx_buffer[rx_index++] = received;

			if (received == '\n') {
				rx_buffer[rx_index] = '\0';  
				rx_index = 0;                
			}
		}
		else
		{
			rx_index = 0;  
		}

		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

int buffer_to_int(const volatile char *buffer) {
    int result = 0;
    int sign = 1;
    int i = 0;

    // 跳过空格
    while (buffer[i] == ' ') i++;

    // 检测正负号
    if (buffer[i] == '-') {
        sign = -1;
        i++;
    } else if (buffer[i] == '+') {
        i++;
    }

    // 逐字符转换
    while (buffer[i] >= '0' && buffer[i] <= '9') {
        result = result * 10 + (buffer[i] - '0');
        i++;
    }

    return sign * result;
}

void float_to_string(float number, char *buffer, int decimal_places)
{
    int int_part = (int)number; // 整数部分
    float fraction = number - int_part;

    // 处理负数
    if (number < 0)
    {
        *buffer++ = '-';
        number = -number;
        int_part = (int)number;
        fraction = number - int_part;
    }

    // 整数部分转换
    char temp[20];
    int i = 0;
    do {
        temp[i++] = (int_part % 10) + '0';
        int_part /= 10;
    } while (int_part > 0);

    // 反转整数部分写入 buffer
    while (i > 0) {
        *buffer++ = temp[--i];
    }

    // 小数部分处理
    if (decimal_places > 0)
    {
        *buffer++ = '.';

        for (int j = 0; j < decimal_places; j++)
        {
            fraction *= 10;
            int digit = (int)fraction;
            *buffer++ = digit + '0';
            fraction -= digit;
        }
    }

    *buffer = '\0';
}

