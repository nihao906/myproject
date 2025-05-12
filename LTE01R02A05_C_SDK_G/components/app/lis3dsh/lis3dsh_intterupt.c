/*
 ******************************************************************************
 * @file    fifo_stream.c
 * @author  Sensors Software Solution Team
 * @brief   This file shows how to extract data from the sensor in
 *          polling mode.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/*
 * This example was developed using the following STMicroelectronics
 * evaluation boards:
 *
 * - STEVAL_MKI109V3 + STEVAL-MKI134V1
 * - NUCLEO_F411RE +STEVAL-MKI134V1
 *
 * and STM32CubeMX tool with STM32CubeF4 MCU Package
 *
 * Used interfaces:
 *
 * STEVAL_MKI109V3    - Host side:   USB (Virtual COM)
 *                    - Sensor side: SPI(Default) / I2C(supported)
 *
 * NUCLEO_STM32F411RE - Host side: UART(COM) to USB bridge
 *                    - Sensor side: I2C(Default) / SPI(supported)
 *
 * If you need to run this example on a different hardware platform a
 * modification of the functions: `platform_write`, `platform_read`,
 * `tx_com` and 'platform_init' is required.
 *
 */

/* STMicroelectronics evaluation boards definition
 *
 * Please uncomment ONLY the evaluation boards in use.
 * If a different hardware is used please comment all
 * following target board and redefine yours.
 */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdio.h>
#include "ql_api_osi.h"
#include "osi_api.h"
#include "ql_i2c.h"
#include "ql_gpio.h"
#include "lis3dsh_reg.h"
#include "ql_log.h"
#include "fft_lib/inc/dsps_fft2r.h"
#include "fft_lib/inc/dsps_wind_hann.h"
#include "json_file_ops.h"
#include "ModbusS.h"
#include "../t2n/include/udp_ota_shell.h"

#define LOG_TAG "[lis3dsh]:"

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, msg, ##__VA_ARGS__)

#define LIS3DSH_GPIO_NUM GPIO_1
#define LIS3DSH_GPIO_PIN 58
#define LIS3DSH_I2C i2c_2

stmdev_ctx_t dev_ctx;

// xyz_ref_t xyz_ref = {.ax_ref = 0, .ay_ref = 0, .az_ref = 0, .norm_ref = 0.0};

// extern parameter shock_count_config;

/* set read single command. Attention: command must be 0x3F at most */
#define SET_READ_SINGLE_CMD(x) (x | 0x80)
/* set read multiple command. Attention: command must be 0x3F at most */
#define SET_READ_MULTI_CMD(x) (x | 0xC0)
/* set write single command. Attention: command must be 0x3F at most */
#define SET_WRITE_SINGLE_CMD(x) (x & (~(0xC0)))
/* set write multiple command. Attention: command must be 0x3F at most */
#define SET_WRITE_MULTI_CMD(x) ((x & (~0x80)) | 0x40)
/* Private macro -------------------------------------------------------------*/
#define BOOT_TIME 10 // ms

#define FREQUENCY 100

/* Private variables ---------------------------------------------------------*/
static uint8_t rx_buffer[1000];

#define N_SAMPLES 1024
int N = N_SAMPLES;
// Input test array
__attribute__((aligned(16)))
float x1[N_SAMPLES * 2];
__attribute__((aligned(16)))
float x2[N_SAMPLES * 2];
__attribute__((aligned(16)))
float x3[N_SAMPLES * 2];
// Window coefficients
__attribute__((aligned(16)))
float wind[N_SAMPLES];
// working complex array
__attribute__((aligned(16)))
float y_cf[N_SAMPLES * 2];
// Pointers to result arrays
float *y1_cf = &y_cf[0];
float *y2_cf = &y_cf[N_SAMPLES];

// Sum of y1 and y2
__attribute__((aligned(16)))
float sum_y[N_SAMPLES / 2];

int buf_r = 0;
int fft_h = 0;
static osiSemaphore_t *fft_sema = NULL;
static osiSemaphore_t *gpio_int_sema = NULL;

