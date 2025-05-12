

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_uart.h"
#include "ql_gpio.h"
// #include "ql_pin_cfg.h"
#include "drv_uart.h"
// #include "modbus_master.h"
#include "osi_api.h"

#include "stdlib.h"
#include "t2n.h"
#include "ModbusS.h"

#include "ql_api_datacall.h"
#include "ql_http_client.h"
#include "ql_fs.h"

#include "ql_api_fota.h"
#include "ql_power.h"
#include "ql_api_dev.h"
#include "../t2n/include/cfg.h"

#define LOG_TAG "[uart1_modbus_master]:"
#define EC600U_HTTP_LOG(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "EC600U_HTTP_DLOAD", msg, ##__VA_ARGS__)

osiThread_t *uart1_thread = NULL;
drvUart_t *uart1 = NULL;
// static uint8_t uart_txbuf[256];
static uint8_t uart_rxbuf[256 + 8];

extern uint16_t gWordVar[];
#define RS485_RT_GPIO 3
#define RS485_RT_LVL_TX 1
#define RS485_RT_LVL_RX 0
// static char hex[280 * 3 + 1];

// char *bin2hex(uint8_t *bin, int lenght)
// {

//     if (lenght > sizeof(uart_rxbuf))
//     {
//         lenght = sizeof(uart_rxbuf);
//     }
//     int i;
//     for (i = 0; i < lenght; i++)
//     {
//         sprintf(hex + i * 3, "%02X ", bin[i]);
//     }
//     hex[i * 3] = '\0';
//     return hex;
// }

void Timeroutcallback(void *param)
{
    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "enter Timeroutcallback\n");
    osiEvent_t event;
    event.id = 10;
    bool oo = osiEventSend(uart1_thread, &event);
    if (oo == FALSE)
    {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "osiEventSend event id 10 fail\n");
    }
}

extern int com_poll_modbus_master_on_revice(int ch, uint8_t *rxbuf, int length);
extern int ESP32DownloadFile(char *filename, int32_t address);
extern void com_poll_init(void);
extern void com_poll_modbus_master_poll(int n);
extern int net_status(void);
extern void setESP32BootValue(int value);
extern void loader_port_reset_target(void);
extern void http_dowload(char *event_param1, char *file_name);
extern int AppImageUpgradeFotaEntry(osiEvent_t *waitevent);

int net_monitor = 0;

void uart1_event_cb(void *param, uint32_t evt)
{
    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "enter uart1_event_cb\n");
    if (evt & QL_UART_EVENT_TX_COMPLETE) // 发送完成
    {
        // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "QL_UART_EVENT_TX_COMPLETE\n");
        osiEvent_t event;
        event.id = 8; //
        if (uart1_thread != NULL)
        {
            bool sd = osiEventSend(uart1_thread, &event);
            if (sd == FALSE)
            {
                QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "osiEventSend fail\n");
            }
        }
    }
    else if (evt & (QL_UART_EVENT_RX_ARRIVED | QL_UART_EVENT_RX_OVERFLOW)) // 接收完成
    {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "QL_UART_EVENT_RX_ARRIVED\n");
        osiEvent_t event;
        event.id = 9; //
        if (uart1_thread != NULL)
        {
            bool sd = osiEventSend(uart1_thread, &event);
            if (sd == FALSE)
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

int uart1_send(uint8_t *buf, int len)
{
    // QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "enter com_poll_uart_send\n");
    if (!uart1)
    {
        QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "uart1 device is null\n");
        return -1;
    }
    ql_gpio_set_level(RS485_RT_GPIO, RS485_RT_LVL_TX);
    int ret = drvUartSend((drvUart_t *)uart1, (void *)buf, len);
    if (ret < 0)
    {
        QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "com_poll_uart_send  failed!!, ret:%d\n", ret);
    }
    // else
    // {
    //     QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "com_poll_uart_send  sucess!!, ret:%d\n", ret);
    // }
    return ret;
}

void uart1_task_thread(void *param)
{
    // ql_rtos_task_sleep_s(20);
    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "enter uart1_task_thread!\n");
    int ret = 0;
    int run_loop = 1;
    int rx_length = 0;
    osiTimer_t *ptimer_t = NULL;
    drvUartCfg_t uart_cfg = {
        .baud = 9600,
        .data_bits = DRV_UART_DATA_BITS_8,
        .stop_bits = DRV_UART_STOP_BITS_1,
        .parity = DRV_UART_NO_PARITY,
        .event_cb = uart1_event_cb,
        .event_mask = DRV_UART_EVENT_RX_OVERFLOW | DRV_UART_EVENT_TX_COMPLETE | DRV_UART_EVENT_RX_ARRIVED,
        .rx_buf_size = 128,
        .tx_buf_size = 128,
        .event_cb_ctx = NULL,
        .cts_enable = false,
        .rts_enable = false,
    };
    // ql_gpio_set_level(GPIO_3, 1); // 5V en

    uart1_thread = osiThreadCurrent();
    if (uart1_thread != NULL)
    {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "uart1_thread is not NULL\n");
    }
    else
    {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "uart1_thread = NULL\n");
    }

