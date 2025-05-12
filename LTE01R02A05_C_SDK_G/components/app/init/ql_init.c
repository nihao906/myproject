/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------

=================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ql_i2c.h"
#include "ql_app_feature_config.h"
#include "ql_api_osi.h"
#include "../peripheral/inc/ql_pin_cfg.h"
#include "ql_log.h"
#include "ql_gpio.h"
// #include "sdmmc.h"
// #include "ql_sdmmc.h"
#include "ql_fs.h"
#include "../esp32_flash/inc/esp32_flash.h"
#include "../audio/inc/audio_demo.h"
// #include "pwm_audio_demo.h"
#include "../t2n/include/cfg.h"
#include "ModbusS.h"
#include "ql_keypad.h"
#include "mqtt_demo.h"

#define QL_INIT_LOG_LEVEL QL_LOG_LEVEL_INFO
#define QL_INIT_LOG(msg, ...) QL_LOG(QL_INIT_LOG_LEVEL, "ql_INIT", msg, ##__VA_ARGS__)
#define QL_INIT_LOG_PUSH(msg, ...) QL_LOG_PUSH("ql_INIT", msg, ##__VA_ARGS__)

extern void lvgl_app_init(void);

static void prvInvokeGlobalCtors(void)
{
    extern void (*__init_array_start[])();
    extern void (*__init_array_end[])();

    size_t count = __init_array_end - __init_array_start;
    for (size_t i = 0; i < count; ++i)
        __init_array_start[i]();
}