// static twai_message_t data_message = {.identifier = 0x1e, .data_length_code = 8, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
// extern struct tcp_pcb *raw_data_pcb;
// extern float wind[N_SAMPLES];
// extern dsps_fft_buff_t y_buff;

// extern uint8_t flag_precision;
// extern uint8_t flag_raw_data;
/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
// static void tx_com(uint8_t *tx_buffer, uint16_t len);
static void platform_delay(uint32_t ms);
static int platform_init(void);
void spi_reg_write(void *handle, uint8_t addr, uint8_t *buf, int len);
void spi_reg_read(void *handle, uint8_t addr, uint8_t *buf, int len);


static void gpio_isr_handler(void *arg)
{
  // OSI_UNUSED(arg);
  osiSemaphoreRelease(gpio_int_sema);
}

extern void tcp_write_to_cilents(uint8_t *data, int len);
/* Main Example --------------------------------------------------------------*/
void lis3dsh_task(void)
{

  lis3dsh_pin_int1_route_t int1_route;
  lis3dsh_all_sources_t all_sources;
  lis3dsh_bus_mode_t bus_mode;
  lis3dsh_status_var_t status;
  lis3dsh_ctrl_reg3_t ctrl_reg3;
  lis3dsh_id_t id;
  lis3dsh_md_t md;
  lis3dsh_reg_t reg;

  /* Initialize mems driver interface */
  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = (void *)platform_init();

  /* Initialize platform specific hardware */
 
  /* Wait sensor boot time */
  platform_delay(BOOT_TIME);
  /* Check device ID */
  lis3dsh_id_get(&dev_ctx, &id);
  LOGI("lis3dsh_id=%02x%02x%02x", id.whoami, id.info1, id.info2);
  while (id.whoami != LIS3DSH_ID)
  {
    platform_delay(100);
    memset(&id, 0, sizeof(id));
    lis3dsh_id_get(&dev_ctx, &id);
    LOGI("lis3dsh_id=%02x%02x%02x", id.whoami, id.info1, id.info2);
  }

  /* Restore default configuration */
  lis3dsh_init_set(&dev_ctx, LIS3DSH_RESET);

  do
  {

    lis3dsh_status_get(&dev_ctx, &status);

  } while (status.sw_reset);

  /* Set bdu and if_inc recommended for driver usage */

  lis3dsh_init_set(&dev_ctx, LIS3DSH_DRV_RDY);

  /* Select bus interface */

  bus_mode = LIS3DSH_SEL_BY_HW;
  lis3dsh_bus_mode_set(&dev_ctx, &bus_mode);

  /* Set Output Data Rate */

  lis3dsh_mode_get(&dev_ctx, &md);
  md.fs = LIS3DSH_2g;
  md.odr = LIS3DSH_1kHz6;
  lis3dsh_mode_set(&dev_ctx, &md);

  /* FIFO configuration */
  lis3dsh_read_reg(&dev_ctx, LIS3DSH_FIFO_CTRL, &reg.byte, 1);
  // reg.byte = 0;
  reg.fifo_ctrl.fmode = 2; /* FIFO stream mode */
  reg.fifo_ctrl.wtmp = 16;
  lis3dsh_write_reg(&dev_ctx, LIS3DSH_FIFO_CTRL, &reg.byte, 1);

  lis3dsh_read_reg(&dev_ctx, LIS3DSH_CTRL_REG6, &reg.byte, 1);
  // reg.byte = 0;
  LOGI("LIS3DSH_CTRL_REG6 old=%02x", reg.byte);
  // reg.byte = 0;
  reg.ctrl_reg6.add_inc = PROPERTY_ENABLE;
  reg.ctrl_reg6.fifo_en = PROPERTY_ENABLE;
  reg.ctrl_reg6.wtm_en = PROPERTY_ENABLE;
  lis3dsh_write_reg(&dev_ctx, LIS3DSH_CTRL_REG6, &reg.byte, 1);

  lis3dsh_read_reg(&dev_ctx, LIS3DSH_CTRL_REG6, &reg.byte, 1);
  LOGI("LIS3DSH_CTRL_REG6 new=%02x", reg.byte);

  lis3dsh_read_reg(&dev_ctx, LIS3DSH_CTRL_REG3, (uint8_t *)&ctrl_reg3, 1);
  // memset(&ctrl_reg3, 0, sizeof(ctrl_reg3));
  ctrl_reg3.int1_en = PROPERTY_ENABLE;
  // ctrl_reg3.dr_en = PROPERTY_ENABLE;
  lis3dsh_write_reg(&dev_ctx, LIS3DSH_CTRL_REG3, (uint8_t *)&ctrl_reg3, 1);

  lis3dsh_pin_int1_route_get(&dev_ctx, &int1_route);
  // memset(&int1_route, 0, sizeof(int1_route));
  //  int1_route.drdy_xl   = PROPERTY_DISABLE;
  //  int1_route.drdy_xl = PROPERTY_ENABLE;
  int1_route.fifo_th = PROPERTY_ENABLE;
  lis3dsh_pin_int1_route_set(&dev_ctx, &int1_route);

  // int cnt = 0;
  /* Read samples in interrupt mode (no int). */
  // uint32_t io_num;
  while (1)
  {
    if (osiSemaphoreTryAcquire(gpio_int_sema, 500))
    {
      // LOGI("int!");
      // lis3dsh_all_sources_get(&dev_ctx, &all_sources);
      // if (all_sources.drdy_xl)
      // {
      lis3dsh_read_reg(&dev_ctx, LIS3DSH_FIFO_SRC, &reg.byte, 1);
      // LOGI("fss:%d", reg.fifo_src.fss);

      for (int i = 0; i < reg.fifo_src.fss; i++)
      {
        // acc_data_t *data_buff = lis3dsh_fifo_get_write();
        lis3dsh_read_reg(&dev_ctx, LIS3DSH_OUT_X_L, rx_buffer, 6);
        // LOGI("read %d:%f, %f, %f", i, (int16_t)(rx_buffer[0] | (rx_buffer[1] << 8)) * 2.0 / 0x7fff, (int16_t)(rx_buffer[2] | (rx_buffer[3] << 8)) * 2.0 / 0x7fff, (int16_t)(rx_buffer[4] | (rx_buffer[5] << 8)) * 2.0 / 0x7fff);
       
        x1[buf_r] = (int16_t)(rx_buffer[0] | (rx_buffer[1] << 8)) * 2.0 / 0x7fff;
        x2[buf_r] = (int16_t)(rx_buffer[2] | (rx_buffer[3] << 8)) * 2.0 / 0x7fff + 1;
        x3[buf_r] = (int16_t)(rx_buffer[4] | (rx_buffer[5] << 8)) * 2.0 / 0x7fff;

        buf_r++;
        if (buf_r == N_SAMPLES || buf_r == N_SAMPLES * 2)
        {
          // LOGI("send fft sema");
          fft_h = buf_r - N_SAMPLES;
          osiSemaphoreRelease(fft_sema);
        }
        if (buf_r == N_SAMPLES * 2)
        {
          buf_r = 0;
        }
      }
    }
  }

  ql_rtos_task_delete(NULL);
}

