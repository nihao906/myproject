#include <stdint.h>
#include <string.h>
#include <time.h>
#include "ql_log.h"
#include "drv_uart.h"
#include "ModbusM.h"
#include "ModbusS.h"
#include "osi_api.h"
#include "../t2n/include/cfg.h"
#include "../rtk/gps_coord.h"

// no such file
//  #include "minmea.h"
//  #include "rtk_gps.h"
//  #include "lv_demo_mcxa.h"
////////////////////////////////////////////////////////////////////////////////
#pragma pack(2)
typedef struct
{
  short enc_val; // 编码器原始值 无效
  short ss_1;    // 1通道瞬时流量
  short ss_2;
  short flow_10cm_1; // 1通道瞬时流量
  short flow_10cm_2;
  int ll_1; // 1通道累计流量
  int ll_2;
  short speed;       // 速度
  short depth;       // 深度
  unsigned short Ia; // a通道电流值
  unsigned short Ib;
  unsigned short Ic;
  unsigned short cnt; // 计数 无效
  short angle_x;      // x轴角度
  short angle_y;
  short angle_z;
  int dx;            // 经度
  int dy;            // 维度
  unsigned short id; // 桩点号
} user_data_t;
#pragma pack() /*取消指定对齐，恢复缺省对齐*/
typedef struct
{
  uint32_t UTC;
  int64_t LAT;
  int64_t LNG;
  int32_t ALT;
  int32_t speed;
  uint8_t Q; // STATUS
  int32_t fix_quality;
  int16_t azimuth;
  uint8_t total_gsv;
  uint8_t total_gsa;
  uint8_t id; // 对应的Gps编号
} gps_t;
#pragma pack(1)
/**
 * @brief 业务部分t2n上报数据结构
 */
