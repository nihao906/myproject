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

/** Includes **/
// #include "cmsis_os.h"
#include "string.h"
// #include "spi.h"
// #include "gpio.h"
#include "trace.h"
#include "spi_drv.h"
#include "ql_api_spi.h"
#include "adapter.h"
#include "serial_drv.h"
#include "netdev_if.h"
#include "stats.h"
#include "ql_log.h"
#include "ql_gpio.h"
#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "spi_drv", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "spi_drv", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "spi_drv", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "spi_drv", msg, ##__VA_ARGS__)

/** Constants/Macros **/
#define TO_SLAVE_QUEUE_SIZE 10
#define FROM_SLAVE_QUEUE_SIZE 10

#define TRANSACTION_TASK_STACK_SIZE 4096
#define PROCESS_RX_TASK_STACK_SIZE 4096

#define MAX_PAYLOAD_SIZE (MAX_SPI_BUFFER_SIZE - sizeof(struct esp_payload_header))

/** Enumeration **/
enum hardware_type_e
{
	HARDWARE_TYPE_ESP32,
	HARDWARE_TYPE_OTHER_ESP_CHIPSETS,
	HARDWARE_TYPE_INVALID,
};

static stm_ret_t spi_transaction_v1(uint8_t *txbuff);
static stm_ret_t spi_transaction_v2(uint8_t *txbuff);

/* spi transaction functions for different hardware types */
static stm_ret_t (*spi_trans_func[])(uint8_t *txbuff) = {
	/* spi_transaction_v1
	 *   It is supported for esp32 only.
	 *
	 * spi_transaction_v2
	 *   It is supported for other esp chipsets
	 *   like ESP32-C2, ESP32-C3, ESP32-C6, ESP32-S2 and ESP32-S3 */
	spi_transaction_v1,
	spi_transaction_v2};

static struct esp_private *esp_priv[MAX_NETWORK_INTERFACES];
static uint8_t hardware_type = HARDWARE_TYPE_INVALID;

static struct netdev_ops esp_net_ops = {
	.netdev_open = esp_netdev_open,
	.netdev_close = esp_netdev_close,
	.netdev_xmit = esp_netdev_xmit,
};

/** Exported variables **/

static ql_sem_t osSemaphore = NULL;
static ql_mutex_t mutex_spi_trans;

static ql_task_t process_rx_task_id = 0;
static ql_task_t transaction_task_id = 0;

/* Queue declaration */
static ql_queue_t to_slave_queue = NULL;
static ql_queue_t from_slave_queue = NULL;

/* callback of event handler */
static void (*spi_drv_evt_handler_fp)(uint8_t);

/** function declaration **/
/** Exported functions **/
static void transaction_task(void const *pvParameters);
static void process_rx_task(void const *pvParameters);
static uint8_t *get_tx_buffer(uint8_t *is_valid_tx_buf);
static void deinit_netdev(void);

/** Local Functions **/
/**
 * @brief  get private interface of expected type and number
 * @param  if_type - interface type
 *         if_num - interface number
 * @retval interface handle if found, else NULL
 */
static struct esp_private *get_priv(uint8_t if_type, uint8_t if_num)
{
	int i = 0;

	for (i = 0; i < MAX_NETWORK_INTERFACES; i++)
	{
		if ((esp_priv[i]) &&
			(esp_priv[i]->if_type == if_type) &&
			(esp_priv[i]->if_num == if_num))
			return esp_priv[i];
	}

	return NULL;
}

/**
 * @brief  create virtual network device
 * @param  None
 * @retval None
 */
static int init_netdev(void)
{
	LOGI("init_netdev");
	void *ndev = NULL;
	int i = 0;
	struct esp_private *priv = NULL;
	char *if_name = STA_INTERFACE;
	uint8_t if_type = ESP_STA_IF;

	for (i = 0; i < MAX_NETWORK_INTERFACES; i++)
	{
		/* Alloc and init netdev */
		ndev = netdev_alloc(sizeof(struct esp_private), if_name);
		if (!ndev)
		{
			deinit_netdev();
			return STM_FAIL;
		}

		priv = (struct esp_private *)netdev_get_priv(ndev);
		if (!priv)
		{
			deinit_netdev();
			return STM_FAIL;
		}

		priv->netdev = ndev;
		priv->if_type = if_type;
		priv->if_num = 0;

		if (netdev_register(ndev, &esp_net_ops))
		{
			deinit_netdev();
			return STM_FAIL;
		}

		if_name = SOFTAP_INTERFACE;
		if_type = ESP_AP_IF;

		esp_priv[i] = priv;
	}

	return STM_OK;
}