float *feq_min = &gWordVar[REG_SHAKE_FEQ_MIN_CFG];
float *feq_max = &gWordVar[REG_SHAKE_FEQ_MAX_CFG];
float *a_th = &gWordVar[REG_SHAKE_A_TH_CFG];
int16_t *is_shake = &gWordVar[REG_VIBRATE_SENSOR];
float *max_feq = &gWordVar[REG_SHAKE_MAX_FEQ];
float *max_a = &gWordVar[REG_SHAKE_MAX_A];

// 检测是否发生震动，根据频谱图，判断频率范围中的最大振幅是否超过阈值
bool detect_shake(float feq_min, float feq_max, float a_th)
{
  float max_a_t = 0;
  float max_feq_t = 0;
  // 获得最高幅值以及其所在的频率
  for (int i = 0; i < N_SAMPLES / 2; i++)
  {
    if (sum_y[i] > max_a_t)
    {
      max_a_t = sum_y[i];
      max_feq_t = i * 1600.0 / N_SAMPLES;
    }
  }
  *max_feq = max_feq_t;
  *max_a = max_a_t;
  // esp32_ota_log(LINFO, "max feq:%f, max a:%f", *max_feq, *max_a);
  LOGI("max feq:%f, max a:%f", *max_feq, *max_a);

  float a_max = 0;
  // int i_min = (int)ceil((feq_min / 1600.0 * N_SAMPLES));
  // int i_max = (int)floor(feq_max / 1600.0 * N_SAMPLES);
  int i_min = (int)feq_min;
  int i_max = (int)feq_max;
  for (int i = i_min; i <= i_max; i++)
  {
    a_max = a_max >= sum_y[i] ? a_max : sum_y[i];
  }
  // esp32_ota_log(LINFO, "i:%d->%d  a_max:%f  a_th:%f", i_min, i_max, a_max, a_th);
  LOGI("i:%d->%d  a_max:%f  a_th:%f", i_min, i_max, a_max, a_th);
  
  if (*is_shake == false && a_max >= a_th)
  {
    return true;
  }
  if (*is_shake == true && a_max >= a_th * 0.5)
  {
    return true;
  }
  else 
  {
    return false;
  }
}

