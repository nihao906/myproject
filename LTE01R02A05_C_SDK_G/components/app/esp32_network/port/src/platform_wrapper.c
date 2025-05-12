// SPDX-License-Identifier: Apache-2.0
// Copyright 2015-2021 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "string.h"
#include "trace.h"
#include "serial_if.h"
#include "serial_ll_if.h"
#include "platform_wrapper.h"
#include "ql_api_osi.h"


#include "ql_log.h"
#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, "platform_wrapper", msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, "platform_wrapper", msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, "platform_wrapper", msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, "platform_wrapper", msg, ##__VA_ARGS__)

#define MILLISEC_TO_SEC 1000
#define TICKS_PER_SEC (1000 / portTICK_PERIOD_MS);
#define SEC_TO_MILLISEC(x) (1000 * (x))

#define HOSTED_CALLOC(buff, nbytes)                               \
	do                                                            \
	{                                                             \
		buff = (uint8_t *)hosted_calloc(1, nbytes);               \
		if (!buff)                                                \
		{                                                         \
			LOGI("%s, Failed to allocate memory \n", __func__); \
			goto free_bufs;                                       \
		}                                                         \
	} while (0);

static ql_sem_t readSemaphore;
static serial_ll_handle_t *serial_ll_if_g;

static void control_path_rx_indication(void);

struct serial_drv_handle_t
{
	int handle; /* dummy variable */
};

// struct timer_handle_t
// {
// 	osTimerId timer_id;
// };

int control_path_platform_init(void)
{
	// osSemaphoreDef(READSEM);

	/* control path semaphore */
	// osSemaphoreCreate(osSemaphore(READSEM), 1);
	ql_rtos_semaphore_create(&readSemaphore, 1);
	assert(readSemaphore);

	/* grab the semaphore, so that task will be mandated to wait on semaphore */
	if (ql_rtos_semaphore_wait(readSemaphore, QL_WAIT_FOREVER) != 1)
	{
		LOGI("could not obtain readSemaphore\n\r");
		return STM_FAIL;
	}

	serial_ll_if_g = serial_ll_init(control_path_rx_indication);
	if (!serial_ll_if_g)
	{
		LOGI("Serial interface creation failed\n\r");
		assert(serial_ll_if_g);
		return STM_FAIL;
	}
	if (STM_OK != serial_ll_if_g->fops->open(serial_ll_if_g))
	{
		LOGI("Serial interface open failed\n\r");
		return STM_FAIL;
	}
	return STM_OK;
}

int control_path_platform_deinit(void)
{
	if (STM_OK != serial_ll_if_g->fops->close(serial_ll_if_g))
	{
		LOGI("Serial interface close failed\n\r");
		return STM_FAIL;
	}
	return STM_OK;
}

static void control_path_rx_indication(void)
{
	/* heads up to control path for read */
	if (readSemaphore)
	{
		ql_rtos_semaphore_release(readSemaphore);
	}
}

/* -------- Memory ---------- */
void *hosted_malloc(size_t size)
{
	return malloc(size);
}

void *hosted_calloc(size_t blk_no, size_t size)
{
	void *ptr = malloc(blk_no * size);
	if (!ptr)
	{
		return NULL;
	}

	memset(ptr, 0, blk_no * size);
	return ptr;
}

void hosted_free(void *ptr)
{
	if (ptr)
	{
		free(ptr);
		ptr = NULL;
	}
}

void *hosted_realloc(void *mem, size_t newsize)
{
	void *p = NULL;

	if (newsize == 0)
	{
		free(mem);
		return NULL;
	}

	p = hosted_malloc(newsize);
	if (p)
	{
		/* zero the memory */
		if (mem != NULL)
		{
			memcpy(p, mem, newsize);
			free(mem);
		}
	}
	return p;
}