/**
 * @brief  destroy virtual network device
 * @param  None
 * @retval None
 */
static void deinit_netdev(void)
{
	for (int i = 0; i < MAX_NETWORK_INTERFACES; i++)
	{
		if (esp_priv[i])
		{
			if (esp_priv[i]->netdev)
			{
				netdev_unregister(esp_priv[i]->netdev);
				netdev_free(esp_priv[i]->netdev);
			}
			esp_priv[i] = NULL;
		}
	}
}
/**
 * @brief  check if alternate function of a GPIO set or not
 * @param  GPIOx - GPIO Instance like A,B,..
 * @retval 1 if alternate function set else 0
 */
static int is_gpio_alternate_function_set(ql_GpioNum pin)
{
	uint8_t func = 2;
	int err = ql_pin_get_func(pin, &func);
	if (err == QL_GPIO_SUCCESS)
	{
		return 1;
	}

	// #define GPIO_NUMBER 16U
	// 	uint32_t position;
	// 	uint32_t ioposition = 0x00U;
	// 	uint32_t iocurrent = 0x00U;

	// 	/* Check the parameters */
	// 	assert_param(IS_GPIO_ALL_INSTANCE(GPIOx));
	// 	assert_param(IS_GPIO_PIN(pin));

	// 	/* Configure the port pins */
	// 	for(position = 0U; position < GPIO_NUMBER; position++)
	// 	{
	// 		/* Get the IO position */
	// 		ioposition = 0x01U << position;
	// 		/* Get the current IO position */
	// 		iocurrent = (uint32_t)(pin) & ioposition;

	// 		if(iocurrent == ioposition)
	// 		{
	// 			if (GPIOx->AFR[position >> 3U]) {
	// 				return 1;
	// 			}
	// 		}
	// 	}
	return 0;
}

/**
 * @brief  Set hardware type to ESP32 or ESP32S2 depending upon
 *         Alternate Function (AF) set for NSS pin (Pin11 i.e. A15)
 *         In case of ESP32, NSS is used by SPI driver, using AF
 *         For ESP32S2, NSS is manually used as GPIO to toggle NSS
 *         NSS (as AF) was not working for ESP32S2, this is workaround for that.
 * @param  None
 * @retval None
 */
static void set_hardware_type(void)
{
	// if (is_gpio_alternate_function_set(USR_SPI_CS_Pin))
	// {
	// 	hardware_type = HARDWARE_TYPE_ESP32;
	// 	LOGI("hardware_type =%d",hardware_type);
	// }
	// else
	// {
	hardware_type = HARDWARE_TYPE_OTHER_ESP_CHIPSETS;
	// }
}

/** function definition **/

/** Exported Functions **/
/**
 * @brief  transport initializes
 * @param  transport_evt_handler_fp - event handler
 * @retval None
 */