uint8_t data_tlv[N_SAMPLES * 4 + 4];
void fft_task(void)
{
  int ret = 0;
  
  // 加载配置参数
  ret = load_shake_cfg(feq_min, feq_max, a_th, SHAKE_CFG_FILE_PATH);
  if (ret != 0)
  {
    LOGI("no cfg file, load default param");
    *feq_min = 10;
    *feq_max = 25;
    *a_th = 0.06;
  }
  LOGI("load shake cfg:%f, %f, %f", *feq_min, *feq_max, *a_th);

  // fft 初始化
  ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
  if (ret  != ESP_OK) {
      LOGI("Not possible to initialize FFT. Error = %i", ret);
      goto exit;
  }
  // Generate hann window
  dsps_wind_hann_f32(wind, N);
  
  while (1)
  {
    int ret;
    // LOGI("wait sema");
    if (osiSemaphoreTryAcquire(fft_sema, 500))  
    {
      // LOGI("cal fft %d:", fft_h);
      // Convert two input vectors to one complex vector
      for (int i = 0 ; i < N ; i++) {
          y_cf[i * 2 + 0] = x2[fft_h + i] * wind[i];
          y_cf[i * 2 + 1] = x3[fft_h + i] * wind[i];
      }
      // FFT
      dsps_fft2r_fc32(y_cf, N);
      // Bit reverse
      dsps_bit_rev_fc32(y_cf, N);
      // Convert one complex vector to two complex vectors
      dsps_cplx2reC_fc32(y_cf, N);

      // 归一化处理参考：https://blog.csdn.net/weixin_39591031/article/details/110392352
      for (int i = 0 ; i < N / 2 ; i++) {
          int normal_div = N / 2.0;
          if (i == 0) normal_div = N;
          y1_cf[i] = (sqrt(y1_cf[i * 2 + 0] * y1_cf[i * 2 + 0] + y1_cf[i * 2 + 1] * y1_cf[i * 2 + 1]) / normal_div);
          y2_cf[i] = (sqrt(y2_cf[i * 2 + 0] * y2_cf[i * 2 + 0] + y2_cf[i * 2 + 1] * y2_cf[i * 2 + 1]) / normal_div);
          // Simple way to show two power spectrums as one plot
          sum_y[i] = fmax(y1_cf[i], y2_cf[i]);
          float freq = i * 1.0 / N * 1600;  // 单位HZ
          LOGI("fft %d: f=%f, y=%f, z=%f sum=%f", i, freq, y1_cf[i], y2_cf[i], sum_y[i]);
      }

      // 震动检测
      *is_shake = detect_shake(*feq_min, *feq_max, *a_th);
      LOGI("is_shake = %d", *is_shake);

      // 上报数据到tcp clients tlv格式
      if (gWordVar[REG_SHAKE_TCP_WRITE_DATA_CLASS_SELECT] & 0x01)   // 控制上报
      {
        LOGI("send to python, raw_data");
        data_tlv[0] = 0xE1;         // 大类：原始数据
        data_tlv[1] = 0x00;         // 小类：哪组数据
        data_tlv[2] = (N * 4) & 0xff;     // 数据长度
        data_tlv[3] = ((N * 4) >> 8) & 0xff;
        if (gWordVar[REG_SHAKE_TCP_WRITE_RAW_DATA_SELECT] & 0x01)
        {
          LOGI("send to python, raw_data1");
          data_tlv[1] = 0x00;
          memcpy(data_tlv + 4, &x1[fft_h], N * sizeof(float));
          tcp_write_to_cilents(data_tlv, N * sizeof(float) + 4);
        }
        if (gWordVar[REG_SHAKE_TCP_WRITE_RAW_DATA_SELECT] & 0x02)
        {
          LOGI("send to python, raw_data2");
          data_tlv[1] = 0x01;
          memcpy(data_tlv + 4, &x2[fft_h], N * sizeof(float));
          tcp_write_to_cilents(data_tlv, N * sizeof(float) + 4);
        }
        if (gWordVar[REG_SHAKE_TCP_WRITE_RAW_DATA_SELECT] & 0x04)
        {
          LOGI("send to python, raw_data3");
          data_tlv[1] = 0x02;
          memcpy(data_tlv + 4, &x3[fft_h], N * sizeof(float));
          tcp_write_to_cilents(data_tlv, N * sizeof(float) + 4);
        }
      } 

      if (gWordVar[REG_SHAKE_TCP_WRITE_DATA_CLASS_SELECT] & 0x02)   // 控制上报
      {
        LOGI("send to python, feq_data");
        data_tlv[0] = 0xE2; // 震动频谱图        
        data_tlv[1] = 0x00;         
        data_tlv[2] = (N / 2 * 4) & 0xff;   
        data_tlv[3] = ((N / 2 * 4) >> 8) & 0xff;
        if (gWordVar[REG_SHAKE_TCP_WRITE_FEQ_DATA_SELECT] & 0x01)
        {
          LOGI("send to python, feq_data1");
          data_tlv[1] = 0x00;
          memcpy(data_tlv + 4, y1_cf, (N / 2) * sizeof(float));
          tcp_write_to_cilents(data_tlv, (N / 2) * sizeof(float) + 4);
        }
        if (gWordVar[REG_SHAKE_TCP_WRITE_FEQ_DATA_SELECT] & 0x02)
        {
          LOGI("send to python, feq_data2");
          data_tlv[1] = 0x01; 
          memcpy(data_tlv + 4, y2_cf, (N / 2) * sizeof(float));
          tcp_write_to_cilents(data_tlv, (N / 2) * sizeof(float) + 4);
        }
        if (gWordVar[REG_SHAKE_TCP_WRITE_FEQ_DATA_SELECT] & 0x04)
        {
          LOGI("send to python, feq_data3");
          data_tlv[1] = 0x02;
          memcpy(data_tlv + 4, sum_y, (N / 2) * sizeof(float));
          tcp_write_to_cilents(data_tlv, (N / 2) * sizeof(float) + 4);
        }
      } 

      data_tlv[0] = 0xE3; // shake 判断数据       
      data_tlv[1] = 0x00;         
      data_tlv[2] = (14) & 0xff;   
      data_tlv[3] = ((14) >> 8) & 0xff;
      memcpy(data_tlv + 4,  feq_min, sizeof(float));
      memcpy(data_tlv + 8,  feq_max, sizeof(float));
      memcpy(data_tlv + 12, a_th,    sizeof(float));
      memcpy(data_tlv + 16, is_shake, sizeof(int16_t));
      tcp_write_to_cilents(data_tlv, 18);
    }
    else 
    {
      LOGI("wait fft sema failed");
    }
  }

exit:
  ql_rtos_task_delete(NULL);
}



