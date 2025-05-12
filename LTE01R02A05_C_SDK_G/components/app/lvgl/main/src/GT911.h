/*
头文件
*/
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

/*
宏定义
*/
#define GT911_ADDRESS	  		    0x5D                    //从机地址
#define GT911_REG_WRITE_ADDRESS     0xBA     				//从机写地址
#define GT911_REG_READ_ADDRESS      0xBB			     	//从机读地址


#define GT911_CTRL_ADDRESS          0x8040                  //控制寄存器地址
#define GT911_CONFIG_ADDRESS        0x8047                  //配置寄存器起始地址
#define GT911_CHECK_REG 	        0X80FF   	            //校验和寄存器地址
#define GT911_TPD_Sta		        0X8150		            //触摸点起始数据地址
#define GT911_STATE_REG 			0X814E   				//触摸状态寄存器, 第7位是触摸标志位，低4位是触摸点数个数

#define GT911_TP1_REG 		        0X8150  	            //第一个触摸点数据地址
#define GT911_TP2_REG 		        0X8158		            //第二个触摸点数据地址
#define GT911_TP3_REG 		        0X8160		            //第三个触摸点数据地址
#define GT911_TP4_REG 		        0X8168		            //第四个触摸点数据地址
#define GT911_TP5_REG 		        0X8170		            //第五个触摸点数据地址

#define MAX_TOUCH_NUMBER 5

typedef struct {
    uint16_t x_point;
    uint16_t y_point;
    uint16_t size;
}touch_coordinates_info;

typedef struct{
    uint8_t touch_state;
    uint8_t touch_number;
    touch_coordinates_info coordinate_info[MAX_TOUCH_NUMBER];
}user_touch_struct;

typedef enum{
    IS_TOUCH,
    NO_TOUCH,
    VALUE_ERR
}GT911_Touch_State;

typedef struct{
	uint16_t X_Resolution;
	uint16_t Y_Resolution;
	uint8_t Number_Of_Touch_Support;
	int ReverseX;
	int ReverseY;
	int SwithX2Y;
	int SoftwareNoiseReduction;
}GT911_Config_t;

typedef enum
{
	X_L = 0,
	X_H = 1,
	Y_L = 2,
	Y_H = 3,
	S_L	= 4,
	S_H = 5
}Data_XYS_P;	//数据X、Y、触摸大小数据偏移量

int GT911_init(GT911_Config_t config);
GT911_Touch_State GT911_is_touched(void);
int GT911_get_XY(int16_t * x, int16_t *y);