extern void gpio_exit_task(void);
void transport_init(void (*transport_evt_handler_fp)(uint8_t))
{
	LOGI("transport_init");
	// int ret;
	stm_ret_t retval = STM_OK;
	/* Check if supported board */
	set_hardware_type();
	// LOGI("hardware_type =%d",hardware_type);

	/* register callback */
	spi_drv_evt_handler_fp = transport_evt_handler_fp;
	LOGI("spi_drv_evt_handler_fp =%d", spi_drv_evt_handler_fp);
	// osSemaphoreDef(SEM);

	retval = init_netdev();
	if (retval)
	{
		LOGI("netdev failed to init\n\r");
		assert(retval == STM_OK);
	}

	/* spi handshake semaphore */
	int err1r = ql_rtos_semaphore_create(&osSemaphore, 1);
	if (err1r != QL_OSI_SUCCESS)
	{
		LOGI("osSemaphore create failed");
	}
	// LOGI("RET =%d",ret);
	// LOGI("osSemaphore =%d",osSemaphore);
	// osSemaphore = osSemaphoreCreate(osSemaphore(SEM) , 1);
	assert(osSemaphore);

	int err2r = ql_rtos_mutex_create(&mutex_spi_trans);
	if (err2r != QL_OSI_SUCCESS)
	{
		LOGI("mutex_spi_trans create failed");
	}
	assert(mutex_spi_trans);

	/* Queue - tx */
	int err3r = ql_rtos_queue_create(&to_slave_queue,
									 sizeof(interface_buffer_handle_t), TO_SLAVE_QUEUE_SIZE);
	if (err3r != QL_OSI_SUCCESS)
	{
		LOGI("to_slave_queue create failed");
	}
	assert(to_slave_queue);

	/* Queue - rx */
	// int ret = xQueueCreate(&from_slave_queue,
	// 					   sizeof(interface_buffer_handle_t), FROM_SLAVE_QUEUE_SIZE);
	int err4r = ql_rtos_queue_create(&from_slave_queue,
									 sizeof(interface_buffer_handle_t), TO_SLAVE_QUEUE_SIZE);
	if (err4r != QL_OSI_SUCCESS)
	{
		LOGI("from_slave_queue create failed");
	}
	assert(from_slave_queue);

	ql_task_t transaction_thread;
	/* Task - SPI transaction (full duplex) */
	int err5r = ql_rtos_task_create(&transaction_thread, TRANSACTION_TASK_STACK_SIZE, APP_PRIORITY_ABOVE_NORMAL,
									"transaction_thread", transaction_task, NULL, 5);
	if (err5r != QL_OSI_SUCCESS)
	{
		LOGI("transaction_thread create failed");
	}
	// transaction_task_id = osThreadCreate(osThread(transaction_thread), NULL);
	assert(transaction_thread);

	ql_task_t rx_thread;
	/* Task - RX processing */
	// osThreadDef(rx_thread, process_rx_task,
	// 			APP_PRIORITY_BELOW_NORMAL, 0, PROCESS_RX_TASK_STACK_SIZE);
	// process_rx_task_id = osThreadCreate(osThread(rx_thread), NULL);
	int err6r = ql_rtos_task_create(&rx_thread, PROCESS_RX_TASK_STACK_SIZE, APP_PRIORITY_ABOVE_NORMAL,
									"rx_thread", process_rx_task, NULL, 5);
	if (err6r != QL_OSI_SUCCESS)
	{
		LOGI("rx_thread create failed");
	}
	assert(rx_thread);

	ql_task_t gpio_exit_thread;
	/* Task - GPIO exit */
	int err7r = ql_rtos_task_create(&gpio_exit_thread, 4096, APP_PRIORITY_ABOVE_NORMAL,
									"gpio_exit_thread", gpio_exit_task, NULL, 5);
	if (err7r != QL_OSI_SUCCESS)
	{
		LOGI("gpio_exit_thread create failed");
	}
	assert(gpio_exit_thread);
}

/**
 * @brief EXTI line detection callback, used as SPI handshake GPIO
 * @param GPIO_Pin: Specifies the pins connected EXTI line
 * @retval None
 */
#define GPIO_DATA_READY_Pin GPIO_23
#define GPIO_HANDSHAKE_Pin GPIO_22
int gpio_level_low_flag = 0;
int gpio_level_high_flag = 0;
void gpio_exit_call_back(ql_GpioNum GPIO_Pin)
{
	ql_LvlMode q = 1;
	// if ((GPIO_Pin == GPIO_DATA_READY_Pin) ||
	// 	(GPIO_Pin == GPIO_HANDSHAKE_Pin))
	if (ql_gpio_get_level(GPIO_Pin, &q) == QL_GPIO_SUCCESS)
	{
		gpio_level_high_flag = 1;
	}
	else if (ql_gpio_get_level(GPIO_Pin, &q) == QL_GPIO_EXECUTE_ERR)
	{
		gpio_level_low_flag = 1;
		if (gpio_level_low_flag == 0 && gpio_level_high_flag == 0)
		{
			LOGI("handshake or data ready is ok");
			/* Post semaphore to notify SPI slave is ready for next transaction */
			if (osSemaphore != NULL)
			{
				ql_rtos_semaphore_release(osSemaphore);
			}
			gpio_level_low_flag = 1;
			gpio_level_high_flag = 1;
		}
	}
}
void gpio_exit_task(void)
{
	LOGI("gpio_exit_task");
	// LOGI("handshake or data ready is ok");
	while (1)
	{
		ql_rtos_task_sleep_ms(1);
		gpio_exit_call_back(GPIO_HANDSHAKE_Pin);
		gpio_exit_call_back(GPIO_DATA_READY_Pin);
	}
}

/**
 * @brief  Schedule SPI transaction if -
 *         a. valid TX buffer is ready at SPI host (STM)
 *         b. valid TX buffer is ready at SPI peripheral (ESP)
 *         c. Dummy transaction is expected from SPI peripheral (ESP)
 * @param  argument: Not used
 * @retval None
 */
