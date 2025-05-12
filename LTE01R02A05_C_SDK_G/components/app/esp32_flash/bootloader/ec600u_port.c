#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"

#include "ql_gpio.h"
#include "ql_uart.h"

#include "osi_api.h"
#include "drv_uart.h"
#include "ec600u_port.h"

#include "ql_log.h"
#define SERIAL_FLASHER_RESET_HOLD_TIME_MS 100
#define SERIAL_FLASHER_BOOT_HOLD_TIME_MS 50

#define UART_PORT QL_UART_PORT_3
#define UART_BAUDRATE QL_UART_BAUD_115200
#define UART_RX_BUFF_SIZE 2048
#define UART_TX_BUFF_SIZE 2048


#define ESP32_RESET_PIN 15
#define ESP32_RESET_GPIO 22

#define ESP32_BOOT_PIN 14
#define ESP32_BOOT_GPIO 21

#define LOG_TAG "esp32_flash"

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, msg, ##__VA_ARGS__)

#define MIN(a, b) (a < b ? a : b)

static int64_t s_time_end;

static drvUart_t *uart = NULL;
static osiSemaphore_t *sem_read;
static osiSemaphore_t *sem_write;

// static char hex[1024];
// static char *bin2hex(uint8_t *bin, int lenght)
// {

//     if (lenght > sizeof(hex) / 3)
//     {
//         lenght = sizeof(hex) / 3;
//     }
//     int i;
//     for (i = 0; i < lenght; i++)
//     {
//         sprintf(hex + i * 3, "%02X ", bin[i]);
//     }
//     hex[i * 3] = '\0';
//     return hex;
// }

static void esp32_uart_notify_cb(unsigned int ind_type, ql_uart_port_number_e port, unsigned int size)
{
    switch (ind_type)
    {
        case QUEC_UART_RX_OVERFLOW_IND: // rx buffer overflow
        case QUEC_UART_RX_RECV_DATA_IND:
            LOGI("esp32_uart_notify_cb revived data");
            ql_rtos_semaphore_release(sem_read);
            break;
        case QUEC_UART_TX_FIFO_COMPLETE_IND:
            /* 清空输入缓冲区！ */

            LOGI("esp32_uart_notify_cb tx fifo complete");
            ql_rtos_semaphore_release(sem_write);
            break;
    }
}

/* 串口阻塞读写操作，timeout单位ms */
static int uart_read_block(unsigned char *data, unsigned int data_len, unsigned int timeout)
{
    uint32_t read_len = 0;
    uint32_t len = 0;

    read_len = ql_uart_read(UART_PORT, data, data_len);
    len += read_len;
    while (len < data_len)
    {
        if (QL_OSI_SUCCESS != ql_rtos_semaphore_wait(sem_read, timeout))
            break;
        read_len = ql_uart_read(UART_PORT, data + len, data_len - len);
        len += read_len;
    }

    return len;
}

static int uart_write_block(unsigned char *data, unsigned int data_len, unsigned int timeout)
{
    unsigned int write_len = 0;
    write_len = ql_uart_write(UART_PORT, data, data_len);
    if (QL_OSI_SUCCESS != ql_rtos_semaphore_wait(sem_write, timeout))
        return -1;
    return write_len;
}

#include "esp32_flash.h"
int loader_port_ec600u_init(void)
{
    int ret = 0;
    esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_FLASH_INIT, ESP32_FLASH_STATUS_DOING, 0);

    /* 串口初始化 */
    ql_uart_config_s uart_cfg = {
        .baudrate = UART_BAUDRATE,
        .flow_ctrl = QL_FC_NONE,
        .data_bit = QL_UART_DATABIT_8,
        .stop_bit = QL_UART_STOP_1,
        .parity_bit = QL_UART_PARITY_NONE,
    };
    ret = ql_uart_set_dcbconfig(UART_PORT, &uart_cfg);
    if (QL_UART_SUCCESS != ret)
    {
        LOGI("uart dcbconfig err, ret: 0x%x", ret);
        ret = -1;
        goto exit;
    }
    ret = ql_uart_open(UART_PORT);
    if (QL_UART_SUCCESS != ret)
    {
        LOGI("uart open err, ret: 0x%x", ret);
        ret = -2;
        goto exit;
    }
    ql_rtos_semaphore_create(&sem_read, 0);
    ql_rtos_semaphore_create(&sem_write, 0);

    ret = ql_uart_register_cb(UART_PORT, esp32_uart_notify_cb);
    if (QL_UART_SUCCESS != ret)
    {
        LOGI("uart register callback err, ret: 0x%x", ret);
        ql_uart_close(UART_PORT);
        ret = -3;
        goto exit;
    }

    /* 初始化reset,gpio0引脚 */
    // ql_pin_set_func(ESP32_RESET_PIN, RESET_FUN);
    // ql_gpio_deinit(ESP32_RESET_GPIO);
    ql_gpio_init(ESP32_RESET_GPIO, GPIO_OUTPUT, PULL_DOWN, LVL_LOW);

    // ql_pin_set_func(ESP32_BOOT_PIN, GPIO0_FUN);
    // ql_gpio_deinit(ESP32_BOOT_GPIO);
    ql_gpio_init(ESP32_BOOT_GPIO, GPIO_OUTPUT, PULL_DOWN, LVL_LOW);

