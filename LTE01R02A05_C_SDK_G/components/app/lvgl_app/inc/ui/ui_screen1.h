#include "lvgl.h"

#define LEFT_BUTTON_X (0)
#define LEFT_BUTTON_Y (0)
#define RIGHT_BUTTON_X (450)
#define RIGHT_BUTTON_Y (0)
#define START_BUTTON_X (440)
#define START_BUTTON_Y (190)
#define STOP_BUTTON_X (440)
#define STOP_BUTTON_Y (240)

/************************************************************************************************************/
/* 自定义图标集合 */
#define ICON_GRAPH_USER_14 "\xEE\xA0\x8F "				  // 0xe80f 曲线图图标
#define ICON_INCLINATION_USER_14 "\xEE\x98\x88 "		  // 0xe608 倾角测量图标
#define ICON_RECORD_USER_14 "\xEE\x98\xB0 "				  // 0xe630 记录图标
#define ICON_OPERATION_USER_14 "\xEE\x99\x82 "			  // 0xe642 作业图标
#define ICON_LIGHT_20 "\xEE\x98\x93 "					  // 0xe613 亮度图标
#define ICON_SET_SYS_24 "\xEE\x98\x99 "					  // 0xe619 系统图标
#define ICON_SET_ABOUT_24 "\xEE\x98\x80 "				  // 0xe600 关于图标
#define ICON_SET_USER_24 "\xEE\x98\xBD "				  // 0xe63d 用户图标
#define ICON_SET_FLOW_24 "\xEE\x98\xA5 "				  // 0xe625 流量计图标
#define ICON_SET_DEPTH_24 "\xEE\xA2\xA9 "				  // 0xe8a9 深度计图标
#define ICON_SET_CAL_24 "\xEE\x9A\x8F "					  // 0xe68f 校准图标
#define ICON_INC_STATE_10 "\xEE\x9B\xA6 "				  // 0xe68f 倾角目标图标
#define ICON_GPS_ON_STATE_24 "\xEE\x98\x80 "			  // 0xe600 GPS开启图标
#define ICON_GPS_OFF_STATE_24 "\xEE\x98\x81 "			  // 0xe601 GPS关闭图标
#define ICON_SIM_OFF_STATE_24 "\xEE\x9A\x98 "			  // 0xe698 sim卡未填充图标
#define ICON_SIM_WARN_STATE_24 "\xEE\x9A\xA9 "			  // 0xe6a9 sim卡故障图标
#define ICON_SIM_ON_STATE_24 "\xEE\x9A\xAA "			  // 0xe6aa sim卡已填充图标
#define ICON_NET_ON_STATE_24 "\xEE\x98\xA3 "			  // 0xe623 连接网络图标
#define ICON_NET_OFF_STATE_24 "\xEE\x98\xA4 "			  // 0xe624 断开网络图标
#define ICON_NET_WARN_STATE_24 "\xEE\x99\x93 "			  // 0xe653 网络故障图标
#define ICON_SERVER_CON_ON_STATE_24 "\xEE\x9A\xB8 "		  // 0xe6b8 服务器连接状态图标
#define ICON_SERVER_CON_DOWNLOAD_STATE_24 "\xEE\x98\x82 " // 0xe602 服务器数据下载状态图标
#define ICON_SERVER_CON_UPLOAD_STATE_24 "\xEE\x98\x83 "	  // 0xe603 服务器数据上传状态图标
#define ICON_SERVER_CON_NO_STATE_24 "\xEE\x98\x84 "		  // 0xe604 服务器无连接连接状态图标
#define ICON_GPS_CSQ_5_24 "\xEE\x98\xAB "				  // 0xe62b GPS信号5格图标
#define ICON_GPS_CSQ_4_24 "\xEE\x98\xB4 "				  // 0xe634 GPS信号4格图标
#define ICON_GPS_CSQ_3_24 "\xEE\x98\xB5 "				  // 0xe635 GPS信号3格图标
#define ICON_GPS_CSQ_2_24 "\xEE\x98\xB6 "				  // 0xe636 GPS信号2格图标
#define ICON_GPS_CSQ_1_24 "\xEE\x98\xB7 "				  // 0xe637 GPS信号1格图标
#define ICON_GPS_CSQ_24 "\xEE\x9A\x80 "					  // 0xe680 GPS信号图标
#define ICON_GNET_CSQ_4_24 "\xEE\x9A\x81 "				  // 0xe681 4G信号4格图标
#define ICON_GNET_CSQ_3_24 "\xEE\x98\x93 "				  // 0xe613 4G信号3格图标
#define ICON_GNET_CSQ_2_24 "\xEE\x98\x91 "				  // 0xe611 4G信号2格图标
#define ICON_GNET_CSQ_1_24 "\xEE\x98\x94 "				  // 0xe614 4G信号1格图标
#define ICON_GNET_CSQ_0_24 "\xEE\x98\x92 "				  // 0xe612 4G信号0格图标
#define THEME_COLOR_LIGHT_MODE_DEF 0xFFFFFF
#define THEME_COLOR_DARK_MODE_DEF 0x292831
#define THEME_COLOR_DODER_BLUE_DEF 0x1E90FF
#define THEME_COLOR_WHITE_DEF 0xFFFFFF
#define THEME_COLOR_BLACK_DEF 0x000000
#define THEME_COLOR_SLATE_GREY_DEF 0x708090
#define THEME_COLOR_SPRING_GREEN_DEF 0x00FF7F
#define THEME_COLOR_FIRE_BRICK1_DEF 0xFF3030
#define THEME_COLOR_FIRE_BRICK3_DEF 0xCD2626
#define THEME_COLOR_YELLOW_DEF 0xFFFF00
#define THEME_COLOR_GRAY81_DEF 0xCFCFCF
#define THEME_COLOR_SEA_GREEN1_DEF 0x54FF9F
#define THEME_COLOR_BTN_BLUE_DEF 0x2196F3
#define THEME_COLOR_SNOW3_DEF 0xCDC9C9
#define THEME_COLOR_SNOW4_DEF 0x8B8989
#define THEME_COLOR_SEAGREEN2_DEF 0x4EEE94
#define THEME_COLOR_GOLD3_DEF 0xCDAD00