typedef struct
{
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
struct minmea_time
{
  uint8_t hours;
  uint8_t minutes;
  uint8_t seconds;
  uint8_t rsv;
  uint32_t microseconds;
};
struct minmea_date
{
  uint8_t day;
  uint8_t month;
  uint16_t year;
};
////////////////////////////////////////////////////////////////////////////////
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define LOG_TAG "[com_poll]: "
#define ZB_CHANNEL 2

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, msg, ##__VA_ARGS__)

extern uint16_t gWordVar[];
device_t zb_device[ZB_CHANNEL];
static send_t zb_send[16];
#define GetTicks osiUpTime // monoclinic system time

int zbapi_send_write(device_t *device, send_t *sender);
int zbapi_send_req(device_t *device);

static uint8_t uart_txbuf[256];
// static float pow_user(float x, float y)
// {
//   int i;
//   float tmp = 1;
//   for (i = 0; i < y; i++)
//   {
//     tmp = tmp * x;
//   }
//   return tmp;
// }

#define TEMP_WINDOWS_SIZE (10)
#define INVAILD_TEMP (-999)

int16_t get_temp_max(int ch, int16_t temp)
{
  static int16_t temp_window[3][TEMP_WINDOWS_SIZE] = {0};
  static int w_index[3] = {0, 0, 0};
  int16_t max_temp = INVAILD_TEMP; // -99.9
  if (ch < 0 || ch > 2)
  {
    ch = 0;
  }
  temp_window[ch][w_index[ch]] = temp;
  if (++w_index[ch] >= TEMP_WINDOWS_SIZE)
  {
    w_index[ch] = 0;
  }
  for (int i = 0; i < TEMP_WINDOWS_SIZE; i++)
  {
    if (temp_window[ch][i] > max_temp)
    {
      max_temp = temp_window[ch][i];
    }
  }
  return max_temp;
}

static uint16_t bin2bcd(uint16_t bin)
{
  uint16_t bcd;
  uint8_t buf[4];
  if (bin > 9999)
  {
    bin = 9999;
  }
  buf[0] = bin % 10;
  bin /= 10;
  buf[1] = bin % 10;
  bin /= 10;
  buf[2] = bin % 10;
  buf[3] = bin / 10;
  bcd = buf[0] | (buf[1] << 4) | (buf[2] << 8) | (buf[3] << 12);
  return bcd;
}

// ��������sn ֵ
static uint8_t get_new_sn(int device_id)
{
  int i;
  int sn = 0;
  for (i = 0; i < 16; i++)
  {
    if (zb_send[i].device_id == device_id)
    {
      sn = MAX(zb_send[i].sn, sn);
    }
  }
  return sn + 1;
}

// �����豸��Сsn��Ӧ���±�
static int device_get_send(int device_id)
{
  int i;
  int sn = 255;
  int min_index = -1;
  for (i = 0; i < 16; i++)
  {
    if (zb_send[i].device_id == device_id)
    {
      if (zb_send[i].sn < sn)
      {
        sn = zb_send[i].sn;
        min_index = i;
      }
    }
  }
  return min_index;
}

static void on_in_pack_error(device_t *device)
{
  if (device->in_err_cnt[device->in_index] < device->in->max_err_cnt)
  {
    if (++device->in_err_cnt[device->in_index] >= device->in->max_err_cnt)
    {
      const packet_t *in_pack = device->in->packets + device->in_index;
      int i;
      device->in_err_cnt[device->in_index] = 0;
      if (in_pack->device_type == 255 || in_pack->device_type == 2)
      {
        if (in_pack->local_addr < gWORD_SIZE)
        {
          gWordVar[in_pack->local_addr] = -999;
        }
      }
      else
      {
        for (i = 0; i < in_pack->length; i++)
        {
          uint32_t reg_addr = in_pack->local_addr + i;
          if (reg_addr < gWORD_SIZE)
          {
            gWordVar[reg_addr] = 0;
          }
        }
        // on_data_error(device);
      }
    }
  }
}
void com_poll_modbus_master_poll(int n)
{
  if (n < 0)
    return;
  uint32_t now = GetTicks();
  if (zb_device[n].status == 0) // 空闲状态
  {
    if (zb_device[n].out_pending > 0) // 如果有发送请求先处理发送
    {
      int send_index = device_get_send(n);
      if (send_index >= 0)
      {
        if (now >= zb_send[send_index].next_try_time)
        {
          zb_device[n].out_index = send_index;
          zb_device[n].send_req_time = now;
          zbapi_send_write(&zb_device[n], &zb_send[send_index]);
          zb_device[n].status = 0x10;
          return;
        }
      }
      else
      {
        zb_device[n].out_pending = 0;
      }
    }
    QL_LOG(QL_LOG_LEVEL_INFO,
           "[uart1_modbus_master]:",
           "out_pending:%d, now:%d, next_scan_time:%d\n",
           zb_device[n].out_pending, now, zb_device[n].next_scan_time);

    if (now >= zb_device[n].next_scan_time)
    {
      zb_device[n].send_req_time = now;
      zbapi_send_req(&zb_device[n]);
      zb_device[n].status = 2;
    }
  }
  else if (zb_device[n].status == 1 || zb_device[n].status == 2)
  {
    if ((now - zb_device[n].send_req_time) > zb_device[n].in->time_out * 100)
    {
      QL_LOG(QL_LOG_LEVEL_INFO,
             "[uart1_modbus_master]:",
             "1/2 now:%d - zb_device[n].send_req_time:%d > zb_device[n].in->time_out * 100:%d\n",
             now, zb_device[n].send_req_time, zb_device[n].in->time_out * 100);

      zb_device[n].status = 0;
      zb_device[n].ErrCode = 0x01;
      zb_device[n].ErrCount++;
      on_in_pack_error(&zb_device[n]);
      if ((++zb_device[n].try_times >= zb_device[n].in->max_try_times) ||
          (zb_device[n].in_err_cnt[zb_device[n].in_index] >= zb_device[n].in->max_err_cnt))
      {
        if (++zb_device[n].in_index >= zb_device[n].in->packet_cnt)
        {
          zb_device[n].in_index = 0;
          zb_device[n].group_start_time = now;
        }
        zb_device[n].try_times = 0;
      }
      zb_device[n].next_scan_time = zb_device[n].group_start_time + zb_device[n].in->scan_rate * 10;
      zb_device[n].status = 0;
    }
  }
  else if (zb_device[n].status == 3) // ���յ��ظ�
  {
    if (++zb_device[n].in_index >= zb_device[n].in->packet_cnt)
    {
      zb_device[n].in_index = 0;
      zb_device[n].group_start_time = now; //+ zb_device[n].in->scan_rate*10;
    }
    zb_device[n].next_scan_time = zb_device[n].group_start_time + zb_device[n].in->scan_rate * 10;
    zb_device[n].status = 255;
    zb_device[n].try_times = 0;
  }
  else if (zb_device[n].status == 0x10) // �������������ѷ����ȴ���Ӧ���߳�ʱ
  {
    int index = zb_device[n].out_index;
    if ((now - zb_device[n].send_req_time) > zb_device[n].in->time_out * 100)
    {
      QL_LOG(QL_LOG_LEVEL_INFO,
             "[uart1_modbus_master]:",
             "0x10 now:%d - zb_device[n].send_req_time:%d > zb_device[n].in->time_out * 100:%d\n",
             now, zb_device[n].send_req_time, zb_device[n].in->time_out * 100);

      zb_device[n].status = 0;
      zb_device[n].ErrCode = 0x01;
      zb_device[n].ErrCount++;
      if (++zb_send[index].try_times >= zb_device[n].out->max_try_times) // ����������Դ���������ʧ��
      {
        zb_send[index].device_id = 0xff;
      }
      else
      {
        zb_send[index].next_try_time = now + zb_device[n].out->try_space * 100;
      }
      zb_device[n].status = 0;
      zb_device[n].try_times = 0;
    }
  }
  else if (zb_device[n].status == 0x11) // ���յ���������ȷ��
  {
    zb_send[zb_device[n].out_index].device_id = 0xff;
    zb_device[n].status = 255;
    zb_device[n].try_times = 0;
  }
  else
  {
    if ((now - zb_device[n].revice_resp_time) >= (zb_device[n].in->scan_space * 10))
    {
      QL_LOG(
          QL_LOG_LEVEL_INFO,
          "[uart1_modbus_master]:",
          "else now:%d - zb_device[n].revice_resp_time:%d > zb_device[n].in->scan_space *10:%d\n",
          now, zb_device[n].revice_resp_time, zb_device[n].in->scan_space * 10);
      zb_device[n].status = 0;
      zb_device[n].try_times = 0;
    }
  }
}

static int intersection(int x0, int x1, int y0, int y1, int *s, int *e)
{
  QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "enter intersection\n");
  int ret = 0;
  if ((y0 >= x0) && (y0 < x1))
  {
    *s = y0;
    *e = MIN(x1, y1);
    ret = 1;
  }
  else if ((y1 > x0) && (y1 <= x1))
  {
    *s = y1 - 1;
    *e = MIN(x1, y1);
    ret = 1;
  }
  else if ((x0 >= y0) && (x0 < y1))
  {
    *s = x0;
    *e = MIN(y1, x1);
    ret = 1;
  }
  else if ((x1 > y0) && (x1 <= y1))
  {
    *s = x1 - 1;
    *e = MIN(y1, x1);
    ret = 1;
  }
  return ret;
}