exit:
    if (ret == 0)
    {
        esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_FLASH_INIT, ESP32_FLASH_STATUS_FINISH, 0);
    }
    else 
    {
        esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_FLASH_INIT, ESP32_FLASH_STATUS_ERR, (uint16_t)ret);
    }
    return ret;
}

void loader_port_ec600u_init_gpio(void)
{
    ql_gpio_init(ESP32_RESET_GPIO, GPIO_OUTPUT, PULL_DOWN, LVL_LOW);
    ql_gpio_init(ESP32_BOOT_GPIO, GPIO_OUTPUT, PULL_DOWN, LVL_LOW);
}
void loader_port_ec600u_deinit_gpio(void)
{
    ql_gpio_deinit(ESP32_RESET_GPIO);
    ql_gpio_deinit(ESP32_BOOT_GPIO);
}

void loader_port_ec600u_deinit(void)
{
    ql_uart_close(UART_PORT);
    ql_gpio_deinit(ESP32_RESET_GPIO);
    ql_gpio_deinit(ESP32_BOOT_GPIO);
}

esp_loader_error_t loader_port_write(const uint8_t *data, uint16_t size, uint32_t timeout)
{
    // LOGD("uart_write %d bytes data=[%s]", size, bin2hex(data, size));
    unsigned int write_len = 0;
    write_len = ql_uart_write(UART_PORT, data, size);
    if (QL_OSI_SUCCESS != ql_rtos_semaphore_wait(sem_write, timeout))
    {
        LOGE("loader_port_write timeout");
        return ESP_LOADER_ERROR_TIMEOUT;
    }
    return ESP_LOADER_SUCCESS;
}

esp_loader_error_t loader_port_read(uint8_t *data, uint16_t size, uint32_t timeout)
{
    uint32_t read_len = 0;
    uint32_t len = 0;

    if (timeout == 0)
    {
        return ESP_LOADER_ERROR_TIMEOUT;
    }

    read_len = ql_uart_read(UART_PORT, data, size);
    if (read_len > 0)
    {
        // timeout = 10;
        len = read_len;
    }
    while (len < size)
    {
        if (QL_OSI_SUCCESS != ql_rtos_semaphore_wait(sem_read, loader_port_remaining_time()))
        {
            break;
        }
        timeout = 10;
        read_len = ql_uart_read(UART_PORT, data + len, size - len);
        len += read_len;
    }
    if (len > 0)
    {
        // LOGI(LOG_TAG "uart_read %d bytes data=[%s]", len, bin2hex(data, len));
    }
    if (len == size) return ESP_LOADER_SUCCESS;
    return ESP_LOADER_ERROR_TIMEOUT;
}

// Set GPIO0 LOW, then
// assert reset pin for 50 milliseconds.
void loader_port_enter_bootloader(void)
{
    ql_gpio_set_level(ESP32_BOOT_GPIO, LVL_HIGH);
    loader_port_reset_target();
    ql_rtos_task_sleep_ms(SERIAL_FLASHER_BOOT_HOLD_TIME_MS);
    ql_gpio_set_level(ESP32_BOOT_GPIO, LVL_LOW);
}

void loader_port_reset_target(void)
{
    esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_RESET_TARGET, ESP32_FLASH_STATUS_DOING, 0);
    ql_gpio_set_level(ESP32_RESET_GPIO, LVL_HIGH);
    ql_rtos_task_sleep_ms(SERIAL_FLASHER_RESET_HOLD_TIME_MS);
    ql_gpio_set_level(ESP32_RESET_GPIO, LVL_LOW);
    esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_RESET_TARGET, ESP32_FLASH_STATUS_FINISH, 0);
}

void loader_port_delay_ms(uint32_t ms)
{
    ql_rtos_task_sleep_ms(ms);
}

uint32_t get_time_us(void)
{
    ql_timeval_t timeval;
    if (QL_OSI_SUCCESS != ql_gettimeofday(&timeval))
        return -1;
    return timeval.usec;
}

void loader_port_start_timer(uint32_t ms)
{
    s_time_end = get_time_us() + ms * 1000;
}

uint32_t loader_port_remaining_time(void)
{
    int64_t remaining = (s_time_end - get_time_us()) / 1000;
    return (remaining > 0) ? (uint32_t)remaining : 0;
}

void loader_port_debug_print(const char *str)
{
    LOGI("DEBUG: %s", str);
}

esp_loader_error_t loader_port_change_transmission_rate(uint32_t baudrate)
{
    ql_uart_config_s uart_cfg = {
        .baudrate = baudrate,
        .flow_ctrl = QL_FC_NONE,
        .data_bit = QL_UART_DATABIT_8,
        .stop_bit = QL_UART_STOP_1,
        .parity_bit = QL_UART_PARITY_NONE,
    };
    int ret;
    ret = ql_uart_close(UART_PORT);
    ret = ql_uart_set_dcbconfig(UART_PORT, &uart_cfg);
    if (ret < 0)
    {
        LOGE("ql_uart_get_dcbconfig err, ret: 0x%x", ret);
        return ESP_LOADER_ERROR_FAIL;
    }
    ret = ql_uart_open(UART_PORT);
    if (ret < 0)
    {
        LOGE("change_transmission_rate  open err, ret: 0x%x", ret);
        return ESP_LOADER_ERROR_FAIL;
    }
    ql_uart_register_cb(UART_PORT, esp32_uart_notify_cb);
    return ESP_LOADER_SUCCESS;
}
