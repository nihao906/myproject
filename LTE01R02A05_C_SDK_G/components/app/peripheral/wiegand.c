
#include "osi_api.h"
#include "ql_gpio.h"
#include "stdlib.h"
#include "ModbusS.h"
#include "ql_log.h"
#define LOG_TAG "[wiegand]:"

#define LOGD(msg, ...) QL_LOG(QL_LOG_LEVEL_DEBUG, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGI(msg, ...) QL_LOG(QL_LOG_LEVEL_INFO, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGW(msg, ...) QL_LOG(QL_LOG_LEVEL_WARN, LOG_TAG, msg, ##__VA_ARGS__)
#define LOGE(msg, ...) QL_LOG(QL_LOG_LEVEL_ERROR, LOG_TAG, msg, ##__VA_ARGS__)

#define DI0_GPIO_NUM GPIO_1
#define DI0_PIN_NUM 63
#define DI1_GPIO_NUM GPIO_5
#define DI1_PIN_NUM 62

uint32_t wiegand_data = 0;
uint64_t wiegand_last_time = 0;
int wiegand_bit_count = 10;
const int wiegand_bit_timeout = 20;

extern osiThread_t *rtk_thread;
static void check_wiegand_data(uint32_t data);
static void gpio_IntrCB(void *data)
{
  bool gpioLevel = 0;
  uint64_t now = osiUpTime();
  // gWordVar[REG_WIEGAND_ERR_CNT + 1] = now & 0xFFFF;
  // gWordVar[REG_WIEGAND_ERR_CNT + 2] = (now >> 16) & 0xFFFF;
  if (data == (void *)DI1_GPIO_NUM)
  {
    // gWordVar[REG_WIEGAND_ERR_CNT + 3]++;
    //  if (LSAPI_Device_Read(GPIO_DI0, (void *)&gpioLevel, 1) == 1)
    //  {
    //    if (gpioLevel == 0)
    //    {
    if (now - wiegand_last_time > wiegand_bit_timeout)
    {
      wiegand_bit_count = 25;
    }
    wiegand_data |= (1 << wiegand_bit_count);
    wiegand_last_time = now;
    if (--wiegand_bit_count < 0)
    {
      check_wiegand_data(wiegand_data);
      wiegand_data = 0;
      wiegand_bit_count = 25;
    }
    // }
    // else // 错误时序
    // {
    //   wiegand_last_time = 0;
    // }
  }
  // }
  else if (data == (void *)DI0_GPIO_NUM)
  {
    // gWordVar[REG_WIEGAND_ERR_CNT + 4]++;
    //  if (LSAPI_Device_Read(GPIO_DI1, (void *)&gpioLevel, 1) == 1)
    //  {
    //    if (gpioLevel == 0)
    //    {
    if (now - wiegand_last_time > wiegand_bit_timeout)
    {
      wiegand_bit_count = 25;
    }
    wiegand_data &= ~(1 << wiegand_bit_count);
    wiegand_last_time = now;
    if (--wiegand_bit_count < 0)
    {
      check_wiegand_data(wiegand_data);
      wiegand_data = 0;
      wiegand_bit_count = 25;
    }
    // }
    // else // 错误时序
    // {
    //   wiegand_last_time = 0;
    // }
  }

  // }
}

void wiegand_init(void)
{
  int ret;
  ret = ql_pin_set_gpio(DI0_PIN_NUM);
  if (ret != QL_GPIO_SUCCESS)
  {
    LOGE("ql_pin_set_func 66 err ret=%d", ret);
  }
  ret = ql_int_register(DI0_GPIO_NUM, EDGE_TRIGGER, DEBOUNCE_EN, EDGE_RISING, PULL_DOWN, gpio_IntrCB, (void *)DI0_GPIO_NUM);
  if (ret != QL_GPIO_SUCCESS)
  {
    LOGE("ql_int_register GPIO_%d err ret=%d", DI0_GPIO_NUM, ret);
  }
  ret = ql_pin_set_gpio(DI1_PIN_NUM);
  if (ret != QL_GPIO_SUCCESS)
  {
    LOGE("ql_pin_set_func %d err ret=%d", DI1_PIN_NUM, ret);
  }
  ret = ql_int_register(DI1_GPIO_NUM, EDGE_TRIGGER, DEBOUNCE_EN, EDGE_RISING, PULL_DOWN, gpio_IntrCB, (void *)DI1_GPIO_NUM);
  if (ret != QL_GPIO_SUCCESS)
  {
    LOGE("ql_int_register GPIO_%d err ret=%d", DI1_GPIO_NUM, ret);
  }
  ql_int_enable(DI0_GPIO_NUM);
  ql_int_enable(DI1_GPIO_NUM);
}

static void check_wiegand_data(uint32_t data)
{
  int low13_bit = data & 0x1fff;
  int high13_bit = (data >> 13) & 0x1fff;
  int low_parity = 0;
  int hi_parity = 0;
  for (int i = 0; i < 13; i++)
  {
    if (low13_bit & (1 << i))
    {
      low_parity++;
    }
    if (high13_bit & (1 << i))
    {
      hi_parity++;
    }
  }

  if ((low_parity % 2 == 1) && (hi_parity % 2 == 0))
  {
    data = (data >> 1) & 0xffffff;
    gWordVar[REG_WIEGAND_ID_LOW] = data & 0xffff;
    gWordVar[REG_WIEGAND_ID_HIGH] = data >> 16;
    gWordVar[REG_WIEGAND_OK_CNT]++;
    osiEvent_t event;
    event.id = 1000; //
    event.param1 = data;
    if (rtk_thread != NULL)
    {
      osiEventSend(rtk_thread, &event);
    }
  }
  else
  {
    LOGE("wiegand data error\n");
    // data = (data >> 1) & 0xffffff;
    gWordVar[REG_WIEGAND_ID_ERR_LOW] = data & 0xffff;
    gWordVar[REG_WIEGAND_ID_ERR_HIGH] = data >> 16;
    gWordVar[REG_WIEGAND_ERR_CNT]++;
    // gWordVar[REG_WIEGAND_ERR_CNT + 2] = low_parity;
    // gWordVar[REG_WIEGAND_ERR_CNT + 3] = hi_parity;
    // gWordVar[REG_WIEGAND_ERR_CNT + 4] = low13_bit;
    // gWordVar[REG_WIEGAND_ERR_CNT + 5] = high13_bit;
  }
}

uint16_t read_wiegand_io(void)
{
  ql_LvlMode low = 0;
  ql_LvlMode hi = 0;
  ql_gpio_get_level(DI0_GPIO_NUM, &low);
  ql_gpio_get_level(DI1_GPIO_NUM, &hi);
  return (hi << 8) | low;
}