int zb_ModBusWordWriteHook(uint16_t addr, uint16_t length)
{
  QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "enter zb_ModBusWordWriteHook\n");
  int i, j;
  int ret = 0;
  for (i = 0; i < ZB_CHANNEL; i++)
  {
    if (zb_device[i].out == NULL)
    {
      continue;
    }
    for (j = 0; j < zb_device[i].out->packet_cnt; j++)
    {
      int s1, e1;
      int s2, e2;
      int s, e;
      int n;
      s1 = zb_device[i].out->packets[j].local_addr;
      e1 = zb_device[i].out->packets[j].local_addr + zb_device[i].out->packets[j].length;
      s2 = addr;
      e2 = addr + length;
      int c = intersection(s1, e1, s2, e2, &s, &e);
      if (c)
      {
        QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "intersection sucess\n");
        for (n = 0; n < 16; n++)
        {
          if (zb_send[n].device_id == 0xff)
          {
            zb_send[n].sn = get_new_sn(i);
            zb_send[n].device_id = i;
            zb_send[n].packet_index = j;
            zb_send[n].address = s;
            zb_send[n].length = e - s;
            zb_send[n].next_try_time = 0;
            zb_send[n].try_times = 0;
            zb_device[i].out_pending++;
            ret = 1;
            break;
          }
        }
      }
      else
      {
        QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "intersection fail\n");
      }
    }
  }
  return ret;
}

static uint8_t calc_sum(uint8_t *buf, int len)
{
  uint8_t sum = 0;
  int i;
  for (i = 0; i < len; i++)
  {
    sum += *buf++;
  }
  return sum;
}

int zbapi_send_req(device_t *device)
{
  if (device->protocol == 255)
  {
    uint8_t slave = device->in->packets[device->in_index].slave;
    device->txbuf[0] = 0x54;
    device->txbuf[1] = 0x50;
    device->txbuf[2] = slave;
    device->txbuf[3] = 0xF1;
    device->txbuf[4] = calc_sum(uart_txbuf, 4);
    device->write(device->txbuf, 5);
  }
  else
  {
    packet_t *in_pack = &device->in->packets[device->in_index];
    if (in_pack->device_type == 255)
    {
      uint8_t slave = device->in->packets[device->in_index].slave;
      device->txbuf[0] = 0x54;
      device->txbuf[1] = 0x50;
      device->txbuf[2] = slave;
      device->txbuf[3] = 0xF1;
      device->txbuf[4] = calc_sum(uart_txbuf, 4);
      device->write(device->txbuf, 5);
    }
    else
    {
      uint16_t crc;
      device->txbuf[0] = device->in->packets[device->in_index].slave;
      device->txbuf[1] = device->in->packets[device->in_index].funcode;
      device->txbuf[2] = device->in->packets[device->in_index].address >> 8;
      device->txbuf[3] = device->in->packets[device->in_index].address & 0xff;
      device->txbuf[4] = device->in->packets[device->in_index].length >> 8;
      device->txbuf[5] = device->in->packets[device->in_index].length & 0xff;
      crc = crc16(device->txbuf, 6);
      device->txbuf[6] = crc >> 8;
      device->txbuf[7] = crc & 0xff;
      device->write(device->txbuf, 8);
    }
  }
  gWordVar[507]++;
  return 0;
}