/* -------- Threads ---------- */
void *hosted_thread_create(void (*start_routine)(void const *), void *arg)
{
	if (!start_routine)
	{
		LOGI("start_routine is mandatory for thread create\n");
		return NULL;
	}

	ql_task_t *thread_handle = (ql_task_t *)hosted_malloc(
		sizeof(ql_task_t));
	if (!thread_handle)
	{
		LOGI("Failed to allocate thread handle\n");
		return NULL;
	}

	// osThreadDef(
	// 	thread_handle,
	// 	start_routine,
	// 	APP_PRIORITY_NORMAL, 0,
	// 	CTRL_PATH_TASK_STACK_SIZE);
	// *thread_handle = osThreadCreate(osThread(thread_handle), arg);
	ql_rtos_task_create(&thread_handle, CTRL_PATH_TASK_STACK_SIZE, APP_PRIORITY_BELOW_NORMAL,"thread_handle" ,start_routine, NULL, 0);

	if (!(*thread_handle))
	{
		LOGI("Failed to create ctrl path task\n");
		free(thread_handle);
		return NULL;
	}
	return thread_handle;
}

int hosted_thread_cancel(void *thread_handle)
{
	int ret = STM_OK;
	ql_task_t *thread_hdl = NULL;

	if (!thread_handle)
	{
		LOGI("Invalid thread handle\n");
		return STM_FAIL;
	}

	thread_hdl = (ql_task_t *)thread_handle;

	ret = ql_rtos_task_delete(*thread_hdl);
	if (ret)
	{
		LOGI("Prob in pthread_cancel, destroy handle anyway\n");
		free(thread_handle);
		return STM_FAIL;
	}

	free(thread_handle);
	return STM_OK;
}

/* -------- Semaphores ---------- */
void *hosted_create_semaphore(int init_value)
{

	// semaphore_handle_t *sem_id = NULL;
	// osSemaphoreDef(sem_template_ctrl);
	ql_sem_t *sem_id = NULL;
	sem_id = (ql_sem_t *)hosted_malloc(
		sizeof(ql_sem_t));

	if (!sem_id)
	{
		LOGI("Sem allocation failed\n");
		return NULL;
	}

	int err = ql_rtos_semaphore_create(&sem_id, 1);
	// *sem_id = osSemaphoreCreate(osSemaphore(sem_template_ctrl), 1);

	// if (!*sem_id)
	// {
	// 	LOGI("sem create failed\n");
	// 	return NULL;
	// }

	return err;
}

unsigned int sleep(unsigned int seconds)
{
	ql_rtos_task_sleep_ms(seconds * 1000);
	return 0;
}

unsigned int msleep(unsigned int mseconds)
{
	ql_rtos_task_sleep_ms(mseconds);
	return 0;
}

int hosted_get_semaphore(void *semaphore_handle, int timeout)
{
	// semaphore_handle_t *sem_id = NULL;
	ql_sem_t *sem_id = NULL;
	if (!semaphore_handle)
	{
		LOGI("Uninitialized sem id 1\n\r");
		return STM_FAIL;
	}

	// sem_id = (semaphore_handle_t *)semaphore_handle;
	sem_id = (ql_sem_t *)semaphore_handle;

	if (!*sem_id)
	{
		LOGI("Uninitialized sem id 2\n\r");
		return STM_FAIL;
	}

	if (!timeout)
	{
		/* non blocking */
		// return osSemaphoreWait(*sem_id, 0);
		return ql_rtos_semaphore_wait(*sem_id, 0);
		;
	}
	else if (timeout < 0)
	{
		/* Blocking */
		return ql_rtos_semaphore_wait(*sem_id, QL_WAIT_FOREVER);
	}
	else
	{
		return ql_rtos_semaphore_wait(*sem_id, SEC_TO_MILLISEC(timeout));
	}
}

int hosted_post_semaphore(void *semaphore_handle)
{
	// semaphore_handle_t *sem_id = NULL;
	ql_sem_t *sem_id = NULL;

	if (!semaphore_handle)
	{
		LOGI("Uninitialized sem id 3\n");
		return STM_FAIL;
	}

	// sem_id = (semaphore_handle_t *)semaphore_handle;
	sem_id = (ql_sem_t *)semaphore_handle;
	return ql_rtos_semaphore_release(*sem_id);
}