#define GPIO_PIN_RESET 0
static void check_and_execute_spi_transaction(void)
{
	uint8_t *txbuff = NULL;
	uint8_t is_valid_tx_buf = 0;
	ql_LvlMode level = 1;
	int gpio_handshake;
	int gpio_rx_data_ready;

	/* handshake line SET -> slave ready for next transaction */
	gpio_handshake = ql_gpio_get_level(GPIO_HANDSHAKE_Pin, &level);
	LOGI("gpio_handshake: %d", gpio_handshake);

	/* data ready line SET -> slave wants to send something */
	gpio_rx_data_ready = ql_gpio_get_level(GPIO_DATA_READY_Pin, &level);
	LOGI("gpio_rx_data_ready: %d", gpio_rx_data_ready);

	if (gpio_handshake == QL_GPIO_SUCCESS)
	{

		/* Get next tx buffer to be sent */
		txbuff = get_tx_buffer(&is_valid_tx_buf);

		if ((gpio_rx_data_ready == QL_GPIO_SUCCESS) ||
			(is_valid_tx_buf))
		{

			/* Execute transaction only if EITHER holds true-
			 * a. A valid tx buffer to be transmitted towards slave
			 * b. Slave wants to send something (Rx for host)
			 */
			ql_rtos_semaphore_wait(mutex_spi_trans, QL_WAIT_FOREVER);
			spi_trans_func[hardware_type](txbuff);
			ql_rtos_semaphore_release(mutex_spi_trans);
		}
	}
}

/**
 * @brief  Send to slave via SPI
 * @param  iface_type -type of interface
 *         iface_num - interface number
 *         wbuffer - tx buffer
 *         wlen - size of wbuffer
 * @retval sendbuf - Tx buffer
 */
stm_ret_t send_to_slave(uint8_t iface_type, uint8_t iface_num,
						uint8_t *wbuffer, uint16_t wlen)
{
	interface_buffer_handle_t buf_handle = {0};

	if (!wbuffer || !wlen || (wlen > MAX_PAYLOAD_SIZE))
	{

		LOGI("write fail: buff(%p) 0? OR (0<len(%u)<=max_poss_len(%u))?\n\r",
			 wbuffer, wlen, MAX_PAYLOAD_SIZE);
		if (wbuffer)
		{
			free(wbuffer);
			wbuffer = NULL;
		}
		return STM_FAIL;
	}
	memset(&buf_handle, 0, sizeof(buf_handle));

	buf_handle.if_type = iface_type;
	buf_handle.if_num = iface_num;
	buf_handle.payload_len = wlen;
	buf_handle.payload = wbuffer;
	buf_handle.priv_buffer_handle = wbuffer;
	buf_handle.free_buf_handle = 0;

	if (TRUE != ql_rtos_queue_release(to_slave_queue, sizeof(buf_handle), &buf_handle, QL_WAIT_FOREVER))
	{
		LOGI("Failed to send buffer to_slave_queue\n\r");
		if (wbuffer)
		{
			free(wbuffer);
			wbuffer = NULL;
		}
		return STM_FAIL;
	}

	check_and_execute_spi_transaction();

	return STM_OK;
}

/** Local functions **/

/**
 * @brief  Give breathing time for slave on spi
 * @param  x - for loop delay count
 * @retval None
 */
static void stop_spi_transactions_for_msec(int x)
{
	hard_delay(x);
}
#define spi_port 1
/**
 * @brief  Full duplex transaction SPI transaction for ESP32 hardware
 * @param  txbuff: TX SPI buffer
 * @retval STM_OK / STM_FAIL
 */
