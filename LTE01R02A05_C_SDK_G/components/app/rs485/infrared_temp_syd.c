
#include <stdlib.h>
#include "ql_api_osi.h"
#include "drv_uart.h"
#include "ql_gpio.h"
#include "osi_api.h"
#include "t2n.h"
#include "ModbusS.h"
#define LOG_TAG "[rs485]:"
#include "../t2n/include/cfg.h"
#include "ql_log.h"

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, msg, ##__VA_ARGS__)

#define RS485_UART_NAME DRV_NAME_UART1
#define RS485_RT_GPIO 3
#define RS485_RT_LVL_TX 1
#define RS485_RT_LVL_RX 0

ql_task_t rs485_task_ref;

static drvUart_t *uart = NULL;
static osiSemaphore_t *tx_sema;
static osiSemaphore_t *rx_sema;

static uint8_t uart_txbuf[16];
static uint8_t uart_rxbuf[16];

extern uint16_t gWordVar[];

static char hex[1024];

#define TEMP_WINDOWS_SIZE (10)
#define INVAILD_TEMP (-999)
int16_t get_temp_max(int16_t temp)
{
  static int16_t temp_window[TEMP_WINDOWS_SIZE] = {0};
  static int w_index = 0;
  int16_t max_temp = INVAILD_TEMP; // -99.9
  temp_window[w_index] = temp;
  if (++w_index >= TEMP_WINDOWS_SIZE)
  {
    w_index = 0;
  }
  for (int i = 0; i < TEMP_WINDOWS_SIZE; i++)
  {
    if (temp_window[i] > max_temp)
    {
      max_temp = temp_window[i];
    }
  }
  return max_temp;
}

char *bin2hex(uint8_t *bin, int lenght)
{

  if (lenght > sizeof(hex) / 3)
  {
    lenght = sizeof(hex) / 3;
  }
  int i;
  for (i = 0; i < lenght; i++)
  {
    sprintf(hex + i * 3, "%02X ", bin[i]);
  }
  hex[i * 3] = '\0';
  return hex;
}
static void uart_event_cb(void *param, uint32_t evt)
{
  if (evt & DRV_UART_EVENT_TX_COMPLETE) // 发送完成
  {
    // ql_gpio_set_level(RS485_RT_GPIO, RS485_RT_LVL_RX);
    osiSemaphoreRelease(tx_sema);
  }
  if (evt & (DRV_UART_EVENT_RX_ARRIVED | DRV_UART_EVENT_RX_OVERFLOW))
  {
    osiSemaphoreRelease(rx_sema);
  }
}

static int rs485_write_bytes(const uint8_t *data, uint16_t size, uint32_t timeout)
{
  if (!uart)
  {
    LOGE("uart_write_bytes uart is null");
    return -9;
  }
  LOGD(LOG_TAG "uart_write %d bytes data=[%s]", size, bin2hex(data, size));
  int send_size;
  int ret;
  ql_gpio_set_level(RS485_RT_GPIO, RS485_RT_LVL_TX);
  send_size = drvUartSend(uart, data, size);
  ret = osiSemaphoreTryAcquire(tx_sema, timeout);
  ql_delay_us(100); // 150
  ql_gpio_set_level(RS485_RT_GPIO, RS485_RT_LVL_RX);
  if (ret == 0)
  {
    LOGE("uart_write_bytes timeout");
    return -1;
  }
  return send_size;
}