int hosted_destroy_semaphore(void *semaphore_handle)
{
	int ret = STM_OK;
	// semaphore_handle_t *sem_id = NULL;
	ql_sem_t *sem_id = NULL;
	if (!semaphore_handle)
	{
		LOGI("Uninitialized sem id 4\n");
		return STM_FAIL;
	}

	sem_id = (ql_sem_t *)semaphore_handle;

	ret = ql_rtos_semaphore_delete(*sem_id);
	if (ret)
		LOGI("Failed to destroy sem\n");

	free(semaphore_handle);

	return ret;
}
/* -------- Timers  ---------- */
int hosted_timer_stop(void *timer_handle)
{
	int ret = STM_OK;

	if (timer_handle)
	{
		ret = ql_rtos_timer_stop((ql_timer_t)timer_handle);
		if (ret < 0)
			LOGI("Failed to stop timer\n");

		ret = ql_rtos_timer_delete((ql_timer_t)timer_handle);
		if (ret < 0)
			LOGI("Failed to delete timer\n");

		free(timer_handle);
		return ret;
	}
	return STM_FAIL;
}

/* Sample timer_handler looks like this:
 *
 * void expired(union sigval timer_data){
 *     struct mystruct *a = timer_data.sival_ptr;
 * 	LOGI("Expired %u\n", a->mydata++);
 * }
 **/
ql_task_t timerNew = NULL;
void *hosted_timer_start(int duration, int type,
						 void (*timeout_handler)(void const *), void *arg)
{
	struct timer_handle_t *timer_handle = NULL;
	int ret = STM_OK;
	// os_timer_type timer_type = osTimerOnce;
	// osTimerDef(timerNew, timeout_handler);
	ql_timer_t timer_type;
	ql_rtos_timer_create(&timer_type, timerNew, timeout_handler, NULL);

		/* alloc */
		// timer_handle = (struct timer_handle_t *)hosted_malloc(
		// 	sizeof(struct timer_handle_t));
		timer_type = (ql_timer_t *)hosted_malloc(sizeof(ql_timer_t));
	if (!timer_handle)
	{
		LOGI("Memory allocation failed for timer\n");
		return NULL;
	}

	/* timer type */
	if (type == CTRL__TIMER_PERIODIC)
	{
		// timer_type = osTimerPeriodic;
	}
	else if (type == CTRL__TIMER_ONESHOT)
	{
		// timer_type = osTimerOnce;
	}
	else
	{
		LOGI("Unsupported timer type. supported: one_shot, periodic\n");
		free(timer_handle);
		return NULL;
	}

	/* create */
	// timer_handle->timer_id =
	// 	osTimerCreate(osTimer(timerNew),
	// 				  timer_type, arg);

	// if (!timer_handle->timer_id)
	// {
	// 	LOGI("Failed to create timer\n");
	// 	free(timer_handle);
	// 	return NULL;
	// }
	ql_timer_t timer_id;
	ql_rtos_timer_create(&timer_id, timerNew, timeout_handler, NULL);

	/* start */
	ret = osTimerStart(timer_id, SEC_TO_MILLISEC(duration));
	if (ret)
	{
		LOGI("Failed to start timer, destroying timer\n");

		ret = osTimerDelete(timer_id);
		if (ret)
			LOGI("Failed to delete timer\n");

		free(timer_handle);
		return NULL;
	}

	return timer_handle;
}

/* -------- Serial Drv ---------- */
struct serial_drv_handle_t *serial_drv_open(const char *transport)
{
	struct serial_drv_handle_t *serial_drv_handle = NULL;
	if (!transport)
	{
		LOGI("Invalid parameter in open \n\r");
		return NULL;
	}

	if (serial_drv_handle)
	{
		LOGI("return orig hndl\n");
		return serial_drv_handle;
	}

	serial_drv_handle = (struct serial_drv_handle_t *)hosted_calloc(1, sizeof(struct serial_drv_handle_t));
	if (!serial_drv_handle)
	{
		LOGI("Failed to allocate memory \n");
		return NULL;
	}

	return serial_drv_handle;
}

int serial_drv_write(struct serial_drv_handle_t *serial_drv_handle,
					 uint8_t *buf, int in_count, int *out_count)
{
	int ret = 0;
	if (!serial_drv_handle || !buf || !in_count || !out_count)
	{
		LOGI("Invalid parameters in write\n\r");
		return STM_FAIL;
	}

	if ((!serial_ll_if_g) ||
		(!serial_ll_if_g->fops) ||
		(!serial_ll_if_g->fops->write))
	{
		LOGI("serial interface not valid\n\r");
		return STM_FAIL;
	}

	ret = serial_ll_if_g->fops->write(serial_ll_if_g, buf, in_count);
	if (ret != STM_OK)
	{
		*out_count = 0;
		LOGI("Failed to write data\n\r");
		return STM_FAIL;
	}

	*out_count = in_count;
	return STM_OK;
}