void lis3dsh_task_init(void)
{
    QlOSStatus err = 0;

    fft_sema = osiSemaphoreCreate(1, 0);

    ql_task_t task_ref;
    err = ql_rtos_task_create(&task_ref, 4096, APP_PRIORITY_BELOW_NORMAL, "lis3dsh", lis3dsh_task, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        LOGI("creat lis3dsh_task fail err = %d", err);
    }
    LOGI("creat lis3dsh_task sucess!");

    ql_task_t fft_task_ref;
    err = ql_rtos_task_create(&fft_task_ref, 4096, APP_PRIORITY_BELOW_NORMAL, "fft", fft_task, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        LOGI("creat fft_task fail err = %d", err);
    }
    LOGI("creat fft_task sucess!");
}




/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, uint8_t *bufp,
                              uint16_t len)
{
  int ret = ql_I2cWrite(LIS3DSH_I2C, LIS3DSH_I2C_ADD_L >> 1, reg, bufp, len);
  if (ret != QL_I2C_SUCCESS)
  {
    LOGE("platform_write err ret=%d", ret);
  }
  return ret;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  int ret = ql_I2cRead(LIS3DSH_I2C, LIS3DSH_I2C_ADD_L >> 1, reg, bufp, len);
  if (ret != QL_I2C_SUCCESS)
  {
    LOGE("platform_read err ret=0x%x", ret);
  }
}