typedef struct
{
    uint8_t         pin_num;
    uint8_t         default_func;
    uint8_t         gpio_func;

    ql_GpioNum      gpio_num;
    ql_GpioDir      gpio_dir;
    ql_PullMode     gpio_pull;    //for input only
    ql_LvlMode      gpio_lvl;     //for output only
} ql_gpio_cfg;
static ql_gpio_cfg _ql_gpio_cfg1[] =
{            /* pin_num  default_func     gpio_func    gpio_num       gpio_dir         gpio_pull          gpio_lvl  */
/*STATUS     */{  25 ,        0,              0,        GPIO_0,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*NET_STATUS */{  16 ,        1,              0,        GPIO_2,    GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*MAIN_DC    */{  21 ,        1,              0,        GPIO_3,    QUEC_PIN_NONE,      QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*MAIN_CTS   */{  22 ,        0,              4,        GPIO_15,   QUEC_PIN_NONE,      QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*MAIN_RI    */{  20 ,        0,              0,        GPIO_1,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*USB_BOOT   */{  82 ,        1,              0,        GPIO_28,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*I2C2_SCL   */{  67 ,        1,              0,        GPIO_42,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*I2C2_SDA   */{  66 ,        1,              0,        GPIO_43,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*UART1_RXD  */{  17 ,        1,              0,        GPIO_12,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*UART1_RXD  */{  18 ,        1,              0,        GPIO_13,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE }
};
void ql_pin_cfg_init(void)
{
    uint8_t index = 0;
    uint8_t pin_num = 0;
    uint8_t default_func = 0;
    uint8_t gpio_func = 0;
    ql_GpioNum gpio_num = 0;
    ql_GpioDir gpio_dir = 0;
    ql_PullMode gpio_pull = 0;
    ql_LvlMode gpio_lvl = 0;

    for (index = 0; index < sizeof(_ql_gpio_cfg1)/sizeof(_ql_gpio_cfg1[0]); index++)
    {
        // QL_INIT_LOG("pin%d=%d", index, ql_pin_cfg_map[index].pin_num);
        if (QUEC_PIN_NONE == _ql_gpio_cfg1[index].pin_num)
        {
            QL_INIT_LOG("init exit %d!", index);
            break;
        }
        pin_num = _ql_gpio_cfg1[index].pin_num;
        default_func = _ql_gpio_cfg1[index].default_func;
        gpio_func = _ql_gpio_cfg1[index].gpio_func;
        gpio_num = _ql_gpio_cfg1[index].gpio_num;
        gpio_dir = _ql_gpio_cfg1[index].gpio_dir;
        gpio_pull = _ql_gpio_cfg1[index].gpio_pull;
        gpio_lvl = _ql_gpio_cfg1[index].gpio_lvl;

        ql_pin_set_func(pin_num, gpio_func);
        if (default_func == gpio_func)
        {
            ql_gpio_init(gpio_num, gpio_dir, gpio_pull, gpio_lvl);
        }
    }
}

void ql_keypad_callback(ql_keymatrix_t keymatrix)
{   
    QL_INIT_LOG("keymap:%d keyout:%d keyin:%d pressd:%d", keymatrix.keymap, keymatrix.keyout, keymatrix.keyin, keymatrix.keystate);
}

void text_iic(void)
{
    uint8_t i = 0;
    while(1)
        { 
            if(QL_I2C_SUCCESS == ql_I2cRead(i2c_2, 0x18, 0XFD, &i, 1))
            {
            QL_INIT_LOG("iic read data is %d", &i);
            }
            // ql_I2cRead(i2c_2, 0x18, 0XFD, &i, 1);

            ql_rtos_task_sleep_ms(200);
        }  
} 

static void ql_init_app_thread(void *param)
{
    QlOSStatus err = QL_OSI_SUCCESS;
    ql_task_t ql_init_task = NULL;
    QL_INIT_LOG("init demo thread enter, param 0x%x", param);
    ql_mqtt_app_init();

/*Caution:If the macro of secure boot and the function are opened, download firmware and restart will enable secure boot.
          the secret key cannot be changed forever*/
#ifdef QL_APP_FEATURE_SECURE_BOOT
    // ql_dev_enable_secure_boot();
#endif

#if 0
    ql_gpio_app_init();
    ql_gpioint_app_init();
#endif

#ifdef QL_APP_FEATURE_LEDCFG
    // ql_ledcfg_app_init();
#endif

#ifdef QL_APP_FEATURE_LVGL
    // lvgl_app_init();
#endif

    // ql_sim_app_init();
    //  ql_power_app_init();

    ql_rtos_task_sleep_ms(1000); /*Chaos change: set to 1000 for the camera power on*/

    ql_rtos_task_delete(NULL);
}

extern void usb_acm_slip_init(void);
extern void t2n_task_init(void);
extern void wiegand_init(void);
extern void rs485_task_init(void);
extern void uart1_task_init(void);
extern void modbus_rtu_slave_task_init(void);
extern void ql_audio_app_init(void);
extern void ql_hd_gnss_app_init(void);
extern void ql_gnss_app_init(void);
extern void ql_rs485_app_init(void);
extern void ql_uart_app_init(void);
extern void arping_task(void const *arg);
extern void MX_FREERTOS_Init(void);

extern QlOSStatus ql_spi_demo_init(void);
// extern void quec_pin_cfg_init(void);
// void ql_pwm_audio_app_init(void);
// void ql_i2c_demo_init(void);
// void lis3dsh_task_init(void);
// void bl0939_task_init(void);
// void can_task_init(void);
// void bl0939_task(void);


int appimg_enter(void *param)
{
    QlOSStatus err = QL_OSI_SUCCESS;
    ql_task_t ql_init_task = NULL;

    ql_task_t _rtk_task = NULL;
    ql_task_t _t2n_task = NULL;
    // QL_INIT_LOG("init app enter: %s @ %s", QL_APP_VERSION, QL_APP_BUILD_RELEASE_TYPE);
    // senser_cfg =
    prvInvokeGlobalCtors();
    if (0 == strcasecmp(QL_APP_BUILD_RELEASE_TYPE, "release"))
    {
        ql_dev_cfg_wdt(1);
        // open the kernel log
        // ql_quec_trace_enable(1);
    }
    else
    {
        ql_dev_cfg_wdt(0);
        // close the kernel log
        // ql_quec_trace_enable(0);
    }
    
    /*Caution: GPIO pin must be initialized here, otherwise the pin status cannot be determined*/
    // ql_gpio_init_cfg();
    // quec_pin_cfg_init();
    //ql_mqtt_app_init();
    ql_pin_cfg_init();
    usb_acm_slip_init();
    err = ql_rtos_task_create(&ql_init_task, 1024 * 4, APP_PRIORITY_NORMAL, "ql_init", ql_init_app_thread, NULL, 1);
    if (err != QL_OSI_SUCCESS)
    {
        QL_INIT_LOG("init failed");
    }
    
    
    // wiegand_init();
    // rtk_gps_task_init();
    // ql_gnss_app_init();

    // t2n_task_init();
    // ql_spi_demo_init();
    // ql_uart_app_init();
    MX_FREERTOS_Init();
    // ql_rs485_app_init();
    // uart1_task_init();

    // ql_I2cInit(i2c_2, STANDARD_MODE);

    // ql_audio_app_init();
    // ql_hd_gnss_app_init();
    // text_iic();
    // ql_i2c_demo_init();
    // rs485_task_init();
    // lis3dsh_task_init();
    // esp32_flash_init();
    // QL_INIT_LOG("ENTER bl0919 task");
    // bl0939_task_init();
    // can_task_init();//can总线发送电流值
    // bl0939_task();
    // rs485_task_init();

    ql_gpio_set_level(GPIO_1, 0); // do0
    ql_gpio_set_level(GPIO_2, 1); // do1
    // ql_gpio_set_level(GPIO_15, 1); // do1
    // ql_gpio_set_level(GPIO_42, 0); // do1
    // ql_gpio_set_level(GPIO_43, 0); // do1
    // ql_gpio_set_level(GPIO_28, 1); // do2
    return err;
}

void appimg_exit(void)
{
    QL_INIT_LOG("init app exit");
}
