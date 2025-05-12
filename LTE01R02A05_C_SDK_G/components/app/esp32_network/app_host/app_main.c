// SPDX-License-Identifier: Apache-2.0
// Copyright 2015-2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at

//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

/** Includes **/
// #include "usart.h"
#include "transport_drv.h"
#include "control.h"
#include "trace.h"
#include "app_main.h"
#include "arp_server_stub.h"
#include "stats.h"
#include "ql_api_osi.h"
#include "ql_gpio.h"
#include "ql_log.h"
#include "../../peripheral/inc/ql_pin_cfg.h"
#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "app_main", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "app_main", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "app_main", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "app_main", msg, ##__VA_ARGS__)
/** Constants/Macros **/
#define ARPING_PATH_TASK_STACK_SIZE 4096

#ifdef __GNUC__
/* With GCC, small LOGI (option LD Linker->Libraries->Small LOGI
 set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

/** Exported variables **/

/** Function declaration **/
static void init_sta(void);
static void init_ap(void);
static void reset_slave(void);
void arping_task(void const *arg);

// /* Needed for timer task */
// void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
// 	StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize );

// /* GetIdleTaskMemory prototype (linked to static allocation support) */
// void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
// 	StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

struct network_handle *sta_handle, *ap_handle;
// static osThreadId arping_task_id = 0;

/** function definition **/

/** Local Functions **/
/**
 * @brief  Reset slave to initialize
 * @param  None
 * @retval None
 */
#define GPIO_PIN_RESET 0
#define GPIO_PIN_SET 1
#define GPIO_RESET_Pin GPIO_4
int reset_slave_flag = 0;

static void reset_slave(void)
{
	uint8_t pin_num;
	uint8_t gpio_func = 0;
	ql_GpioNum gpio_num = 0;
	ql_GpioDir gpio_dir = 0;
	ql_PullMode gpio_pull = 0;
	ql_LvlMode gpio_lvl = 0;

	/* GPIO Ports Clock Enable */
	gpio_func = 0;
	pin_num = 19;
	gpio_num = GPIO_RESET_Pin;
	gpio_dir = QUEC_PIN_NONE;
	gpio_pull = QUEC_PIN_NONE;
	gpio_lvl = QUEC_PIN_NONE;

	ql_pin_set_func(pin_num, gpio_func);
	ql_gpio_init(gpio_num, gpio_dir, gpio_pull, gpio_lvl);
	// while (1)
	// {
	// 	if (reset_slave_flag == 0)
	// 	{
	// 		ql_gpio_set_level(gpio_num, GPIO_PIN_SET);
	// 		ql_rtos_task_sleep_ms(50);
	// 		reset_slave_flag = 1;
	// 	}
	ql_gpio_set_level(gpio_num, GPIO_PIN_RESET);
	// 	ql_rtos_task_sleep_ms(500);
	// }

	// if (reset_slave_flag == 1)
	// {
	// 	while (1)
	// 	{

	// }
	// }
	/* TODO: make this pin configurable from project config */
	// GPIO_InitStruct.Pin = GPIO_RESET_Pin;
	// GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	// GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	// HAL_GPIO_Init(GPIO_RESET_GPIO_Port, &GPIO_InitStruct);

	// HAL_GPIO_WritePin(GPIO_RESET_GPIO_Port, GPIO_RESET_Pin, GPIO_PIN_RESET);

	// /* revert to initial state */
	// GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	// HAL_GPIO_Init(GPIO_RESET_GPIO_Port, &GPIO_InitStruct);
	// gpio_dir = GPIO_INPUT;
	// ql_gpio_init(gpio_num, gpio_dir, gpio_pull, gpio_lvl);
	// ql_gpio_set_level(gpio_num, GPIO_PIN_RESET);

	/* stop spi transactions short time to avoid slave sync issues */
	// hard_delay(50000);
}

/**
 * @brief  Control path event handler callback
 * @param  event - spi_drv_events_e event to be handled
 * @retval None
 */
static void control_path_event_handler(uint8_t event)
{
	switch (event)
	{
	case STATION_CONNECTED:
	{
		init_sta();
		break;
	}
	case STATION_DISCONNECTED:
	{
		LOGI("station disconnected\n\r");
		break;
	}
	case SOFTAP_STARTED:
	{
		init_ap();
		break;
	}
	case SOFTAP_STOPPED:
	{
		LOGI("softap stopped\n\r");
		break;
	}
	default:
		break;
	}
}