/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 *
 */
static void platform_delay(uint32_t ms)
{
  osiThreadSleep(ms);
}

/*
 * @brief  platform specific initialization (platform dependent)
 */
static  int platform_init(void)
{
  LOGI("platform_init");
  int ret;
  ret = ql_I2cInit(LIS3DSH_I2C, FAST_MODE);
  if (ret != QL_I2C_SUCCESS)
  {
    LOGE("ql_I2cInit err ret=%d", ret);
    return ret;
  }
  ql_pin_set_func(57, 1);
  ql_pin_set_func(56, 1);


  gpio_int_sema = osiSemaphoreCreate(1, 0);

  ret = ql_pin_set_gpio(LIS3DSH_GPIO_PIN);
  if (ret != QL_GPIO_SUCCESS)
  {
    LOGE("ql_pin_set_func %d err ret=%d", LIS3DSH_GPIO_PIN, ret);
    return ret;
  }
  ret = ql_int_register(LIS3DSH_GPIO_NUM, EDGE_TRIGGER, DEBOUNCE_EN, EDGE_FALLING, PULL_UP, gpio_isr_handler, (void *)LIS3DSH_GPIO_NUM);
  if (ret != QL_GPIO_SUCCESS)
  {
    LOGE("ql_int_register GPIO_%d err ret=%d", LIS3DSH_GPIO_NUM, ret);
    return ret;
  }
  ql_int_enable(LIS3DSH_GPIO_NUM);

  return LIS3DSH_I2C;
}

// 根据加速度计算倾斜角度

// static twai_message_t ax_data_message = {.identifier = 0x1A, .data_length_code = 8, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
// static twai_message_t ay_data_message = {.identifier = 0x1B, .data_length_code = 8, .data = {0, 0, 0, 0, 0, 0, 0, 0}};
// static twai_message_t az_data_message = {.identifier = 0x1C, .data_length_code = 8, .data = {0, 0, 0, 0, 0, 0, 0, 0}};

// extern uint8_t xyz_ref_flag;
// float ax, ay, az;
// uint16_t acc_count;
// uint32_t ax_val, ay_val, az_val;
// static fifo_data_t fifodata = {.i16[0] = 0, .i16[1] = 0, .i16[2] = 0};
// bool calc_angle()
// {
//   uint8_t ret = 0;
//   while (1)
//   {

//     // vTaskDelay(10);

//     do
//     {
//       vTaskDelay(1);
//       fifo1_get(&fifo, (int16_t *)&fifodata, 3, &ret);
//       // printf("fifo_get\n");
//     } while (ret == 0);

//     MENS_XYZ_STATUS_T tmp;
//     MENS_XYZ_STATUS_T acc = {.AcceX = 0, .AcceY = 0, .AcceZ = 0};
//     float angle;
//     MENS_XYZ_STATUS_T acc_xyz;