int zbapi_send_write(device_t *device, send_t *sender)
{
  {
    int n = 0;
    uint16_t crc;
    const packet_t *out_pack = &device->out->packets[sender->packet_index];
    if (sender->length == 0)
    {
      return 0;
    }
    device->txbuf[0] = device->out->packets[sender->packet_index].slave;
    if (sender->length > 1)
    {
      int addr = out_pack->address + (sender->address - out_pack->local_addr);
      int i;
      int len;
      device->txbuf[1] = 0x10;
      device->txbuf[2] = addr >> 8;
      device->txbuf[3] = addr & 0xff;
      device->txbuf[4] = sender->length >> 8;
      device->txbuf[5] = sender->length & 0xff;
      device->txbuf[6] = (sender->length * 2) & 0xff;
      len = 7;
      for (i = 0; i < sender->length; i++)
      {
        uint16_t reg16 = gWordVar[sender->address + i];
        if (out_pack->device_type == 5)
        {
          reg16 = bin2bcd(reg16);
        }
        device->txbuf[len++] = reg16 >> 8;
        device->txbuf[len++] = reg16 & 0xff;
      }
      crc = crc16(device->txbuf, len);
      device->txbuf[len++] = crc >> 8;
      device->txbuf[len++] = crc & 0xff;
      n += len;
    }
    else
    {
      int addr = out_pack->address + (sender->address - out_pack->local_addr);
      uint16_t reg16 = gWordVar[sender->address];
      device->txbuf[1] = 0x06;
      device->txbuf[2] = addr >> 8;
      device->txbuf[3] = addr & 0xff;
      if (out_pack->device_type == 1) // 温度传感器设置地址
      {
        device->txbuf[0] = reg16 >> 8; // 源来的地址
        device->txbuf[4] = 0;
        device->txbuf[5] = reg16 & 0xff; // 新设定的地址
      }
      else
      {
        device->txbuf[4] = reg16 >> 8;
        device->txbuf[5] = reg16 & 0xff;
      }
      crc = crc16(device->txbuf, 6);
      device->txbuf[6] = crc >> 8;
      device->txbuf[7] = crc & 0xff;
      n += 8;
    }
    if (device->write != NULL)
    {
      device->write(device->txbuf, n);
    }
  }
  return 1;
}

int com_poll_modbus_master_on_revice(int ch, uint8_t *rxbuf, int length)
{
  QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "enter com_poll_modbus_master_on_revice\n");
  int i;
  // uint8_t src_addr[8];
  if (length < 4)
  {
    return -1;
  }
  i = ch;
  // for(i=0; i<ZB_CHANNEL; i++) // 1
  {
    device_t *device = &zb_device[i];
    if (device->status == 2)
    {
     device->revice_resp_time = GetTicks();
      {
        int index = zb_device[i].in_index;
        packet_t *in_packet = &device->in->packets[index];

        if (in_packet->device_type == 255)
        {
          if ((rxbuf[0] == 0x54) && (rxbuf[1] == 0x50) && (rxbuf[6] == calc_sum(rxbuf, 6)))
          {
            uint16_t temp = (rxbuf[4] << 8) | rxbuf[5];
            uint8_t addr = rxbuf[2];
            if (addr == in_packet->slave)
            {
              int16_t max_temp = get_temp_max(addr - 1, temp);
              if (max_temp != INVAILD_TEMP)
              {
                gWordVar[in_packet->local_addr] = max_temp;
              }
              LOGD("temp %d=%d\n", addr, max_temp);
            }
            device->in_err_cnt[device->in_index] = 0;
          }
          else
          {
            LOGD("temp %d packed error\n", in_packet->slave);
          }
        }
        else
        {

          if ((device->in->packets[index].slave == rxbuf[0]) &&
              device->in->packets[index].funcode == rxbuf[1])
          {
            uint16_t crcChk = crc16(rxbuf, length - 2);
            uint16_t crcData = (rxbuf[length - 2] << 8) | rxbuf[length - 1];
            if (crcData != crcChk)
            {
              device->ErrCode = 2; // CRC error
              device->ErrCount++;
            }
            else if (rxbuf[0] != device->in->packets[index].slave)
            {
              device->ErrCode = 3; // ADD error
              device->ErrCount++;
            }
            else if (rxbuf[1] != device->in->packets[index].funcode)
            {
              device->ErrCode = 4; // Fun err;
              device->ErrCount++;
            }
            else if (rxbuf[2] != (device->in->packets[index].length * 2))
            {
              device->ErrCode = 1; // byte miss
              device->ErrCount++;
            }
            else
            {
              int n = 0;
              // uint32_t reg32;
              uint16_t addr = device->in->packets[index].local_addr;
              uint8_t len = rxbuf[2] / 2;
              if (addr >= gWORD_SIZE)
              {
                device->ErrCode = 5; // regaddr err;
                goto done;
              }
              device->ErrCode = 0;
              // gWordVar[508]++;
              switch (device->in->packets[index].device_type)
              {

              case 2:
              {
                int offset = addr - REG_TEMPS;
                int16_t temp = (rxbuf[3] << 8) | (rxbuf[4]);
                int16_t max_temp = get_temp_max(offset, temp);
                if (max_temp != INVAILD_TEMP)
                {
                  gWordVar[addr] = max_temp;
                }
              }
              break;
              case 3:
              {
                int16_t v = (rxbuf[3] << 8) | (rxbuf[4]);
                gWordVar[addr + 1] = v;
                if (v == 0)
                {
                  break;
                }
                else if (v > 12000)
                {
                  gWordVar[REG_VIBRATE_SENSOR] = 2;
                }
                else if (v > 6000)
                {
                  gWordVar[REG_VIBRATE_SENSOR] = 1;
                }
                else
                {
                  gWordVar[REG_VIBRATE_SENSOR] = 0;
                }
                int16 v2 = v - 4000;
                if (v2 < 0)
                {
                  v2 = 0;
                }
                gWordVar[addr] = v2 / 100;
              }
              break;
              default:
                if (zb_device[n].out_pending > 0)
                {
                  break;
                }
                for (n = 0; n < len; n++)
                {
                  if (addr + n < gWORD_SIZE)
                  {
                    gWordVar[addr + n] = (rxbuf[3 + n * 2] << 8) | (rxbuf[3 + n * 2 + 1]);
                  }
                  else
                  {
                    break;
                  }
                }
                break;
              }
              if (device->in->packets[index].cb)
              {
                device->in->packets[index].cb(&gWordVar[addr], len);
              }
              device->in_err_cnt[device->in_index] = 0;
            }
          done:
            device->status = 3;
            // LSAPI_Log_Debug(LOG_TAG "ErrCode:%d\n", device->ErrCode);
          }
          else
          {
            // LSAPI_Log_Debug(LOG_TAG "slave:%d, rxbuf[0]:%d, funcode:%d, rxbuf[1]:%d\n",
            //                 device->in->packets[device->in_index].slave, rxbuf[0],
            //                 device->in->packets[device->in_index].funcode, rxbuf[1]);
          }
          gWordVar[510] = device->ErrCode;
          gWordVar[509] = device->ErrCount;
        }
      }
    }
    else if (device->status == 0x10)
    {
      // LSAPI_Log_Debug(LOG_TAG "status:0x10\n");
      device->revice_resp_time = GetTicks();
      // if(memcmp(src_addr,((zb_dev_t*)device->ext)->zb_addr,8) == 0)
      {
        device->status = 0x11;
      }
    }
    else
    {
      // LSAPI_Log_Debug(LOG_TAG "state:%d\n", device->status);
    }
  }
  return 0;
}

