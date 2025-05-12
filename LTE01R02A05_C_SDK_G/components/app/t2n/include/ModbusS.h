#ifndef _MODBUS_H
#define _MODBUS_H
#include <stdint.h>

#define setBit(Add) gBitVar[(Add) >> 3] |= (1 << ((Add)&0x07))
#define clrBit(Add) gBitVar[(Add) >> 3] &= ~(1 << ((Add)&0x07))
//#define ModBusTxData uart1_tx

extern uint16_t crc16(uint8_t *puchMsg, uint16_t usDataLen);
extern int ModbusSlaveProcess(uint8_t *txbuf, uint8_t *rxbuf, uint16_t rxLen, int is_crc);
extern void modbus(void);
extern void ModbusSetLocalReg(uint16_t addr, uint16_t value);

// #define local 1

int isBitHI(uint16_t Add);
void xorBit(uint16_t Add);

void WriteBit(uint16_t Add, uint8_t bit_value);
#define gBIT_SIZE 128
#define gWORD_SIZE 8192
extern uint8_t gBitVar[(gBIT_SIZE + 7) / 8];
extern uint16_t gWordVar[gWORD_SIZE];


// typedef struct
// {
//     int16_t csq;         16
//     uint16_t net_status; 17
//     uint16_t gps_view;
//     uint16_t gps_use;
//     uint16_t gps_status;
//     int16_t speed; // 速度
//     int16_t TEMP;  // 温度
// } ui_data_t;

void modbus(void);

#define REG_IMEI 32
#define REG_CCID 48

#define REG_CSQ 16
#define REG_NET_STATUS 17
#define REG_VIEW_STAS 18
#define REG_USE_STAS 19
#define REG_GPS_STATUS 20
#define REG_SPEED 21
#define REG_TEMPS 22
#define REG_PASS_NUMBER 25
#define REG_VIBRATE_SENSOR 26
#define REG_UPDATE_STATUS 31
#define REG_UBLOX_CFG_REQ REG_GPS_STATUS

#define REG_WIEGAND_ID_LOW 64
#define REG_WIEGAND_ID_HIGH 65
#define REG_WIEGAND_ID_ERR_LOW 66
#define REG_WIEGAND_ID_ERR_HIGH 67
#define REG_WIEGAND_OK_CNT 68
#define REG_WIEGAND_ERR_CNT 69
#define REG_WIEGAND_DI_VALUE 70
#define RTCM_AGE     71

#define REG_SHAKE_WRITE_MAGIC   (100)   // 占1个word
#define REG_SHAKE_FEQ_MIN_CFG   (REG_SHAKE_WRITE_MAGIC + 2)  //102
#define REG_SHAKE_FEQ_MAX_CFG   (REG_SHAKE_FEQ_MIN_CFG + 2) //104
#define REG_SHAKE_A_TH_CFG      (REG_SHAKE_FEQ_MAX_CFG + 2)     //106
#define REG_SHAKE_MAX_FEQ       (REG_SHAKE_A_TH_CFG + 2)  // 108
#define REG_SHAKE_MAX_A         (REG_SHAKE_MAX_FEQ + 2) // 110
    
#define REG_REAL_PRESSURE_VAL   112  // /100
#define REG_JUMP                113  // /10
#define REG_FEQ                 114  
#define REG_AMPLITUDE           115


#define REG_CFG_SERSER          256
#define REG_CFG_SERSER_SAVE    (REG_CFG_SERSER + 7)
// // flash
// #define REG_ESP32_FLASH_COMMAND         (110)
// #define REG_ESP32_FLASH_PHASE           (111)
// #define REG_ESP32_FLASH_STATUS          (112)
// #define REG_ESP32_FLASH_MESSAGE         (113)

// 震动数据tcp回传控制
#define REG_SHAKE_TCP_WRITE_DATA_CLASS_SELECT  (126)    // raw_data(bit0)和feq_data(bit1)的传输开关
#define REG_SHAKE_TCP_WRITE_RAW_DATA_SELECT    (127)    // raw_data三路数据的选择x(bit0), y(bit1), z(bit2)
#define REG_SHAKE_TCP_WRITE_FEQ_DATA_SELECT    (128)    // feq_data三路数据的选择fq_y(bit0), fq_z(bit1), fd_sum(bit2)



#endif