//     acc_xyz.AcceX = fifodata.i16[0];
//     acc_xyz.AcceY = fifodata.i16[1];
//     acc_xyz.AcceZ = fifodata.i16[2];

//     ESP_LOGI(TAG, "AcceX :%u,  AcceY :%u,  AcceZ :%u,\n", acc_xyz.AcceX, acc_xyz.AcceY, acc_xyz.AcceZ);
//     acc_count++;
//     if (acc_count > 160)
//     {
//       acc_count = 0;
//       // ax =  atan(acc_xyz.AcceY/sqrt(acc_xyz.AcceY * acc_xyz.AcceY + acc_xyz.AcceZ * acc_xyz.AcceZ))*180/3.14;
//       // ay =  atan(acc_xyz.AcceX /sqrt(acc_xyz.AcceZ * acc_xyz.AcceZ+acc_xyz.AcceX * acc_xyz.AcceX))*180/3.14;
//       // az =  atan(sqrt(acc_xyz.AcceX * acc_xyz.AcceX+acc_xyz.AcceY * acc_xyz.AcceY)/acc_xyz.AcceZ)*180/3.14;

//       ax = atan(acc_xyz.AcceX / sqrt(acc_xyz.AcceY * acc_xyz.AcceY + acc_xyz.AcceZ * acc_xyz.AcceZ)) * 180 / 3.14;
//       ay = atan(acc_xyz.AcceY / sqrt(acc_xyz.AcceZ * acc_xyz.AcceZ + acc_xyz.AcceX * acc_xyz.AcceX)) * 180 / 3.14;
//       az = atan(sqrt(acc_xyz.AcceX * acc_xyz.AcceX + acc_xyz.AcceY * acc_xyz.AcceY) / acc_xyz.AcceZ) * 180 / 3.14;

//       ax_val = (uint32_t)(ax * 10000.0);
//       ay_val = (uint32_t)(ay * 10000.0);
//       az_val = (uint32_t)(az * 10000.0);
//       ax_data_message.data[0] = ax_val & 0xff;
//       ax_data_message.data[1] = (ax_val >> 8) & 0xff;
//       ax_data_message.data[2] = (ax_val >> 16) & 0xff;
//       ax_data_message.data[3] = (ax_val >> 24) & 0xff;

//       ay_data_message.data[0] = ay_val & 0xff;
//       ay_data_message.data[1] = (ay_val >> 8) & 0xff;
//       ay_data_message.data[2] = (ay_val >> 16) & 0xff;
//       ay_data_message.data[3] = (ay_val >> 24) & 0xff;

//       az_data_message.data[0] = az_val & 0xff;
//       az_data_message.data[1] = (az_val >> 8) & 0xff;
//       az_data_message.data[2] = (az_val >> 16) & 0xff;
//       az_data_message.data[3] = (az_val >> 24) & 0xff;

//       // data_message.data[1] = *((uint8_t *)&ax);
//       // data_message.data[2] = *((uint8_t *)&ay);
//       // data_message.data[3] = *((uint8_t *)&az);
//       twai_transmit(&ax_data_message, 10);
//       twai_transmit(&ay_data_message, 10);
//       twai_transmit(&az_data_message, 10);
//       // ESP_LOGI(TAG, "ax_val :%d,  ax_val :%d,  ax_val :%d,\n", ax_val, ay_val, az_val);
//       // ESP_LOGI(TAG, "ax :%5f,  ay :%5f,  az :%5f,\n", ax, ay, az);
//     }

//     if (xyz_ref_flag == 1) // 更新xyz_ref的值
//     {
//       xyz_ref_flag = 0;
//       xyz_ref.ax_ref = acc_xyz.AcceX;
//       xyz_ref.ay_ref = acc_xyz.AcceY;
//       xyz_ref.az_ref = acc_xyz.AcceZ;
//     }
//   }
// }

// bool calc_angle1(fifo_data_t *fifo_data)
// {

