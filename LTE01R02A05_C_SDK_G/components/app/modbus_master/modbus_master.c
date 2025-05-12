

#include "osi_api.h"
#include "stdlib.h"
#include "drv_uart.h"
// #include "t2n.h"

#include "ModbusM.h"
#include "ql_api_osi.h"
#define LOG_TAG "[modbus_slave]:"
#include "ql_log.h"
#include "../esp32_flash/inc/esp32_flash.h"

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, msg, ##__VA_ARGS__)
#define UART_NAME DRV_NAME_UART2

osiThread_t *modbus_slave_thread = NULL;
static drvUart_t *uart = NULL;
static uint8_t uart_txbuf[256];
static uint8_t uart_rxbuf[256];

extern uint16_t gWordVar[];

static void uart_event_cb(void *param, uint32_t evt)
{
  QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "enter uart1_event_cb\n");
  if (evt & DRV_UART_EVENT_TX_COMPLETE) // 发送完成
  {
    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "QL_UART_EVENT_TX_COMPLETE\n");
  }
  else if (evt & DRV_UART_EVENT_RX_ARRIVED) // 接收完成
  {
    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "QL_UART_EVENT_RX_ARRIVED\n");
    osiEvent_t event;
    event.id = 9; //
    if (modbus_slave_thread != NULL)
    {
      bool sd = osiEventSend(modbus_slave_thread, &event);
      if (sd)
      {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "osiEventSend sucess\n");
      }
      else
      {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "osiEventSend fail\n");
      }
    }
  }
  // else(evt & QL_UART_EVENT_RX_OVERFLOW)
  // {
  //     QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG ,"QL_UART_EVENT_RX_OVERFLOW\n");
  // }
}

// extern int ESP32DownloadFile(char *filename, int32_t address);
// extern int http_dowload(char *phttp_url, char *file_patch);
void modbus_slave_task(void *param)
{

  int ret = 0;
  int run_loop = 1;
  int led_reset_cnt = 0;
  int led_dog_cnt = 0;
  int led_dog_timeout = 3;
  drvUartCfg_t uart_cfg = {
      .baud = 115200,
      .data_bits = 1,
      .stop_bits = 8,
      .parity = DRV_UART_NO_PARITY,
      .auto_baud_lc = 0,
      .cts_enable = 0,
      .rts_enable = 0,
      .rx_buf_size = 256,
      .tx_buf_size = 256,
      .event_mask = DRV_UART_EVENT_RX_ARRIVED | DRV_UART_EVENT_RX_OVERFLOW | DRV_UART_EVENT_TX_COMPLETE,
      .event_cb = uart_event_cb,
  };
restart:
  uart = drvUartCreate(UART_NAME, &uart_cfg);
  if (uart == NULL)
  {
    LOGE("uart%d create faild!\n", UART_NAME >> 24);
    goto err1;
  }
  ret = drvUartOpen(uart);
  if (!ret)
  {
    LOGE("uart%d open faild!\n", UART_NAME >> 24);
    goto err1;
  }
  LOGI("uart%d init success\n", UART_NAME >> 24);
  modbus_slave_thread = osiThreadCurrent();
  run_loop = 1;
  while (run_loop)
  {
    osiEvent_t waitevent;
    memset(&waitevent, 0, sizeof(osiEvent_t));
    bool ret = osiEventTryWait(modbus_slave_thread, &waitevent, 1000);
    if (ret)
    { // 响应到事件发生
      LOGI("mudbus event:0x%x", waitevent.id);
      switch (waitevent.id)
      {
        case 9:
        {
          int len = drvUartReceive(uart, uart_rxbuf, sizeof(uart_rxbuf));
          LOGI("uart rx=%d bytes", len);
          if (len > 0)
          {
            int tx_len = modbus_master_on_revice(0, uart_rxbuf, len);
            if (tx_len > 0)
            {
            //   int ret = drvUartSend(uart, uart_txbuf, tx_len);
              LOGI("uart tx=%d ret=%d", ret, tx_len);
            //   led_dog_timeout = 10;
            }
          }
          else
          {
              modbus_master_poll(0);
          }
          led_dog_cnt = 0;
          break;
        }
        case ESP32_FLASH_COMMAND_HTTP_DOWNLOAD:
          if (waitevent.param1 != 0 && waitevent.param2 != 0)
          {
            esp32_flash_http_download_command((char *)waitevent.param1, (char *)waitevent.param2);
            free(waitevent.param1);
          }
          break;
        case ESP32_FLASH_COMMAND_ESP32_FLASH:
          drvUartClose(uart);
          drvUartDestroy(uart);
          esp32_flash_flash_command();
          run_loop = 0;
          break;
        case ESP32_FLASH_COMMAND_ESP32_RESET:
          esp32_flash_reset_target_command();
          break;
        default:
          break;
      }
      waitevent.id = 0;
    }
    else
    {
      if (++led_dog_cnt > led_dog_timeout && led_reset_cnt < 5)
      {
        led_dog_cnt = 0;
        // setESP32BootValue(0);
        // LSAPI_OSI_ThreadSleep(100);
        // loader_port_reset_target();
        // LSAPI_OSI_ThreadSleep(100);
        // setESP32BootValue(1);
        led_dog_timeout = 5;
        led_reset_cnt++;
      }
    }
  }

  goto restart;
err1:
  osiThreadExit();
}

void modbus_rtu_slave_task_init(void)
{
  QlOSStatus err = 0;
  ql_task_t task_ref = NULL;
  err = ql_rtos_task_create(&task_ref, 4096, APP_PRIORITY_NORMAL-2, "mbslave", modbus_slave_task, NULL, 5);
  if (err != QL_OSI_SUCCESS)
  {
    LOGE("creat rs485_task fail err = %d", err);
  }
  LOGI("creat rs485_task sucess!");
}