// const int reg_offset[] = {0, 2, 3, 4, 1};
// static char input_buf[1][414];
// static char output_buf[1][212];
static char in_err_cnt_buf[1][25];

// static uint8_t uart_rxbuf[256];

extern drvUart_t *uart1; // LSAPI_Device_t *uart1;

user_data_t user_data; // modbus处理线程与ui线程通信的数据缓冲区

extern osiThread_t *LV_TASK_THREAD_HANDLE; // extern LSAPI_OSI_Thread_t *LV_TASK_THREAD_HANDLE; ui处理线程
extern osiThread_t *gps_thread;            // extern LSAPI_OSI_Thread_t *gps_thread;             gps数据处理线程
// extern gps_t gps_parse_1;                         // gps1解析数据缓冲区
extern osiMessageQueue_t *t2n_report_msg; // extern LSAPI_OSI_MessageQueue_t *t2n_report_msg;  // t2n report消息队列
extern osiSemaphore_t *t2n_report_sema;   // extern LSAPI_OSI_Semaphore_t *t2n_report_sema;     t2n report信号量

extern osiThread_t *LV_TASK_THREAD_HANDLE; // extern LSAPI_OSI_Thread_t *LV_TASK_THREAD_HANDLE; ui处理线程
extern osiThread_t *gps_thread;            // extern LSAPI_OSI_Thread_t *gps_thread;             gps数据处理线程
// extern gps_t gps_parse_1;                         // gps1解析数据缓冲区
extern osiMessageQueue_t *t2n_report_msg; // extern LSAPI_OSI_MessageQueue_t *t2n_report_msg;  // t2n report消息队列
extern osiSemaphore_t *t2n_report_sema;   // extern LSAPI_OSI_Semaphore_t *t2n_report_sema;     t2n report信号量

// static t2n_report_t t2n_report; // t2n_report 上报数据缓冲区
//  static uint16_t last_cnt = 0;
//  static int first_working = true;
//  #define COM_POLL_APP_UART_RECV_BUF_LEN 512                    // app uart接收缓冲区长度
//  static uint8_t app_input_str[COM_POLL_APP_UART_RECV_BUF_LEN]; // appuart modbus接收缓冲区
int16_t g_depth_last = -1;

//-----------------------------------------------------------------------------------------------------
#define FLOW1_REG_ADDR (0)
#define FLOW2_REG_ADDR (6)
#define DEPTH_REG_ADDR (12)
#define RECORD_REG_ADDR (32)
#define AC_CURRENT_REG_ADDR_1 24
#define AC_CURRENT_REG_ADDR_2 25
#define AC_CURRENT_REG_ADDR_3 26
#define MOVE_REG_ADDR 27 // 3
#define TILT_SENSER_ADDR_X 30
#define TILT_SENSER_ADDR_Y 31
#define TILT_ZHUANGDIAN_NUM_ADDR_Y 32