//   // vTaskDelay(10);
//   MENS_XYZ_STATUS_T tmp;
//   MENS_XYZ_STATUS_T acc = {.AcceX = 0, .AcceY = 0, .AcceZ = 0};
//   float angle;
//   MENS_XYZ_STATUS_T acc_xyz;

//   acc_xyz.AcceX = fifo_data->i16[0];
//   acc_xyz.AcceY = fifo_data->i16[1];
//   acc_xyz.AcceZ = fifo_data->i16[2];

//   ESP_LOGI(TAG, "AcceX :%u,  AcceY :%u,  AcceZ :%u,\n", acc_xyz.AcceX, acc_xyz.AcceY, acc_xyz.AcceZ);
//   acc_count++;
//   if (acc_count > 160)
//   {
//     acc_count = 0;
//     // ax =  atan(acc_xyz.AcceY/sqrt(acc_xyz.AcceY * acc_xyz.AcceY + acc_xyz.AcceZ * acc_xyz.AcceZ))*180/3.14;
//     // ay =  atan(acc_xyz.AcceX /sqrt(acc_xyz.AcceZ * acc_xyz.AcceZ+acc_xyz.AcceX * acc_xyz.AcceX))*180/3.14;
//     // az =  atan(sqrt(acc_xyz.AcceX * acc_xyz.AcceX+acc_xyz.AcceY * acc_xyz.AcceY)/acc_xyz.AcceZ)*180/3.14;

//     ax = atan(acc_xyz.AcceX / sqrt(acc_xyz.AcceY * acc_xyz.AcceY + acc_xyz.AcceZ * acc_xyz.AcceZ)) * 180 / 3.14;
//     ay = atan(acc_xyz.AcceY / sqrt(acc_xyz.AcceZ * acc_xyz.AcceZ + acc_xyz.AcceX * acc_xyz.AcceX)) * 180 / 3.14;
//     az = atan(sqrt(acc_xyz.AcceX * acc_xyz.AcceX + acc_xyz.AcceY * acc_xyz.AcceY) / acc_xyz.AcceZ) * 180 / 3.14;

//     ax_val = (uint32_t)(ax * 10000.0);
//     ay_val = (uint32_t)(ay * 10000.0);
//     az_val = (uint32_t)(az * 10000.0);
//     ax_data_message.data[0] = ax_val & 0xff;
//     ax_data_message.data[1] = (ax_val >> 8) & 0xff;
//     ax_data_message.data[2] = (ax_val >> 16) & 0xff;
//     ax_data_message.data[3] = (ax_val >> 24) & 0xff;

//     ay_data_message.data[0] = ay_val & 0xff;
//     ay_data_message.data[1] = (ay_val >> 8) & 0xff;
//     ay_data_message.data[2] = (ay_val >> 16) & 0xff;
//     ay_data_message.data[3] = (ay_val >> 24) & 0xff;

//     az_data_message.data[0] = az_val & 0xff;
//     az_data_message.data[1] = (az_val >> 8) & 0xff;
//     az_data_message.data[2] = (az_val >> 16) & 0xff;
//     az_data_message.data[3] = (az_val >> 24) & 0xff;

//     // data_message.data[1] = *((uint8_t *)&ax);
//     // data_message.data[2] = *((uint8_t *)&ay);
//     // data_message.data[3] = *((uint8_t *)&az);
//     twai_transmit(&ax_data_message, 10);
//     // twai_transmit(&ay_data_message, 10);
//     // twai_transmit(&az_data_message, 10);
//     ESP_LOGI(TAG, "ax_val :%d,  ax_val :%d,  ax_val :%d,\n", ax_val, ay_val, az_val);
//     // ESP_LOGI(TAG, "ax :%5f,  ay :%5f,  az :%5f,\n", ax, ay, az);
//   }

//   if (xyz_ref_flag == 1) // 更新xyz_ref的值
//   {
//     xyz_ref_flag = 0;
//     xyz_ref.ax_ref = acc_xyz.AcceX;
//     xyz_ref.ay_ref = acc_xyz.AcceY;
//     xyz_ref.az_ref = acc_xyz.AcceZ;
//   }
//   return 0;
// }