/************************************************************************************************************/
// 字体合集
extern const lv_font_t ui_font_12;
extern const lv_font_t lv_font_montserrat_16;
extern const lv_font_t lv_font_montserrat_20;
extern const lv_font_t lv_font_montserrat_42;
extern const lv_font_t system_status_icon_24;

extern lv_obj_t *ui_Screen1;		   // 用于切屏
extern lv_obj_t *current_bar;		   // 显示电流的bar
extern lv_obj_t *SIM_card_label;	   // sim卡状态
extern lv_obj_t *gnet_dsc_label;	   // 4G描述label
extern lv_obj_t *g_gnet_label;		   // 4G状态label
extern lv_obj_t *gps_level_label;	   // gps状态
extern lv_obj_t *net_connect_label;	   // 网络连接状态
extern lv_obj_t *server_connect_label; // 服务器连接状态
extern lv_obj_t *screen1_led;		   // 用于更改led颜色
extern lv_obj_t *screen1_play_btn_label;

extern lv_obj_t *screen1_measurements_label1;
extern lv_obj_t *screen1_measurements_label2;
extern lv_obj_t *screen1_measurements_label3;
extern lv_obj_t *screen1_measurements_label4;
extern lv_obj_t *screen1_measurements_label5;
extern lv_obj_t *screen1_measurements_label6;
extern lv_obj_t *screen1_measurements_label7;
extern lv_obj_t *screen1_measurements_label8;
extern lv_obj_t *screen1_measurements_label9;

#pragma pack(2)
typedef struct
{
	short enc_val; // 编码器原始值 无效
	short ss_1;	   // 1通道瞬时流量
	short ss_2;
	short flow_10cm_1; // 1通道瞬时流量
	short flow_10cm_2;
	uint16_t accumulate1;
	uint16_t accumulate2;
	uint16_t one_pile_work_time;
	int ll_1; // 1通道累计流量
	int ll_2;
	short speed;	   // 速度
	short depth;	   // 深度
	unsigned short Ia; // a通道电流值
	unsigned short Ib;
	unsigned short Ic;
	unsigned short cnt; // 计数 无效
	short angle_x;		// x轴角度
	short angle_y;
	short angle_z;
	int dx;			   // 经度
	int dy;			   // 维度
	unsigned short id; // 桩点号
} user_data_t;
#pragma pack() /*取消指定对齐，恢复缺省对齐*/

#pragma pack(1)
typedef enum
{
	WORK = 0x0200,
	PAUSE = 0x0100,
	STOP = 0x0000,
} MACHINE_WORK_STATE;
#pragma pack()

void ui_screen1_init(void);