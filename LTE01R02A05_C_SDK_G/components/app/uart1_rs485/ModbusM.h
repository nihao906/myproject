#ifndef _MODBUSM_H_
#define _MODBUSM_H_

#include <stdint.h>

typedef struct _MODBUS_MASTRT_ {
  uint8_t Status;
  uint8_t ErrCode;

  uint8_t Slave;
  uint8_t Function;

  uint8_t PackId;
  uint8_t Length;
  uint16_t Address;

  uint16_t *Data;

  uint32_t txtime;
  uint32_t rxtime;
  uint16_t TxCont;
  uint16_t TimeOut;
  uint16_t CrcErr;
  uint16_t ByteMiss;
  uint16_t OtherErr;
} MODBUS_MASTERT_T;

typedef struct _MODBUS_READ_T_ {
  uint8_t SaveAdd;
  uint8_t Length;
  uint16_t RegAdd;
  uint16_t *pDat;
} MODBUS_READ_T;

typedef struct _packet_ {
  uint8_t device_type;
  uint8_t slave;
  uint8_t funcode;
  uint8_t ch;
  uint16_t length;
  uint16_t address;
  uint16_t local_addr;
  uint16_t ext_flg;
  const void *ext;
  void (*cb)(uint16_t *a, int b);
} packet_t;

typedef struct _input_ {
  uint8_t packet_cnt;
  uint8_t max_try_times;
  uint8_t time_out;

  uint16_t scan_rate;
  uint16_t scan_space;

  uint8_t max_err_cnt;
  uint8_t ch;
  uint8_t rsv[2];
  packet_t packets[1];
} input_t;

typedef struct _output_ {
  uint8_t packet_cnt;
  uint8_t max_try_times;
  uint8_t try_space;
  uint8_t rsv;
  packet_t packets[1];
} output_t;

typedef struct _device_ {
  uint8_t channl;
  uint8_t protocol;
  uint8_t status;
  uint8_t in_index;

  uint8_t try_times;
  uint8_t out_pending;
  uint8_t out_index;
  uint8_t ErrCode;

  uint8_t continue_err;
  uint8_t max_coutinue;
  uint16_t ErrCount;
  uint16_t PackId;

  uint32_t group_start_time;
  uint32_t next_scan_time;
  uint32_t send_req_time;
  uint32_t revice_resp_time;
  uint8_t *txbuf;
  int txbuf_size;
  int (*write)(uint8_t *, int);
  uint8_t *in_err_cnt;
  input_t *in;
  output_t *out;
  void *ext;
} device_t;

typedef struct _send_ {
  uint8_t device_id;
  uint8_t sn;
  uint8_t packet_index;
  uint8_t try_times;
  uint16_t address;
  uint16_t length;
  uint32_t next_try_time;
} send_t;

int ModbusMastertRx(MODBUS_MASTERT_T *m, uint8_t *rxbuf, uint8_t len);
void ModbusMastertPoll(MODBUS_MASTERT_T *m);
int zb_ModBusWordWriteHook(unsigned short addr, unsigned short length);
#endif  // _MODBUSM_H_