static stm_ret_t spi_transaction_v1(uint8_t *txbuff)
{
	uint8_t *rxbuff = NULL;
	interface_buffer_handle_t buf_handle = {0};
	struct esp_payload_header *payload_header;
	uint16_t len, offset;
	// HAL_StatusTypeDef retval = HAL_ERROR;
	uint16_t rx_checksum = 0, checksum = 0;

	/* Allocate rx buffer */
	rxbuff = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
	assert(rxbuff);
	memset(rxbuff, 0, MAX_SPI_BUFFER_SIZE);

	if (!txbuff)
	{
		/* Even though, there is nothing to send,
		 * valid resetted txbuff is needed for SPI driver
		 */
		txbuff = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
		assert(txbuff);
		memset(txbuff, 0, MAX_SPI_BUFFER_SIZE);
	}

	/* SPI transaction */
	int retval = ql_uart_read(spi_port, (uint8_t *)txbuff,
							  (uint8_t *)rxbuff, MAX_SPI_BUFFER_SIZE, QL_WAIT_FOREVER);

	switch (retval)
	{
	case 0:

		/* Transaction successful */

		/* create buffer rx handle, used for processing */
		payload_header = (struct esp_payload_header *)rxbuff;

		/* Fetch length and offset from payload header */
		len = le16toh(payload_header->len);
		offset = le16toh(payload_header->offset);

		if ((!len) ||
			(len > MAX_PAYLOAD_SIZE) ||
			(offset != sizeof(struct esp_payload_header)))
		{

			/* Free up buffer, as one of following -
			 * 1. no payload to process
			 * 2. input packet size > driver capacity
			 * 3. payload header size mismatch,
			 * wrong header/bit packing?
			 * */
			if (rxbuff)
			{
				free(rxbuff);
				rxbuff = NULL;
			}
			/* Give chance to other tasks */
			ql_rtos_task_sleep_ms(0);
		}
		else
		{
			rx_checksum = le16toh(payload_header->checksum);
			payload_header->checksum = 0;
			checksum = compute_checksum(rxbuff, len + offset);
			if (checksum == rx_checksum)
			{
				buf_handle.priv_buffer_handle = rxbuff;
				buf_handle.free_buf_handle = 0;
				buf_handle.payload_len = len;
				buf_handle.if_type = payload_header->if_type;
				buf_handle.if_num = payload_header->if_num;
				buf_handle.payload = rxbuff + offset;
				buf_handle.seq_num = le16toh(payload_header->seq_num);
				buf_handle.flag = payload_header->flags;

				if (TRUE != ql_rtos_queue_release(from_slave_queue, sizeof(buf_handle),
												  &buf_handle, QL_WAIT_FOREVER))
				{
					LOGI("Failed to send buffer\n\r");
					goto done;
				}
			}
			else
			{
				if (rxbuff)
				{
					free(rxbuff);
					rxbuff = NULL;
				}
			}
		}

		/* Free input TX buffer */
		if (txbuff)
		{
			free(txbuff);
			txbuff = NULL;
		}
		break;

	case 1:
		LOGI("timeout in SPI transaction\n\r");
		goto done;
		break;

	case 2:
		LOGI("Error in SPI transaction\n\r");
		goto done;
		break;
	default:
		LOGI("default handler: Error in SPI transaction\n\r");
		goto done;
		break;
	}

	return STM_OK;

done:
	/* error cases, abort */
	if (txbuff)
	{
		free(txbuff);
		txbuff = NULL;
	}

	if (rxbuff)
	{
		free(rxbuff);
		rxbuff = NULL;
	}
	return STM_FAIL;
}

/**
 * @brief  Full duplex transaction SPI transaction for ESP32S2 hardware
 * @param  txbuff: TX SPI buffer
 * @retval STM_OK / STM_FAIL
 */