// #define CAL_4_20MA_ADDR 384

typedef struct
{
  int16_t flow_;
  int16_t flow;       // 瞬时流量
  int32_t total_flow; // 累计累计流量
  uint32_t update_time;
} flow_t;
// flow_t *pflow = (flow_t *)&gWordVar[FLOW_REG_ADDR];
typedef struct
{
  int16_t up_down;
  int16_t speed;
  int16_t depth;
  uint16_t sample_count;
  uint16_t depth_flow[2]; // flow1/2
  uint32_t last_total_flow[2];
  int16_t depth_offset;
} depth_t;

typedef struct
{
  uint16_t reset_cmd;
  uint16_t pile_id;
  uint16_t count;
  uint16_t work_time;
  struct _item
  {
    int16_t speed;
    int16_t depth;
    int16_t flow[2];
    uint32_t total_flow[2];
    int16_t tilt_x;
    int16_t tilt_y;
    uint16_t current1;
    uint16_t current2;
  } item[10];
} record_t;

// static uint32_t date_time2utc(struct minmea_date *date, struct minmea_time *time)
// {
//   struct tm gps_time;
//   uint32_t gps_utc;
//   gps_time.tm_hour = time->hours;
//   gps_time.tm_min = time->minutes;
//   gps_time.tm_sec = time->seconds;
//   gps_time.tm_year = date->year + 2000 - 1900;
//   gps_time.tm_mon = date->month - 1;
//   gps_time.tm_mday = date->day;
//   gps_utc = mktime(&gps_time);
//   return gps_utc;
// }
// depth_t *depth_data = (depth_t *)&gWordVar[DEPTH_REG_ADDR];
// modbus排布 = (1 + 2通道flow_t) + (深度depth_t) +
//-----------------------------------------------------------------------------------------------------
extern user_data_t g_ui_user_data;
/**
 * @brief modbus接收的ui数据处理
 *
 * @param modbus_recv_reg
 * @param b
 */
