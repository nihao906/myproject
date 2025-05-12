#ifndef __CFG_H
#define __CFG_H

#include "stdint.h"

#define ADC_CAL_OFFSET 18
#define LOCATION_DATA_ADDR 64
#define TIME_CFG_ADDR 10
#define BATT_VOLTAGE_ADDR 10

#define SAVE_DSC_MAGIC 0x11223344
typedef struct
{
  char host[2][32];
  uint16_t port[2];
  uint8_t dtu_type[4];
  char dtu_uid[16];
  char pass[16];
  char APN[16];
  char User[16];
  char Pwd[16];
} dsc_cfg_t;

// typedef struct {
//   uint32_t baud;
//   uint8_t pwr_pin;
// } gps_cfg_t;

// typedef struct {
//   uint16_t hw_type;
//   uint16_t port_fwd_en;
//   uint8_t ip[4];
//   uint8_t mask[4];
//   uint8_t gw[4];
//   uint8_t mac[6];
//   uint16_t local_port;
//   uint16_t dest_port;
//   uint8_t dest_ip[4];
// } net_cfg_t;

// typedef struct {
//   uint32_t baud;
//   char parity;
//   uint8_t data_bits;
//   uint8_t stop_bits;
//   uint8_t enable;
// } uart_cfg_t;

// extern uart_cfg_t uart_cfg[2];
// typedef struct _meter_cfg_ {
//   uint16_t type;
//   uint16_t count;
//   uint16_t addr;
//   uint16_t scan_rate;
//   uint16_t scan_delay;
//   uint16_t scan_timeout;
//   uint16_t trans_time;
// } meter_cfg_t;

// typedef struct {
//   int16_t ad;
//   int16_t voltage;
// } adc_cal_t;
#define SENSER_CFG_MAGIC  0x55aa
#define SENSER_TEMP_TYPE_YAHUA  1
#define SENSER_TEMP_TYPE_SHIAO  2
#define SENSER_CMV_TYPE_WEIKONG      1
#define SENSER_CMV_TYPE_DONGTAI      2
typedef struct senser_cfg
{
  uint16_t temp_type;  // 温度传感器类型
  uint16_t temp_slave; // 温度传感器从机地址
  uint16_t temp_count; // 温度传感器个数
  uint16_t cmv_type;
  int16_t speed_avg;
  int16_t speed_dec;
  int16_t speed_max;
  uint16_t magic;
} senser_cfg_t;

extern senser_cfg_t *senser_cfg;

extern int load_sensor_cfg(senser_cfg_t *gps_cfg);
extern int save_sensor_cfg(senser_cfg_t *gps_cfg);
extern int load_dsc_cfg(dsc_cfg_t *dsc_cfg);

#endif //__CFG_H