/**
 * @brief  transport driver event handler callback
 * @param  event - spi_drv_events_e event to be handled
 * @retval None
 */
static void transport_driver_event_handler(uint8_t event)
{
	
	LOGI("transport event: %d\n\r", event);
	switch (event)
	{
	case TRANSPORT_ACTIVE:
	{
		/* Initiate control path now */
#if DEBUG_TRANSPORT
		LOGI("Base transport is set-up\n\r");
#endif
		control_path_init(control_path_event_handler);
		break;
	}
	default:
		break;
	}
}

/** Exported functions **/

/**
 * @brief  FreeRTOS initialization
 * @param  None
 * @retval None
 */
extern void spi_init(void);
extern int test_set_wifi_mode_station(void);
void MX_FREERTOS_Init(void)
{
	LOGI("every thing init");
	spi_init();

	reset_slave();

	/* Init network interface */
	network_init();

	/* init spi driver */
	transport_init(transport_driver_event_handler);

	test_set_wifi_mode_station();

	#if !TEST_RAW_TP
		/* This thread's priority shouls be >= transport driver's transaction task priority */
		// osThreadDef(Arping_Thread, arping_task, osPriorityAboveNormal, 0,
		// 		ARPING_PATH_TASK_STACK_SIZE);
		// arping_task_id = osThreadCreate(osThread(Arping_Thread), NULL);
		// assert(arping_task_id);
		ql_task_t Arping_Thread;
		int err = ql_rtos_task_create(&Arping_Thread, ARPING_PATH_TASK_STACK_SIZE, APP_PRIORITY_BELOW_NORMAL, "arping_task", arping_task, NULL, 5);
		assert(Arping_Thread);
	#endif
}

/**
 * @brief  Retargets the C library LOGI function to the USART.
 * @param  None
 * @retval None
 */
#include "drv_uart.h"
#include "ql_uart.h"
#define QL_CUR_UART_PORT QL_UART_PORT_2
PUTCHAR_PROTOTYPE
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART6 and
	 * Loop until the end of transmission */
	// HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, 0xFFFF);
	uint8_t ch_buf = 0xFFFF;
	ql_uart_write(QL_CUR_UART_PORT, ch_buf, 1);

	return ch;
}

/**
 * @brief FreeRTOS hook function for idle task stack
 * @param  None
 * @retval None
 */
// void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
// 	StackType_t **ppxIdleTaskStackBuffer,
// 	uint32_t *pulIdleTaskStackSize)
// {
// 	/* If the buffers to be provided to the Idle task are declared
// 	 * inside this function then they must be declared static –
// 	 * otherwise they will be allocated on the stack and so not exists
// 	 * after this function exits.
// 	 * */
// 	static StaticTask_t xIdleTaskTCB;
// 	static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

// 	/* Pass out a pointer to the StaticTask_t structure in which the
// 	 * Idle task’s state will be stored. */
// 	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

// 	/* Pass out the array that will be used as the Idle task’s stack. */
// 	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

// 	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
// 	Note that, as the array is necessarily of type StackType_t,
// 	configMINIMAL_STACK_SIZE is specified in words, not bytes. */
// 	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
// }

/**
 * @brief FreeRTOS hook function for timer task stack
 * @param  None
 * @retval None
 */
// void vApplicationGetTimerTaskMemory( StaticTask_t **ppxTimerTaskTCBBuffer,
// 	StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize )
// {
// 	/* USER CODE BEGIN GET_TIMER_TASK_MEMORY */
// 	static StaticTask_t xTimerTaskTCBBuffer;
// 	static StackType_t xTimerStack[configTIMER_TASK_STACK_DEPTH];

// 	*ppxTimerTaskTCBBuffer = &xTimerTaskTCBBuffer;
// 	*ppxTimerTaskStackBuffer = &xTimerStack[0];
// 	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
//   /* place for user code */
// }

/**
 * @brief Station mode rx callback
 * @param  net_handle - station network handle
 * @retval None
 */