#define QL_SPI_DEMO_WAIT_NONE 0
#define QL_SPI_DEMO_WAIT_WRITE 1
#define QL_SPI_DEMO_WAIT_READ 2
static int spi_wait_write_read = 0;
ql_sem_t spi_demo_write;
ql_sem_t spi_demo_read;
unsigned char spi_demo_wait_write_read = QL_SPI_DEMO_WAIT_NONE;
void ql_spi_demo_cb_handler(ql_spi_irq_s cause)
{
	if (cause.tx_dma_done == 1 && spi_demo_wait_write_read == QL_SPI_DEMO_WAIT_WRITE)
	{
		spi_demo_wait_write_read = QL_SPI_DEMO_WAIT_NONE;
		ql_rtos_semaphore_release(spi_demo_write);
	}

	if (cause.rx_dma_done == 1 && spi_demo_wait_write_read == QL_SPI_DEMO_WAIT_READ)
	{
		spi_demo_wait_write_read = QL_SPI_DEMO_WAIT_NONE;
		ql_rtos_semaphore_release(spi_demo_read);
	}

	LOGI("cause.tx_dma_done=%d", cause.tx_dma_done);
	LOGI("cause.rx_dma_done=%d", cause.rx_dma_done);
}
static int spi_write_read(int fd, uint8_t *outbuf, uint8_t *inbuf, int len)
{
	int ret;
	spi_wait_write_read = QL_SPI_DEMO_WAIT_READ;
	ret = ql_spi_write_read(fd, inbuf, outbuf, len);
	ql_rtos_semaphore_wait(spi_demo_read, 10);
	LOGI("spi_write_read inbuf=%p,outbuf=%p,len=%d,ret=%d", inbuf, outbuf, len, ret);
	return ret;
}
static int spi_write(int fd, uint8_t *inbuf, int len)
{
	int ret;
	spi_wait_write_read = QL_SPI_DEMO_WAIT_WRITE;
	ql_spi_request_sys_clk(fd);
	ret = ql_spi_write(fd, inbuf, len);
	ql_rtos_semaphore_wait(spi_demo_write, 10);
	LOGI("spi_write inbuf=%p,len=%d,ret=%d", inbuf, len, ret);
	return ret;
}
void spi_init(void)
{

	ql_spi_config_s spi_config = {0};
	int spi_no = QL_SPI_PORT2;
	spi_config.port = spi_no;
	spi_config.spiclk = QL_SPI_CLK_1_5625MHZ;
	spi_config.framesize = 8;
	spi_config.input_mode = QL_SPI_INPUT_TRUE;
	spi_config.cs_polarity0 = QL_SPI_CS_ACTIVE_LOW;
	spi_config.cs_polarity1 = QL_SPI_CS_ACTIVE_LOW;
	spi_config.cpol = QL_SPI_CPOL_LOW;
	spi_config.cpha = QL_SPI_CPHA_2Edge; // mode 1
	spi_config.input_sel = QL_SPI_DI_1;
	spi_config.transmode = QL_SPI_DMA_IRQ;
	spi_config.cs = QL_SPI_CS0;
	spi_config.clk_delay = QL_SPI_CLK_DELAY_0;
	LOGI("spi_task entry");
	ql_rtos_task_sleep_ms(5000);
	ql_spi_init_ext(spi_config);
	// ql_spi_cs_auto(spi_no);

	ql_spi_irq_s mask = {0};
	mask.rx_dma_done = 1;
	mask.tx_dma_done = 1;
	ql_rtos_semaphore_create(&spi_demo_write, 0);
	ql_rtos_semaphore_create(&spi_demo_read, 0);
	ql_spi_set_irq(spi_no, mask, ql_spi_demo_cb_handler);
}
static uint8_t spi_txbuf[512] OSI_CACHE_LINE_ALIGNED;
static uint8_t spi_rxbuf[512] OSI_CACHE_LINE_ALIGNED;
static stm_ret_t spi_transaction_v2(uint8_t *txbuff)
{
	uint8_t *rxbuff = NULL;
	interface_buffer_handle_t buf_handle = {0};
	struct esp_payload_header *payload_header;
	uint16_t len, offset;
	// HAL_StatusTypeDef retval = HAL_ERROR;
	uint16_t rx_checksum = 0, checksum = 0;

	/* Allocate rx buffer */
	rxbuff = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
	assert(rxbuff);
	memset(rxbuff, 0, MAX_SPI_BUFFER_SIZE);

	if (!txbuff)
	{
			/* Even though, there is nothing to send,
			 * valid reseted txbuff is needed for SPI driver
			 */
			txbuff = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
			assert(txbuff);
			memset(txbuff, 0, MAX_SPI_BUFFER_SIZE);
		// memset(spi_txbuf, 0, 512);
	}

	/* SPI transaction */
	// HAL_GPIO_WritePin(USR_SPI_CS_GPIO_Port, USR_SPI_CS_Pin, 0);
	// retval = HAL_SPI_TransmitReceive(&hspi1, (uint8_t *)txbuff,
	// 								 (uint8_t *)rxbuff, MAX_SPI_BUFFER_SIZE, HAL_MAX_DELAY);
	// while (hspi1.State == HAL_SPI_STATE_BUSY)
	// 	;
	// HAL_GPIO_WritePin(USR_SPI_CS_GPIO_Port, USR_SPI_CS_Pin, 1);
	ql_spi_cs_low(QL_SPI_PORT2);
	// memcpy(spi_rxbuf, rxbuff, 512);
	for (int i; i < 3; i++)
	{
		// if (!txbuff)
		// {
			memcpy(spi_txbuf, txbuff + i * 512, 512);
		// }

		LOGI("txbuff=%d", txbuff);
		int retval = spi_write_read(QL_SPI_PORT2, spi_txbuf, spi_rxbuf, 512);
		// spi_write(QL_SPI_PORT2, txbuff, MAX_SPI_BUFFER_SIZE);
		LOGI("retval=%d", retval);
		switch (retval)
		{
		case QL_SPI_SUCCESS:

			/* Transaction successful */

			memcpy(rxbuff + i * 512, spi_rxbuf, 512);
			if (i < 2)
			{
				continue;
			}
			ql_spi_cs_high(QL_SPI_PORT2);
			/* create buffer rx handle, used for processing */
			payload_header = (struct esp_payload_header *)rxbuff;

			/* Fetch length and offset from payload header */
			len = le16toh(payload_header->len);
			offset = le16toh(payload_header->offset);

			if ((!len) ||
				(len > MAX_PAYLOAD_SIZE) ||
				(offset != sizeof(struct esp_payload_header)))
			{

				/* Free up buffer, as one of following -
				 * 1. no payload to process
				 * 2. input packet size > driver capacity
				 * 3. payload header size mismatch,
				 * wrong header/bit packing?
				 * */
				if (rxbuff)
				{
					free(rxbuff);
					rxbuff = NULL;
				}

				/* Give chance to other tasks */
				ql_rtos_task_sleep_ms(0);
			}
			else
			{
				rx_checksum = le16toh(payload_header->checksum);
				payload_header->checksum = 0;

				checksum = compute_checksum(rxbuff, len + offset);

				if (checksum == rx_checksum)
				{
					buf_handle.priv_buffer_handle = rxbuff;
					buf_handle.free_buf_handle = 0;
					buf_handle.payload_len = len;
					buf_handle.if_type = payload_header->if_type;
					buf_handle.if_num = payload_header->if_num;
					buf_handle.payload = rxbuff + offset;
					buf_handle.seq_num = le16toh(payload_header->seq_num);
					buf_handle.flag = payload_header->flags;

					if (TRUE != ql_rtos_queue_release(from_slave_queue, sizeof(buf_handle),
													  &buf_handle, QL_WAIT_FOREVER))
					{
						LOGI("Failed to send buffer\n\r");
						goto done;
					}
				}
				else
				{
					if (rxbuff)
					{
						free(rxbuff);
						rxbuff = NULL;
					}
				}
			}

			/* Free input TX buffer */
			if (txbuff)
			{
				free(txbuff);
				txbuff = NULL;
			}
			break;

			// case 1:
			// 	LOGI("timeout in SPI transaction\n\r");
			// 	goto done;
			// 	break;

		case QL_SPI_ERROR:
			LOGI("Error in SPI transaction\n\r");
			goto done;
			break;
		default:
			LOGI("default handler: Error in SPI transaction\n\r");
			goto done;
			break;
		}
	}

	return STM_OK;

done:
	/* error cases, abort */
	if (txbuff)
	{
		free(txbuff);
		txbuff = NULL;
	}

	if (rxbuff)
	{
		free(rxbuff);
		rxbuff = NULL;
	}
	ql_spi_cs_high(QL_SPI_PORT2);
	return STM_FAIL;
}