extern int drp_write(uint8_t endpoint, uint8_t flag, uint16_t tran_id, void *data, int length);
uint16_t tran_id = 0;
uint16_t last_record_count = 0;
#if 0
static void recv_ui_data_cb(uint16_t *modbus_recv_reg, int b)
{
  if (modbus_recv_reg == NULL)
    return;
  depth_t *depth = (depth_t *)&gWordVar[DEPTH_REG_ADDR];
  flow_t *flow1 = (flow_t *)&gWordVar[FLOW1_REG_ADDR];
  flow_t *flow2 = (flow_t *)&gWordVar[FLOW2_REG_ADDR];

  // ## 1
  // t2n_report_t *temp = &t2n_report;
  // ## WARN: 注意变量类型-类型转换
  g_ui_user_data.ss_1 = flow1->flow;
  g_ui_user_data.ll_1 = flow1->total_flow;
  g_ui_user_data.ss_2 = flow2->flow;
  g_ui_user_data.ll_2 = flow2->total_flow;
  g_ui_user_data.depth = depth->depth;
  g_ui_user_data.speed = depth->speed;
  g_ui_user_data.Ia = (int16_t)gWordVar[AC_CURRENT_REG_ADDR_1];
  g_ui_user_data.Ib = (int16_t)gWordVar[AC_CURRENT_REG_ADDR_2];
  g_ui_user_data.Ic = (int16_t)gWordVar[AC_CURRENT_REG_ADDR_3];
  g_ui_user_data.angle_x = (int16_t)gWordVar[TILT_SENSER_ADDR_X];
  g_ui_user_data.angle_y = (int16_t)gWordVar[TILT_SENSER_ADDR_Y];

  // user_data.ss_1 = flow1->flow;       // 1通道瞬时流量
  // user_data.ll_1 = flow1->total_flow; // 1通道累计流量
  // flow_t *flow2 = (flow_t *)&modbus_recv_reg[FLOW2_REG_ADDR];
  // user_data.ss_2 = flow2->flow;
  // user_data.ll_2 = flow2->total_flow;

  // user_data.depth = depth->depth; // 深度
  // user_data.speed = depth->speed; // 速度

  // user_data.angle_x = (int16_t)modbus_recv_reg[TILT_SENSER_ADDR_X];
  // user_data.angle_y = (int16_t)modbus_recv_reg[TILT_SENSER_ADDR_Y];

  // log_printf(LINFO, LOG_TAG "ss_1:%d, ll_1:%d, ss_2:%d, ll_2:%d, depth:%d, speed:%d, angle_x:%d, angle_y:%d\n",
  //            user_data.ss_1, user_data.ll_1, user_data.ss_2, user_data.ll_2, user_data.depth,
  //            user_data.speed, user_data.angle_x, user_data.angle_y);

  /*! @note: 向ui界面发送用户数据 */
  // if (LV_TASK_THREAD_HANDLE != NULL) {
  //   LSAPI_OSI_Event_t send_event;
  //   send_event.id = BK_EVENT_UI_USER_DATA_ID;
  //   send_event.param1 = (uint32_t)&user_data;
  //   LSAPI_OSI_EvnetSend(LV_TASK_THREAD_HANDLE, &send_event);
  // }

  /*! @note: 向t2n上报数据 */
  record_t *record = (record_t *)&gWordVar[RECORD_REG_ADDR];
  if (record->count != last_record_count)
  { // sample_count数据变化时需要对t2n数据进行上报处理
  //flow1,2  times  TS
    t2n_report_t *temp = &t2n_report;
    last_record_count = record->count;
    temp->times = record->count;
    // temp->timeline = record->;
    temp->current1 = gWordVar[AC_CURRENT_REG_ADDR_1];
    temp->current2 = gWordVar[AC_CURRENT_REG_ADDR_2];
    temp->depth = record->item[0].depth;
    //temp->speed = record->item[0].speed;
    temp->flow1 = record->item[0].flow[0];
    //temp->toatal_flow1 = record->item[0].total_flow[0];
    temp->flow2 = record->item[0].flow[1];
    //temp->toatal_flow2 = record->item[0].total_flow[1];
    temp->pile_id = record->pile_id;
    temp->tilt_x = user_data.angle_x;
    temp->tilt_y = user_data.angle_y;
    temp->LNG = gps_parse_1.LNG % 0x100000000ul;
    //temp->LNG_H = gps_parse_1.LNG / 0x100000000ul;
    temp->LAT = gps_parse_1.LAT % 0x100000000ul;
    //temp->LAT_H = gps_parse_1.LAT / 0x100000000ul;
    temp->ALT = gps_parse_1.ALT % 0x10000;
    //temp->ALT_H = gps_parse_1.ALT / 0x10000;
    temp->Q = gps_parse_1.Q;
    temp->azimuth = gps_parse_1.azimuth;
    //temp->UTC = date_time2utc(&gps_parse_1.date, &gps_parse_1.time);
    drp_write(50, 2, ++tran_id, temp, sizeof(t2n_report_t));
    //log_printf(LINFO, LOG_TAG "drp_write ep=%d,len=%d,alt=%d\n", 50, sizeof(t2n_report_t), gps_parse_1.ALT);
    // ##
    // if ((t2n_report_sema != NULL) && (t2n_report_msg != NULL))
    // { // 放到消息队列中，进行t2n数据上报
    //   LSAPI_OSI_SemaphoreRelease(t2n_report_sema);
    //   LSPAI_OSI_MessageQueuePut(t2n_report_msg, (void *)(temp));
    // }
    // if (LV_TASK_THREAD_HANDLE != NULL)
    // { // 向ui界面发送t2n上报数据
    //   LSAPI_OSI_Event_t send_event;
    //   send_event.id = BK_EVENT_T2N_REPORT_ID;
    //   send_event.param1 = (uint32_t)temp;
    //   LSAPI_OSI_EvnetSend(LV_TASK_THREAD_HANDLE, &send_event);
    // }

    // g_depth_last = depth_val;
  } // 数据没有变化时什么也不做

  // ## 2
  // if ((bl0939_sema != NULL) && (bl0939_msg != NULL)) {
  //   LSAPI_OSI_SemaphoreRelease(bl0939_sema);
  //   LSPAI_OSI_MessageQueuePut(bl0939_msg, (void *)(modbus_recv_reg + 11));
  // }
}
#endif
struct input
{
  input_t input;
  packet_t packet[5];
};
struct output
{
  output_t output;
  packet_t packet[5];
};
struct input input1;
struct output output1;
/**
 * @brief com_poll任务初始化
 *
 */
