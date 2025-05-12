#ifndef _AC_CURRENT_H
#define _AC_CURRENT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../t2n/include/ModbusS.h"

#define STOP    0x0100
#define WORK    0x0200
#define AC_CURRENT_REG_ADDR 24
#define ENC_VALUE_REG_ADDR 28

#define REG_DOSING_DATA_CFG  300
#define REG_HAMMER_DATA_CFG  310
#define REG_WORK_DATA_CFG  322
#define REG_CONFIG_DATA_CFG  340
#define REG_GPS_DATA_CFG  360
typedef struct
{
    uint32_t total_time;
    uint32_t work_time;
    uint32_t stop_time;
    uint16_t current;//电流
    uint16_t cnt;
    uint16_t walk_state;

} dosing_config_t;

typedef struct
{

    uint32_t total_time;
    uint32_t work_time;
    uint32_t up_time;
    uint32_t stop_time;
    uint16_t current;
    uint16_t cnt;
    uint16_t walk_state;

} hammer_config_t;
typedef struct
{
    uint32_t work_time;
    uint32_t total_time;
    uint16_t work_current;
    uint16_t state;
    uint16_t id;  
} total_config_t;

typedef struct
{

    uint16_t dosing_up;
    uint16_t dosing_down;
    uint16_t dosing_con_time;
    uint16_t hammer_start;
    uint16_t hammer_up;
    uint16_t hammer_down;
    uint16_t hammer_con_time; 
    uint16_t work_up;
    uint16_t work_down;
    uint16_t work_con_time;
    uint16_t work_total_con_time;
    int16_t coefficient_up; //系数
    int16_t coefficient_down;

} config_t;

typedef struct gps_message
{
    uint8_t gps_status; // GPS定位状态
    uint8_t gps_view;   // GPS可视状态
    uint8_t gps_use;    // GPS使用状态
    uint8_t reg[1];     // 保留
    uint32_t utc;       // utc时间
    double x;           // 大地平面坐标
    double y;
    double x_res;
    double y_res;
    double dir;         // 方向
    float pitch;       // 俯仰角
    int32_t height;         
} gps_message_t;
#define GPS_DISTANCE_TH         (0.5)

struct Point {
  double x;  // x
  double y;  // y
  double z;  // h
};
// extern gps_message_t *gps_message = (gps_message_t*)&gWordVar[REG_GPS_DATA_CFG];



void get_gps_xy(void);
void judg_location(void);

extern uint8_t DATA_status;
extern uint8_t times;
extern uint64_t first_dosing_time;
extern uint64_t end_hammer_time;
extern uint32_t duration;
extern uint32_t subtotal_time;
extern uint16_t depth;
extern uint16_t timeline;

extern dosing_config_t *dosing;
extern hammer_config_t *hammer;
extern total_config_t *work ;
extern config_t *config;
extern double gps_xy[2];
extern uint32_t now;
#endif