/**
 * @brief  Task for SPI transaction
 * @param  argument: Not used
 * @retval None
 */
static void transaction_task(void const *pvParameters)
{
	if (hardware_type == HARDWARE_TYPE_ESP32)
	{
		LOGI("ESP-Hosted for ESP32");
	}
	else if (hardware_type == HARDWARE_TYPE_OTHER_ESP_CHIPSETS)
	{
		LOGI("ESP-Hosted for ESP32-C2/C3/C6/S2/S3");
	}
	else
	{
		LOGI("Unsupported slave hardware\n\r");
		assert(hardware_type != HARDWARE_TYPE_INVALID);
	}
	while (1)
	{
		if (osSemaphore != NULL)
		{
			/* Wait till slave is ready for next transaction */
			if ((ql_rtos_semaphore_wait(osSemaphore, QL_WAIT_FOREVER)) == 0)
			{
				check_and_execute_spi_transaction();
				LOGI("osSemaphore WAIT ");
			}
		}
		ql_rtos_task_sleep_ms(1);
	}
}

/**
 * @brief  RX processing task
 * @param  argument: Not used
 * @retval None
 */
static void process_rx_task(void const *pvParameters)
{
	LOGI("process_rx_task ENTER");
	stm_ret_t ret = STM_OK;
	interface_buffer_handle_t buf_handle = {0};
	uint8_t *payload = NULL;
	struct pbuf *buffer = NULL;
	struct esp_priv_event *event = NULL;
	struct esp_private *priv = NULL;
	while (1)
	{
		// LOGI(" process_rx_task TEXT 1");
		ql_rtos_task_sleep_ms(1);
		ret = ql_rtos_queue_wait(from_slave_queue, &buf_handle, sizeof(interface_buffer_handle_t), QL_WAIT_FOREVER);

		if (ret != true)
		{
			continue;
		}

		/* point to payload */
		payload = buf_handle.payload;

		LOGI("buf_handle.if_type = %d", buf_handle.if_type);
		/* process received buffer for all possible interface types */
		if (buf_handle.if_type == ESP_SERIAL_IF)
		{
			LOGI("buf_handle.if_type == esp_serial_if");
			/* serial interface path */
			serial_rx_handler(&buf_handle);
		}
		else if ((buf_handle.if_type == ESP_STA_IF) ||
				 (buf_handle.if_type == ESP_AP_IF))
		{
			LOGI("buf_handle.if_type == esp_sta_if or esp_ap_if");
			priv = get_priv(buf_handle.if_type, buf_handle.if_num);

			if (priv)
			{
				buffer = (struct pbuf *)malloc(sizeof(struct pbuf));
				assert(buffer);

				buffer->len = buf_handle.payload_len;
				buffer->payload = malloc(buf_handle.payload_len);
				assert(buffer->payload);

				memcpy(buffer->payload, buf_handle.payload,
					   buf_handle.payload_len);

				netdev_rx(priv->netdev, buffer);
			}
		}
		else if (buf_handle.if_type == ESP_PRIV_IF)
		{
			LOGI("buf_handle.if_type == esp_priv_if");
			buffer = (struct pbuf *)malloc(sizeof(struct pbuf));
			assert(buffer);

			buffer->len = buf_handle.payload_len;
			buffer->payload = malloc(buf_handle.payload_len);
			assert(buffer->payload);

			memcpy(buffer->payload, buf_handle.payload,
				   buf_handle.payload_len);

			process_priv_communication(buffer);
			/* priv transaction received */
			LOGI("Received INIT event\n\r");

			event = (struct esp_priv_event *)(payload);
			if (event->event_type == ESP_PRIV_EVENT_INIT)
			{
				/* halt spi transactions for some time,
				 * this is one time delay, to give breathing
				 * time to slave before spi trans start */
				stop_spi_transactions_for_msec(50000);
				LOGI("event->event_type == ESP_PRIV_EVENT_INIT");
				if (spi_drv_evt_handler_fp)
				{
					spi_drv_evt_handler_fp(TRANSPORT_ACTIVE);
				}
			}
			else
			{
				/* User can re-use this type of transaction */
			}
		}
		else if (buf_handle.if_type == ESP_TEST_IF)
		{
			LOGI("buf_handle.if_type == esp_test_if");
#if TEST_RAW_TP
			update_test_raw_tp_rx_len(buf_handle.payload_len);
#endif
		}
		else
		{
			LOGI("unknown type %d \n\r", buf_handle.if_type);
		}

		/* Free buffer handle */
		/* When buffer offloaded to other module, that module is
		 * responsible for freeing buffer. In case not offloaded or
		 * failed to offload, buffer should be freed here.
		 */
		if (buf_handle.free_buf_handle)
		{
			free(buf_handle.priv_buffer_handle);
		}
	}
}

