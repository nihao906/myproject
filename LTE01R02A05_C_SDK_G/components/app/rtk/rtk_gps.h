#ifndef _LSAPI_H_
#define _LSAPI_H_

#include "minmea.h"

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

#pragma pack() /*取消指定对齐，恢复缺省对齐*/

enum gps_id {
  GPS_ID_1 = 0,
  GPS_ID_2 = 1
};
/**
 * @brief 业务部分gps信息
 */
typedef struct {
  uint32_t UTC;
  int64_t LAT;
  int64_t LNG;
  int32_t ALT;
  int32_t speed;
  struct minmea_date date;
  struct minmea_time time;
  uint8_t Q;  // STATUS
  int32_t fix_quality;
  int16_t azimuth;
  uint8_t total_gsv;
  uint8_t total_gsa;
  uint8_t id; // 对应的Gps编号
  float pitch; //俯仰角
  float baseline; //长度
  float Heading;  //航向角
} gps_t;

extern gps_t gps1_parse;
// struct Point flat_res_point;
void rtk_gps_task_init(void);
void gps_cale_point(gps_t *gps1);
void save_gps_point_coord(void);
void gps_task(void *pvParameters);
void gps_point_coord_init(void);
// void modbus_write_gps_data_cb(uint16_t *addr, int len);
#endif