uint8_t *serial_drv_read(struct serial_drv_handle_t *serial_drv_handle,
						 uint32_t *out_nbyte)
{
	uint16_t init_read_len = 0;
	uint16_t rx_buf_len = 0;
	uint8_t *read_buf = NULL;
	int ret = 0;
	/* Any of `CTRL_EP_NAME_EVENT` and `CTRL_EP_NAME_RESP` could be used,
	 * as both have same strlen in adapter.h */
	const char *ep_name = CTRL_EP_NAME_RESP;
	uint8_t *buf = NULL;
	uint32_t buf_len = 0;

	if (!serial_drv_handle || !out_nbyte)
	{
		LOGI("Invalid parameters in read\n\r");
		return NULL;
	}

	*out_nbyte = 0;

	if (!readSemaphore)
	{
		LOGI("Semaphore not initialized\n\r");
		return NULL;
	}
	if (ql_rtos_semaphore_wait(readSemaphore, QL_WAIT_FOREVER) != 1)
	{
		LOGI("Failed to read data \n\r");
		return NULL;
	}

	if ((!serial_ll_if_g) ||
		(!serial_ll_if_g->fops) ||
		(!serial_ll_if_g->fops->read))
	{
		LOGI("serial interface refusing to read\n\r");
		return NULL;
	}

	/* Get buffer from serial interface */
	read_buf = serial_ll_if_g->fops->read(serial_ll_if_g, &rx_buf_len);
	if ((!read_buf) || (!rx_buf_len))
	{
		LOGI("serial read failed\n\r");
		return NULL;
	}
	print_hex_dump(read_buf, rx_buf_len, "Serial read data");

	/*
	 * Read Operation happens in two steps because total read length is unknown
	 * at first read.
	 *      1) Read fixed length of RX data
	 *      2) Read variable length of RX data
	 *
	 * (1) Read fixed length of RX data :
	 * Read fixed length of received data in below format:
	 * ----------------------------------------------------------------------------
	 *  Endpoint Type | Endpoint Length | Endpoint Value  | Data Type | Data Length
	 * ----------------------------------------------------------------------------
	 *
	 *  Bytes used per field as follows:
	 *  ---------------------------------------------------------------------------
	 *      1         |       2         | Endpoint Length |     1     |     2     |
	 *  ---------------------------------------------------------------------------
	 *
	 *  int_read_len = 1 + 2 + Endpoint length + 1 + 2
	 */

	init_read_len = SIZE_OF_TYPE + SIZE_OF_LENGTH + strlen(ep_name) +
					SIZE_OF_TYPE + SIZE_OF_LENGTH;

	if (rx_buf_len < init_read_len)
	{
		free(read_buf);
		LOGI("Incomplete serial buff, return\n");
		return NULL;
	}

	HOSTED_CALLOC(buf, init_read_len);

	memcpy(buf, read_buf, init_read_len);

	/* parse_tlv function returns variable payload length
	 * of received data in buf_len
	 **/
	ret = parse_tlv(buf, &buf_len);
	if (ret || !buf_len)
	{
		free(buf);
		LOGI("Failed to parse RX data \n\r");
		goto free_bufs;
	}

	if (rx_buf_len < (init_read_len + buf_len))
	{
		LOGI("Buf read on serial iface is smaller than expected len\n");
		free(buf);
		goto free_bufs;
	}

	free(buf);
	/*
	 * (2) Read variable length of RX data:
	 */
	HOSTED_CALLOC(buf, buf_len);

	memcpy((buf), read_buf + init_read_len, buf_len);

	free(read_buf);

	*out_nbyte = buf_len;
	return buf;

free_bufs:
	free(read_buf);
	free(buf);
	return NULL;
}

int serial_drv_close(struct serial_drv_handle_t **serial_drv_handle)
{
	if (!serial_drv_handle || !(*serial_drv_handle))
	{
		LOGI("Invalid parameter in close \n\r");
		if (serial_drv_handle)
			free(serial_drv_handle);
		return STM_FAIL;
	}
	free(*serial_drv_handle);
	return STM_OK;
}