/**
 * @brief  Next TX buffer in SPI transaction
 * @param  argument: Not used
 * @retval sendbuf - Tx buffer
 */
static uint8_t *get_tx_buffer(uint8_t *is_valid_tx_buf)
{
	struct esp_payload_header *payload_header;
	uint8_t *sendbuf = NULL;
	uint8_t *payload = NULL;
	uint16_t len = 0;
	interface_buffer_handle_t buf_handle = {0};

	*is_valid_tx_buf = 0;

	/* Check if higher layers have anything to transmit, non blocking.
	 * If nothing is expected to send, queue receive will fail.
	 * In that case only payload header with zero payload
	 * length would be transmitted.
	 */
	if (TRUE == ql_rtos_queue_wait(to_slave_queue, &buf_handle, sizeof(interface_buffer_handle_t), 0))
	{
		len = buf_handle.payload_len;
	}

	if (len)
	{

		sendbuf = (uint8_t *)malloc(MAX_SPI_BUFFER_SIZE);
		if (!sendbuf)
		{
			LOGI("malloc failed\n\r");
			goto done;
		}

		memset(sendbuf, 0, MAX_SPI_BUFFER_SIZE);

		*is_valid_tx_buf = 1;

		/* Form Tx header */
		payload_header = (struct esp_payload_header *)sendbuf;
		payload = sendbuf + sizeof(struct esp_payload_header);
		payload_header->len = htole16(len);
		payload_header->offset = htole16(sizeof(struct esp_payload_header));
		payload_header->if_type = buf_handle.if_type;
		payload_header->if_num = buf_handle.if_num;
		memcpy(payload, buf_handle.payload, min(len, MAX_PAYLOAD_SIZE));
		payload_header->checksum = htole16(compute_checksum(sendbuf,
															sizeof(struct esp_payload_header) + len));
		;
	}

done:
	/* free allocated buffer */
	if (buf_handle.free_buf_handle)
		free(buf_handle.priv_buffer_handle);

	return sendbuf;
}