static void sta_rx_callback(struct network_handle *net_handle)
{
	struct pbuf *rx_buffer = NULL;
	struct pbuf *snd_buffer = NULL;
	uint8_t *arp_resp = NULL;
	uint16_t arp_resp_len = 0;
	uint32_t sta_ip = 0;
	int ret;

	rx_buffer = network_read(net_handle, 0);

	if (get_self_ip_station(&sta_ip))
	{
		LOGI("Problem getting self station ip\n\r");
		if (rx_buffer)
		{
			free(rx_buffer->payload);
			rx_buffer->payload = NULL;
			free(rx_buffer);
			rx_buffer = NULL;
		}
		return;
	}

	if (rx_buffer)
	{
		arp_resp = arp_req_handler(&sta_ip, get_self_mac_station(), rx_buffer->payload,
								   rx_buffer->len, &arp_resp_len);

		if (arp_resp)
		{
			snd_buffer = malloc(sizeof(struct pbuf));
			assert(snd_buffer);

			snd_buffer->payload = arp_resp;
			snd_buffer->len = arp_resp_len;

			ret = network_write(net_handle, snd_buffer);

			if (ret)
				LOGI("%s: Failed to send arp response\n\r", __func__);
		}

		free(rx_buffer->payload);
		rx_buffer->payload = NULL;
		free(rx_buffer);
		rx_buffer = NULL;
	}
}

/**
 * @brief Softap mode rx callback
 * @param  net_handle - Softap network handle
 * @retval None
 */
static void ap_rx_callback(struct network_handle *net_handle)
{
	struct pbuf *rx_buffer = NULL;
	struct pbuf *snd_buffer = NULL;
	uint8_t *arp_resp = NULL;
	uint16_t arp_resp_len = 0;
	int ret;
	uint32_t softap_ip = 0;

	rx_buffer = network_read(net_handle, 0);

	if (get_self_ip_softap(&softap_ip))
	{
		LOGI("Problem getting self softap ip\n\r");
		if (rx_buffer)
		{
			free(rx_buffer->payload);
			rx_buffer->payload = NULL;
			free(rx_buffer);
			rx_buffer = NULL;
		}
		return;
	}

	if (rx_buffer)
	{
		arp_resp = arp_req_handler(&softap_ip, get_self_mac_softap(),
								   rx_buffer->payload, rx_buffer->len, &arp_resp_len);

		if (arp_resp)
		{
			snd_buffer = malloc(sizeof(struct pbuf));
			assert(snd_buffer);

			snd_buffer->payload = arp_resp;
			snd_buffer->len = arp_resp_len;

			ret = network_write(net_handle, snd_buffer);

			if (ret)
				LOGI("%s: Failed to send arp response\n\r", __func__);
		}

		free(rx_buffer->payload);
		rx_buffer->payload = NULL;
		free(rx_buffer);
		rx_buffer = NULL;
	}
}

/**
 * @brief start station mode network path
 * @param None
 * @retval None
 */
static void init_sta(void)
{
	sta_handle = network_open(STA_INTERFACE, sta_rx_callback);
	assert(sta_handle);
}

/**
 * @brief start softap mode network path
 * @param None
 * @retval None
 */
static void init_ap(void)
{
	ap_handle = network_open(SOFTAP_INTERFACE, ap_rx_callback);
	assert(ap_handle);
}

/**
 * @brief task initiate arping req periodically
 * @param Not used
 * @retval None
 */
void arping_task(void const *arg)
{
	uint32_t sta_ip, softap_ip;
	uint32_t sta_dest_ip, softap_dest_ip;
	uint8_t dst_mac_bytes[MAC_LEN] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	sta_ip = softap_ip = sta_dest_ip = softap_dest_ip = 0;
	// while(1)
	// {
	// 	LOGI("ARPING TASK\n\r");
	// }
	get_self_ip_station(&sta_ip);
	get_self_ip_softap(&softap_ip);
	get_arp_dst_ip_station(&sta_dest_ip);
	get_arp_dst_ip_softap(&softap_dest_ip);

	while (1)
	{
		if (sta_handle)
			send_arp_req(sta_handle, get_self_mac_station(), &sta_ip, dst_mac_bytes, &sta_dest_ip);

		if (ap_handle)
			send_arp_req(ap_handle, get_self_mac_softap(), &softap_ip, dst_mac_bytes, &softap_dest_ip);
		// LOGI("ARPING TASK\n\r");
		ql_rtos_task_sleep_ms(1000);
	}
}
