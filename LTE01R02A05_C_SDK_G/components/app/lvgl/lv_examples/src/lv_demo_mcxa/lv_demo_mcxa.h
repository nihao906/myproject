/**
 * @file lv_demo_mcxa.h
 *
 */

#ifndef LV_DEMO_MCXA_H
#define LV_DEMO_MCXA_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdint.h>
#include <stdio.h>
#pragma pack(1)
/**
 * @brief 业务部分t2n上报数据结构
 */
typedef struct {
  uint32_t TS;
  int32_t LAT;
  int32_t LNG;
  int16_t ALT;
  uint8_t Q;
  int16_t azimuth;
  int16_t pile_id;
  uint8_t times;
  int16_t depth;
  uint16_t flow1;
  uint16_t flow2;
  int8_t tilt_x;
  int8_t tilt_y;
  uint16_t current1;
  uint16_t current2;
} t2n_report_t;
#pragma pack()
#pragma pack(2)
/**
 * @brief 业务部分配置信息
 */
typedef struct {
  unsigned short magic;  // R
  unsigned short save;  // RW
  unsigned short bps;
  unsigned short modbus_addr;
  unsigned short cof;  // 系数
  unsigned short enc_threshold;  // 阈值cm
  signed short dir;  // ±1
  unsigned short traffic_mode;  // 流量计模式选择
  unsigned short encode_mode;  // 编码器模式选择
  unsigned short encode_pluse_dir_pin_select;  // 脉冲编码器方向引脚选择
  unsigned short widget_eng_h;  // 工程值 对应实际单位
  unsigned short widget_eng_l;
  unsigned short widget_raw_h;  // 传感器原始值
  unsigned short widget_raw_l;
  unsigned short ma_1_eng_h;  // 工程值 对应实际单位
  unsigned short ma_1_eng_l;
  unsigned short ma_1_raw_h;  // 传感器原始值
  unsigned short ma_1_raw_l;
  unsigned short ma_2_eng_h;  // 工程值 对应实际单位
  unsigned short ma_2_eng_l;
  unsigned short ma_2_raw_h;  // 传感器原始值
  unsigned short ma_2_raw_l;
  unsigned short frq_ll_1_eng_h;  // 工程值 对应实际单位
  unsigned short frq_ll_1_eng_l;
  unsigned short frq_ll_1_raw_h;  // 传感器原始值
  unsigned short frq_ll_1_raw_l;
  unsigned short frq_ll_2_eng_h;  // 工程值 对应实际单位
  unsigned short frq_ll_2_eng_l;
  unsigned short frq_ll_2_raw_h;  // 传感器原始值
  unsigned short frq_ll_2_raw_l;
} config_data_t;
typedef enum {
  LSAPI_SIM_ABSENT = 0x00, // Sim Status is SIM_ABSENT
  LSAPI_SIM_NORMAL = 0x01, // Sim Status is SIM_NORMAL
  LSAPI_SIM_TEST = 0x02, // Sim Status is SIM_TEST
  LSAPI_SIM_ABNORMAL = 0x03, // Sim Status is SIM_ABNORMAL
  LSAPI_SIM_STATUS_END = 0x04, // Sim Status is SIM_STATUS_END
  LSAPI_SIM_TYPE_SOFT = 0x05, // Sim Status is SIM_TYPE_SOFT
  LSAPI_SIM_STATUS_ENUM_FILL = 0x7fffffff // Sim Status is SIM_STATUS_ENUM_FILL 
} LSAPI_SIM_STATUS;
enum sys_ser_data_sta{
  SYS_STA_SER_LOGIN_FAILED = 0x00,
  SYS_STA_SER_LOGIN_SUCCESS = 0x01,
  SYS_STA_SER_NO_DATA = 0x02,
  SYS_STA_SER_UPLOAD_DATA = 0x03,
  SYS_STA_SER_UPLOAD_COMPLETE_DATA = 0x04,
  SYS_STA_SER_DOWNLOAD_DATA = 0x05,
  SYS_STA_SER_DOWNLOAD_COMPLETE_DATA = 0x06,
};
/**
 * @brief 系统部分网络信息
 */
typedef struct {
  int csq; // 网络信号强度
  int att_state; // 网络连接状态
  int sim_state; // sim卡状态
  int server_state; // 服务器连接状态
} sys_net_status_t;
/**
 * @brief 系统部分Gps信息
 */
typedef struct {
  int csq; // Gps信号强度
  int con_state; // 连接状态
} sys_gps_status_t;
/**
 * @brief 同步给ui的系统数据
 */
typedef struct {
  sys_net_status_t snts; // 网络部分状态
  sys_gps_status_t sgs1; // Gps1部分状态
  sys_gps_status_t sgs2; // Gps2部分状态
} sys_data_t;
/**
 * @brief 同步给ui的业务数据
 */
typedef struct {
  short enc_val;  // 编码器原始值 无效
  short ss_1;  // 1通道瞬时流量
  short ss_2;
  short flow_10cm_1;  // 1通道瞬时流量
  short flow_10cm_2;
  int ll_1;  // 1通道累计流量
  int ll_2;
  short speed;  // 速度
  short depth;  // 深度
  unsigned short Ia;  // a通道电流值
  unsigned short Ib;
  unsigned short Ic;
  unsigned short cnt;  //计数 无效
  short angle_x;  // x轴角度
  short angle_y;
  short angle_z;
  int dx;  // 经度
  int dy;  // 维度
  unsigned short id;  // 桩点号
} user_data_t;
#pragma pack() /*取消指定对齐，恢复缺省对齐*/
/**
 * @brief 后台事件处理ID
 */
enum {
  UI_EVENT_SEND_DATA = 0x2000,
  UI_EVENT_STOP_BTN_ID = 10,
  UI_EVENT_PAUSE_BTN_ID = 11,
  UI_EVENT_CAL_INC_BTN_ID = 12,
};
/**
 * @brief 前台事件处理ID
 */
enum {
  BK_EVENT_UI_USER_DATA_ID = 9,
  BK_EVENT_T2N_REPORT_ID = 10,
  BK_EVENT_DATA_SYNC_ID = 11,
  BK_EVENT_IMEI_DATA_SYNC_ID = 12,
  BK_EVENT_SYS_DATA_SYNC_ID = 13,
};
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/
void lv_demo_mcxa(void);
/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /*LV_DEMO_MCXA_H*/