restart:
    if (uart1 == NULL)
    {
        uart1 = drvUartCreate(DRV_NAME_UART1, &uart_cfg);
        if (uart1 == NULL)
        {
            QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, "uart1 create faild!\n");
            goto err1;
        }
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "uart1 create OK!\n");
        ret = drvUartOpen(uart1); // ret = LSAPI_Device_Open(uart1);
        if (!ret)
        {
            QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, "uart1 open faild!\n");
            goto err1;
        }
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "uart1 open OK!\n");
    }
    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "uart1 init success\n");

    if (ptimer_t == NULL)
    {
        ptimer_t = osiTimerCreate(osiThreadCurrent(), Timeroutcallback, NULL);
        // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "osiTimerCreate success\n");
    }
    else
    {
        // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "exist osiTimer\n");
    }
    bool t = osiTimerStart(ptimer_t, 10);
    if (t == FALSE)
    {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "osiTimerStart fail\n");
    }
    gWordVar[2000] = 2;
    com_poll_init();
    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "com_poll_init success\n");
    run_loop = 1;
    while (run_loop)
    {
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "enter while recycle success\n");
        // com_poll_modbus_master_poll(0);

        osiEvent_t waitevent;
        bool ret = osiEventTryWait(osiThreadCurrent(), &waitevent, 1000); // osiEventTryWait(LSAPI_OSI_ThreadCurrent(), &waitevent, 1000);
        if (ret)
        {

            // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "event happend\n"); // 响应到事件发生
            switch (waitevent.id)
            {
            case 9:
            {
                QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "event id = 9\n");
                int len = drvUartReceive(uart1, &uart_rxbuf[rx_length], sizeof(uart_rxbuf) - rx_length);
                if (len > 0)
                {
                    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "receive data sucess, length is %d\n", len);
                    rx_length += len;
                    if (uart_rxbuf[1] == 0x03 || uart_rxbuf[1] == 0x04)
                    {
                        // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "uart_rxbuf[1] == 0x03 || uart_rxbuf[1] == 0x04\n");
                        if (rx_length >= uart_rxbuf[2] + 5)
                        {
                            // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "rx_length >= uart_rxbuf[2] + 5\n");
                            com_poll_modbus_master_on_revice(0, uart_rxbuf, rx_length);
                            // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "com_poll_modbus_master_on_revice end\n");
                            rx_length = 0;
                        }
                    }
                    else
                    {
                        // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "uart_rxbuf[1] != 0x03 && uart_rxbuf[1] != 0x04\n");
                        com_poll_modbus_master_on_revice(0, uart_rxbuf, rx_length);
                        // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "com_poll_modbus_master_on_revice end\n");
                        rx_length = 0;
                    }
                }
                else
                {
                    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "receive data timeout\n");
                }
            }
            break;
            case 10: // 10ms timmer
            {
                // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "event id = 10\n");
                bool u = osiTimerStart(ptimer_t, 10);
                if (u == FALSE)
                {
                    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "osiTimerStart fail\n");
                }
                com_poll_modbus_master_poll(0);
                // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "com_poll_modbus_master_poll end\n");
                int status = net_status();
                if (status < 4)
                {
                    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "net_status < 4, status = %d\n", status);
                    net_monitor++;
                    if (net_monitor > 100 * 60 * 5)
                    {
                        net_monitor = 0;
                    }
                }
                else
                {
                    // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "net_status > status = %d\n", status);
                    net_monitor = 0;
                }
            }
            break;
            case 8:
            {
                ql_delay_us(200); // 发送完成事件产生时最后一个字节尚未发送完成，需要等待一段时间,时间需要用示波器测量
                ql_gpio_set_level(RS485_RT_GPIO, RS485_RT_LVL_RX);
                break;
            }
            case 0x2000:
                // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "event id = 0x2000\n");
                ModbusSetLocalReg(waitevent.param1, waitevent.param2);
                break;
            default:
                break;
            }
            waitevent.id = 0;
        }
        else
        {
            // QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "no event happend\n");
        }
    }
    goto restart;
err1:
    osiThreadExit();
}
ql_task_t uart1_task = NULL;

void uart1_task_init(void)
{
    QlOSStatus err = 0;
    ;
    err = ql_rtos_task_create(&uart1_task, 4096, APP_PRIORITY_NORMAL, "uart1_task", uart1_task_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        // QL_SDMMC_DEMO_LOG("creat sd task fail err = %d", err);
        QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "creat sd task fail err = %d", err);
    }
    QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, "creat sd task sucess");
}
