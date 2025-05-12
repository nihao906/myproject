#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TP_nRST_pin (137)
#define nTP_INT_pin (62)

#define TP_nRST_GPIO_num            GPIO_4
#define nTP_INT_GPIO_num            GPIO_5

#define X_RESOLUTION                480
#define Y_RESOLUTION                320
#define MAX_TOUCH_NUMBER 			1

#define GT911_ADDRESS	  		    0x5D                    //从机地址
#define GT911_REG_WRITE_ADDRESS     0xBA     				//从机写地址
#define GT911_REG_READ_ADDRESS      0xBB			     	//从机读地址


#define GT911_CTRL_ADDRESS          0x8040                  //控制寄存器地址
#define GT911_CFG_ADDRESS           0x8047                  //配置寄存器起始地址
#define GT911_CHECK_REG 	        0X80FF   	            //校验和寄存器地址
#define GT911_TPD_Sta		        0X8150		            //触摸点起始数据地址
#define GT911_STATE_REG 			0X814E   				//触摸状态寄存器, 第7位是触摸标志位，低4位是触摸点数个数

#define GT911_TP1_REG 		        0X8150  	            //第一个触摸点数据地址
#define GT911_TP2_REG 		        0X8158		            //第二个触摸点数据地址
#define GT911_TP3_REG 		        0X8160		            //第三个触摸点数据地址
#define GT911_TP4_REG 		        0X8168		            //第四个触摸点数据地址
#define GT911_TP5_REG 		        0X8170		            //第五个触摸点数据地址

typedef struct{
	uint16_t X_Resolution;
	uint16_t Y_Resolution;
	uint8_t Number_Of_Touch_Support;
	int ReverseX;
	int ReverseY;
	int SwithX2Y;
	int SoftwareNoiseReduction;
}gt911_t;

void lvgl_demo_init(void);