extern int uart1_send(uint8_t *buf, int len);
void com_poll_init(void)
{
  QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "enter com_poll_init\n");
  int i = 0;
  // int meter_count[4] = {0, 0, 0, 0};
  // int size;
  // char *p;
  // int n;
  uint8_t *in_err_cnt;
 
  zb_device[0].txbuf = uart_txbuf;
  zb_device[0].txbuf_size = sizeof(uart_txbuf);
  zb_device[0].write = uart1_send; // return 8
  QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "zb_device[0].write = %d\n", zb_device[0].write);
  {
    input_t *input;
    output_t *output;
    // uint8_t *in_err_cnt;

    // int tmp1 = sizeof(input_buf[i]);
    // int tmp2 = sizeof(output_buf[i]);
    // int tmp3 = sizeof(in_err_cnt_buf[i]);

    // memset(input_buf[i], 0, sizeof(input_buf[i]));
    // memset(output_buf[i], 0, sizeof(output_buf[i]));
    // memset(in_err_cnt_buf[i], 0, sizeof(in_err_cnt_buf[i]));
    memset(&output1, 0, sizeof(output1));
    memset(&input1, 0, sizeof(input1));
    memset(in_err_cnt_buf, 0, sizeof(in_err_cnt_buf));
    output = &output1.output; //(output_t *)output_buf[i];
    input = &input1.input;    // (input_t *)input_buf[i];
    in_err_cnt = (uint8_t *)in_err_cnt_buf[i];

    zb_device[i].in_err_cnt = in_err_cnt;

    input->scan_rate = 20; //*10ms 扫描周期时间
    input->time_out = 2;   //*100ms 超时时间
    input->max_try_times = 2;
    input->scan_space = 5; //*10ms
    input->max_err_cnt = 20;

    output->max_try_times = 3;
    output->try_space = 20;

    zb_device[i].channl = i;
    zb_device[i].next_scan_time = 0;
    zb_device[i].status = 0;
    zb_device[i].try_times = 0;
    zb_device[i].in_index = 0;
    zb_device[i].out_pending = 0;
    zb_device[i].in_err_cnt = in_err_cnt;
    zb_device[i].in = input;
    zb_device[i].out = output;
    // zb_device[i].ext = &serial_port_cfg[i];
  }
  {
    // 研华温度传感器
    device_t *dev = (device_t *)&zb_device;
    gWordVar[REG_TEMPS] = -999;

    // dev->in->packets[dev->in->packet_cnt].slave = 1;
    // dev->in->packets[dev->in->packet_cnt].funcode = 3;
    // dev->in->packets[dev->in->packet_cnt].address = 0;
    // dev->in->packets[dev->in->packet_cnt].length = 1;
    // dev->in->packets[dev->in->packet_cnt].device_type = 3;
    // dev->in->packets[dev->in->packet_cnt].local_addr = REG_AMPLITUDE;
    // dev->in->packets[dev->in->packet_cnt].ch = 0;
    // dev->in->packets[dev->in->packet_cnt].cb = NULL;
    // dev->in->packet_cnt++;

    // 温度传感器地址
    dev->out->packets[dev->out->packet_cnt].slave = 1;
    dev->out->packets[dev->out->packet_cnt].funcode = 6;
    dev->out->packets[dev->out->packet_cnt].local_addr = 2000;
    dev->out->packets[dev->out->packet_cnt].length = 1;
    dev->out->packets[dev->out->packet_cnt].address = 2000;
    dev->out->packets[dev->out->packet_cnt].device_type = 0;
    dev->out->packets[dev->out->packet_cnt].ch = 0;
    dev->out->packet_cnt++;

    
    // gps坐标转换信息
    dev->in->packets[dev->in->packet_cnt].slave = 1;
    dev->in->packets[dev->in->packet_cnt].funcode = 3;
    dev->in->packets[dev->in->packet_cnt].address = REG_GPS_WRITE_DATA;
    dev->in->packets[dev->in->packet_cnt].length = 50;
    dev->in->packets[dev->in->packet_cnt].device_type = 0;
    dev->in->packets[dev->in->packet_cnt].local_addr = REG_GPS_WRITE_DATA;
    dev->in->packets[dev->in->packet_cnt].ch = 0;
    dev->in->packets[dev->in->packet_cnt].cb = modbus_write_gps_data_cb;
    dev->in->packet_cnt++;
  }


  
  for (int i = 0; i < 16; i++)
  {
    zb_send[i].device_id = 0xff;
  }
}

int fill_data(uint8_t *pData, uint16_t local_addr, uint8_t remote_offset, uint8_t length)
{
  uint16_t *pWordVar;
  int i;
  *pData++ = remote_offset;
  *pData++ = length;
  pWordVar = &gWordVar[local_addr];
  for (i = 0; i < length; i++)
  {
    *pData++ = *pWordVar >> 8;
    *pData++ = *pWordVar & 0xff;
    pWordVar++;
  }
  return i * 2 + 2;
}

int build_data(int dest_addr, const int *src_addr_map, int length)
{
  int i;
  int change_cnt = 0;
  for (i = 0; i < length; i++)
  {
    int src_addr = src_addr_map[i];
    if (dest_addr < gWORD_SIZE && src_addr >= 0 && src_addr < gWORD_SIZE)
    {
      if (gWordVar[dest_addr] != gWordVar[src_addr])
      {
        gWordVar[dest_addr] = gWordVar[src_addr];
        change_cnt++;
      }
    }
    else
    {
      return -1;
    }
    dest_addr++;
  }
  return change_cnt;
}

int copy_data(int dest_addr, int src_addr, int length)
{
  int i;
  int change_cnt = 0;
  for (i = 0; i < length; i++)
  {
    // int src_addr = src_addr_map[i];
    if (dest_addr < gWORD_SIZE && src_addr >= 0 && src_addr < gWORD_SIZE)
    {
      if (gWordVar[dest_addr] != gWordVar[src_addr])
      {
        gWordVar[dest_addr] = gWordVar[src_addr];
        change_cnt++;
      }
    }
    else
    {
      return -1;
    }
    dest_addr++;
    src_addr++;
  }
  return change_cnt;
}
