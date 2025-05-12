#include "lvgl.h"

#define THEME_COLOR_BTN_BLUE_DEF 0x2196F3
#define THEME_COLOR_WHITE_DEF 0xFFFFFF

extern const lv_font_t ui_font_16;
extern lv_obj_t * ui_Screen2;

struct Dropdown_t{
    int dropdown_size[2];
    int dropdown_align[2];
    char dropdown_options[64];
    char dropdown_dsc[32];
};

struct Textarea_t{
    int num;
    int label_size[14][2];
    int label_align[14][2];
    char label_text[14][32];
    int textarea_size[14][2];
    int textarea_align[14][2];
    char textarea_default_data[14][10];
};

struct Btn_t{
    lv_obj_t * btn_p[3];
    int btn_align[3][2];
    void (*set_cb)(lv_event_t * e);
    void (*reset_cb)(lv_event_t * e);
    void (*default_cb)(lv_event_t * e);
};

/**********************************************************************************************************/
struct _ch
{
    uint16_t ad_4ma;
	uint16_t ad_20ma;
};

typedef struct _cal_4_20ma_t
{
	uint16_t magic;
	struct _ch ch[2];
}cal_4_20ma_t;

/**********************************************************************************************************/
struct ad_flow_cal_t
{
	int16_t flow_min;
	int16_t flow_max;
};					//存储ad转换的最大流量和最小流量

typedef struct _flow_config_t
{
	uint16_t magic;					//可能是一个用于标识流量配置的特殊字段或标志?
	uint16_t input_type; 			// 1 : 4~20ma 2: 0~3.6K
	int16_t min_flow[2]; 			// 小流量切除
	struct ad_flow_cal_t ad_cal[2];
	uint16_t pulse_coef[2];			//可能代表了脉冲输出的系数或倍率?
	uint16_t rsv[6];				//可能是预留给未来扩展使用的保留字段?
}flow_config_t;

/**********************************************************************************************************/
typedef struct _depth_config_t
{
	uint16_t magic;
	uint8_t input_type;				// 0：正交 1:正交反向 2:方向脉冲 3:方向脉冲反向
	uint8_t port;					// 编码器端口
	uint16_t N;						//编码器系数分子
	uint16_t M;						//编码器系数分母
	int16_t min_depth;				// 最小深度 mm
	int16_t max_depth;				// 最大深度 mm
	int16_t sample_depth;			// 采样深度 mm
	int16_t depth_offset;			// 默认深度偏移
	int16_t min_valid_depth;		// 最小有效深度
	int16_t inc_pile_depth;			// 允许换桩深度
	uint16_t current_on_threshold; 	// 行走电机开启电流
	uint16_t current_off_threshold;	// 行走电机关闭电流
	uint16_t move_on_duration;		// 持续时间
	uint16_t move_off_duration;		// 持续时间
	uint16_t move_current_channel;	//行走电流通道
}depth_config_t;

typedef struct _check_t
{
	uint8_t id;
	uint16_t addr;//该数据在modbus寄存器中的地址
	uint8_t type;//该数据类型
	uint8_t decimal_places;//如果该数据是浮点型有几位小数
	uint16_t max_num;//该数据的最大值
	int16_t min_num;
}check_t;

#define 	TYPE_INT8	0	
#define 	TYPE_UINT8	1
#define 	TYPE_INT16	2
#define 	TYPE_UINT16	3
#define		TYPE_FLOAT	4	//数据类型为浮点型

#define 	SAVE_CMD				(0x55aa)
#define		CAL_4_20MA_ADDR 		(384)
#define		CAL1_AD_4MA_ADDR 		(CAL_4_20MA_ADDR + 1)
#define		CAL1_AD_20MA_ADDR		(CAL_4_20MA_ADDR + 2)
#define		CAL2_AD_4MA_ADDR 		(CAL_4_20MA_ADDR + 3)
#define		CAL2_AD_20MA_ADDR		(CAL_4_20MA_ADDR + 4)

#define 	FLOW_CONFIG_ADDR 			(CAL_4_20MA_ADDR + (sizeof(cal_4_20ma_t) + 15)/16*8) 
#define		FLOW_INPUT_TYPE_ADDR		(FLOW_CONFIG_ADDR + 1)
#define		FLOW_MIN_FLOW1_ADDR			(FLOW_CONFIG_ADDR + 2)
#define		FLOW_MIN_FLOW2_ADDR			(FLOW_CONFIG_ADDR + 3)
#define		FLOW_AD_CAL1_FLOW_MIN_ADDR	(FLOW_CONFIG_ADDR + 4)
#define		FLOW_AD_CAL1_FLOW_MAX_ADDR	(FLOW_CONFIG_ADDR + 5)
#define		FLOW_AD_CAL2_FLOW_MIN_ADDR	(FLOW_CONFIG_ADDR + 6)
#define		FLOW_AD_CAL2_FLOW_MAX_ADDR	(FLOW_CONFIG_ADDR + 7)
#define		FLOW_PAUSE_COEF1_ADDR		(FLOW_CONFIG_ADDR + 8)
#define		FLOW_PAUSE_COEF2_ADDR		(FLOW_CONFIG_ADDR + 9)

#define 	DEPTH_CONFIG_ADDR 			(FLOW_CONFIG_ADDR + (sizeof(flow_config_t) + 15)/16*8) 
#define 	DEPTH_INPUT_TYPE_ADDR		(DEPTH_CONFIG_ADDR + 1) //高八位 
#define     DEPTH_PORT_ADDR				(DEPTH_CONFIG_ADDR + 1) //低八位
#define     DEPTH_ENC_N_ADDR			(DEPTH_CONFIG_ADDR + 2) 
#define     DEPTH_ENC_M_ADDR			(DEPTH_CONFIG_ADDR + 3) 
#define		DEPTH_MIN_DEPTH_ADDR		(DEPTH_CONFIG_ADDR + 4)
#define		DEPTH_MAX_DEPTH_ADDR		(DEPTH_CONFIG_ADDR + 5)
#define		DEPTH_SAMPLE_DEPTH_ADDR		(DEPTH_CONFIG_ADDR + 6)
#define		DEPTH_OFFSET_ADDR			(DEPTH_CONFIG_ADDR + 7)
#define		DEPTH_MIN_VALID_ADDR		(DEPTH_CONFIG_ADDR + 8)
#define		DEPTH_INC_PILE_ADDR			(DEPTH_CONFIG_ADDR + 9)
#define		DEPTH_CUR_ON_ADDR			(DEPTH_CONFIG_ADDR + 10)
#define		DEPTH_CUR_OFF_ADDR			(DEPTH_CONFIG_ADDR + 11)
#define		DEPTH_MOVE_ON_DUR_ADDR		(DEPTH_CONFIG_ADDR + 12)
#define		DEPTH_MOVE_OFF_DUR_ADDR		(DEPTH_CONFIG_ADDR + 13)
#define		DEPTH_MOVE_CUR_CH_ADDR		(DEPTH_CONFIG_ADDR + 14)

#define		FLOW_INPUT_TYPE_MODE1		(1) //4-20ma
#define		FLOW_INPUT_TYPE_MODE2		(2) //0-3.6k

#define		DEPTH_INPUT_TYPE_MODE1		(0)
#define		DEPTH_INPUT_TYPE_MODE2		(0b100000000) //1<<8
#define		DEPTH_INPUT_TYPE_MODE3		(0b1000000000) //2<<8
#define		DEPTH_INPUT_TYPE_MODE4		(0b1100000000) //3<<8

void ui_screen2_init(void);