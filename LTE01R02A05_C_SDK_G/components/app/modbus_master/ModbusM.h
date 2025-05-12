#ifndef _MODBUSM_H_
#define _MODBUSM_H_
#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int

typedef struct _MODBUS_MASTRT_
{
    uint8 Status;
    uint8 ErrCode;

    uint8 Slave;
    uint8 Function;

    uint8 PackId;
    uint8 Length;
    uint16 Address;

    uint16 *Data;

    uint32 txtime;
    uint32 rxtime;
    uint16 TxCont;
    uint16 TimeOut;
    uint16 CrcErr;
    uint16 ByteMiss;
    uint16 OtherErr;
}MODBUS_MASTERT_T;
typedef  struct _MODBUS_READ_T_
{
    uint8 SaveAdd;
    uint8 Length;
    uint16 RegAdd;
    uint16 *pDat;
}MODBUS_READ_T;



typedef  struct  _packet_
{
    uint8_t device_type;
    uint8_t slave;
    uint8_t funcode;
    uint8_t ch;
    uint16_t length;
    uint16_t address;
    uint16_t local_addr;
    uint16_t ext_flg; 
    const void* ext;
}packet_t;

typedef  struct  _input_
{
    uint8_t packet_cnt;
    uint8_t max_try_times;
    uint8_t time_out;

    uint16_t scan_rate;
    uint16_t scan_space;

    uint8_t  max_err_cnt;
		uint8_t  ch;
    uint8_t rsv[2];
    packet_t packets[1];
}input_t;

typedef  struct  _output_
{
    uint8_t packet_cnt;
    uint8_t max_try_times;
    uint8_t try_space;
    uint8_t rsv;
    packet_t packets[1];
}output_t;

typedef  struct  _device_
{
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
		int (*write)(uint8*, int);
    uint8_t *in_err_cnt;
    input_t *in;
    output_t *out;
    void *ext;
}device_t;

typedef  struct  _send_
{
    uint8_t device_id;
    uint8_t sn;
    uint8_t packet_index;
    uint8_t try_times;
    uint16_t address;
    uint16_t length;
    uint32_t next_try_time;
}send_t;
extern int ModbusM_init(void);
extern int ModbusMastertRx(MODBUS_MASTERT_T *m,uint8 *rxbuf, uint8 len);
extern void ModbusMastertPoll(MODBUS_MASTERT_T *m);
extern int zb_ModBusWordWriteHook(unsigned short addr, unsigned short length);
extern void modbus_master_poll(int n);
extern int modbus_master_on_revice(int ch, uint8_t *rxbuf, int length);
extern void ModbusM_reinit(void);
#endif