int rs485_read_bytes(uint8_t *data, uint16_t size, uint32_t timeout)
{
  if (!uart)
  {
    LOGE("uart_write_bytes uart is null");
    return -9;
  }
  int actual_bytes = 0;
  int ret;
  uint32_t tick = ql_rtos_get_system_tick();
  do
  {
    ret = osiSemaphoreTryAcquire(rx_sema, timeout);
    int read = drvUartReceive(uart, &data[actual_bytes], size - actual_bytes);
    if (read > 0)
    {
      actual_bytes += read;
    }
    if (ret == 0)
    {
      break;
    }
    timeout = 10;
  } while (actual_bytes < size);
  LOGD("uart_read want %d bytes actual %d bytes use %d tick", size, actual_bytes, ql_rtos_get_system_tick() - tick);
  if (actual_bytes == 0)
  {
    return -2;
  }
  LOGD(LOG_TAG "uart_read %d bytes data=[%s]", actual_bytes, bin2hex(data, actual_bytes));
  return actual_bytes;
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

#define TEMP_COUNT 1
int uart_flag = 0;
void rs485_task(void *param)
{

  int ret = 0;
  uint8_t timeout_cnt1 = 0;
  uint8_t timeout_cnt2 = 0;
  uint8_t slave = 1;
  uint16_t temp_max[TEMP_COUNT] = {0};
  uint16_t temp_count[TEMP_COUNT];
 
  drvUartCfg_t uart_cfg = {
      .baud = 9600,
      .data_bits = 1,
      .stop_bits = 8,
      .parity = DRV_UART_NO_PARITY,
      .auto_baud_lc = 0,
      .cts_enable = 0,
      .rts_enable = 0,
      .rx_buf_size = 256,
      .tx_buf_size = 256,
      .event_mask = DRV_UART_EVENT_TX_COMPLETE | DRV_UART_EVENT_RX_ARRIVED | DRV_UART_EVENT_RX_OVERFLOW,
      .event_cb = uart_event_cb,
  };
  tx_sema = osiSemaphoreCreate(1, 0);
  rx_sema = osiSemaphoreCreate(1, 0);

  ql_gpio_set_level(GPIO_3, 1); // 5V en

  uart = drvUartCreate(RS485_UART_NAME, &uart_cfg);
  if (uart == NULL)
  {
    LOGE("uart%d create faild!\n", RS485_UART_NAME >> 24);
    goto err1;
  }
  ret = drvUartOpen(uart);
  if (!ret)
  {
    LOGE("uart%d open faild!\n", RS485_UART_NAME >> 24);
    goto err1;
  }
  LOGI("uart%d init success\n", RS485_UART_NAME >> 24);
  for (int i = 0; i < TEMP_COUNT; i++)
  {
    temp_count[i] = 10;
  }
  while (1)
  {
    int rx_len;
    if (uart_flag == 0) // 读温度
    {
      uart_txbuf[0] = 0x54;
      uart_txbuf[1] = 0x50;
      uart_txbuf[2] = slave;
      uart_txbuf[3] = 0xF1;
      uart_txbuf[4] = calc_sum(uart_txbuf, 4);
      rs485_write_bytes(uart_txbuf, 5, 100);
    }
    else // 读振动
    {
      uart_txbuf[0] = 0x01;
      uart_txbuf[1] = 0x03;
      uart_txbuf[2] = 0x00;
      uart_txbuf[3] = 0x00;
      uart_txbuf[4] = 0x00;
      uart_txbuf[5] = 0x01;
      uint16_t crc = crc16(uart_txbuf, 6);
      uart_txbuf[6] = crc >> 8;
      uart_txbuf[7] = crc & 0xff;
      rs485_write_bytes(uart_txbuf, 8, 100);
    }
    rx_len = rs485_read_bytes(uart_rxbuf, 256, 500);
    if (rx_len > 0)
    {
      if (uart_flag == 0)
      {
        if ((uart_rxbuf[0] == 0x54) && (uart_rxbuf[1] == 0x50) && (uart_rxbuf[6] == calc_sum(uart_rxbuf, 6)))
        {
          uint16_t temp = (uart_rxbuf[4] << 8) | uart_rxbuf[5];
          uint8_t addr = uart_rxbuf[2];
          if (addr == slave)
          {
            int16_t max_temp = get_temp_max(temp);
            if (max_temp != INVAILD_TEMP)
            {
              gWordVar[REG_TEMPS + addr - 1] = max_temp;
            }
            timeout_cnt1 = 0;
            LOGD("temp %d=%d\n", addr, max_temp);
          }
        }
        else
        {
          LOGD("temp %d packed error\n", slave);
        }
        if (++slave > TEMP_COUNT)
        {
          slave = 1;
        }
        uart_flag = 1;
      }
      else
      {
        uint16_t crc16_cal = crc16(uart_rxbuf, 6);
        uint16_t crc16_rx = (uart_rxbuf[6] << 8) | uart_rxbuf[7];
        if (crc16_cal == crc16_rx)
        {
          uint16_t vibrate = (uart_rxbuf[3] << 8) | uart_rxbuf[4];
          int amp = vibrate - 4000;
          if (amp < 0)
          {
            amp = 0;
          }
          gWordVar[REG_AMPLITUDE] = amp / 100;
          if (vibrate > 16000)
          {
            gWordVar[REG_VIBRATE_SENSOR] = 2;
          }
          else if (vibrate > 8000)
          {
            gWordVar[REG_VIBRATE_SENSOR] = 1;
          }
          else
          {
            gWordVar[REG_VIBRATE_SENSOR] = 0;
          }
          timeout_cnt2 = 0;
          LOGD("vibrate %d\n", 1, vibrate);
        }
        else
        {
          // gWordVar[REG_VIBRATE_SENSOR+1] = crc16_cal;
          // gWordVar[REG_VIBRATE_SENSOR+2] = crc16_rx;
          LOGD("vibrate packed crc error cal=%04x,rx=%04x \n", crc16_cal, crc16_rx);
        }
        uart_flag = 0;
      }
    }
    else
    {
      LOGW("temp %d read timeout", slave);
      if (uart_flag == 0)
      {
        if (++slave > TEMP_COUNT)
        {
          slave = 1;
        }
        if (++timeout_cnt1 > 10)
        {
          gWordVar[REG_TEMPS] = INVAILD_TEMP;
          timeout_cnt1 = 0;
        }
        uart_flag = 1;
      }
      else
      {
        if (++timeout_cnt2 > 10)
        {
          gWordVar[REG_VIBRATE_SENSOR] = 0;
          gWordVar[REG_AMPLITUDE] = 0;
          timeout_cnt2 = 0;
        }
        uart_flag = 0;
      }
    }
    osiThreadSleep(100);
  }
err1:
  osiThreadExit();
}

void rs485_task_init(void)
{
  QlOSStatus err = 0;
  err = ql_rtos_task_create(&rs485_task_ref, 4096, APP_PRIORITY_NORMAL, "rs485_task", rs485_task, NULL, 5);
  if (err != QL_OSI_SUCCESS)
  {
    LOGE("creat rs485_task fail err = %d", err);
  }
  LOGI("creat rs485_task sucess!");
}
