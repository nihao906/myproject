/**  @file
  I2C_demo.c

  @brief
  This file is demo of I2C.

*/

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

/*===========================================================================
 * include files
 ===========================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_camera.h"
#include "ql_i2c.h"
#include "es8311.h"

/*===========================================================================
 * Macro Definition
 ===========================================================================*/
typedef int    QlI2CStatus;
#define QL_APP_I2C_LOG_LEVEL             QL_LOG_LEVEL_INFO
#define QL_APP_I2C_LOG(msg, ...)         QL_LOG(QL_APP_I2C_LOG_LEVEL, "QL_APP_I2C", msg, ##__VA_ARGS__)
#define QL_APP_I2C_LOG_PUSH(msg, ...)    QL_LOG_PUSH("QL_APP_I2C", msg, ##__VA_ARGS__)
    
#define QL_I2C_TASK_STACK_SIZE     		1024
#define QL_I2C_TASK_PRIO          	 	APP_PRIORITY_NORMAL
#define QL_I2C_TASK_EVENT_CNT      		5

#define SalveAddr_w_8bit        (0x42 >> 1)
#define SalveAddr_r_8bit        (0x43 >> 1)

/*reserved*/
#define SalveAddr_w_16bit       (0xff >> 1)
#define SalveAddr_r_16bit       (0xff >> 1)

#define demo_for_8bit_or_16bit  (1)       //1:test 8bit register address     0:test 16bit register address
/*===========================================================================
 * Struct
 ===========================================================================*/

/*===========================================================================
 * Enum
 ===========================================================================*/

/*===========================================================================
 * Variate
 ===========================================================================*/
 
/*===========================================================================
 * Functions
 ===========================================================================*/
bool ES8311_I2C_write(const uint8_t reg, uint8_t value)
{
   ql_I2cWrite(i2c_2, 0x30, reg, &value, 1);
}
bool ES8311_I2C_read(const uint8_t reg, uint8_t *value)
{
   ql_I2cRead(i2c_2, 0x31, reg, value, 1);
}
void ql_i2c_demo_thread(void *param)
{
    /*test 8bit register address*/
    uint8_t read_data = 0;
    uint8_t data = 0xaa;

    // /*operate the camera for the example*/
    // ql_CamInit(320, 240);
    // ql_CamPowerOn();
    
    ql_I2cInit(i2c_2, STANDARD_MODE);
    ES8311_init(SAMPLING_8K);
    while(1)
    {
        QL_APP_I2C_LOG("ES8311_I2C");

        // ql_I2cRead(i2c_2, SalveAddr_r_8bit, 0xf0, &read_data, 1);
        // QL_APP_I2C_LOG("I2C read_data = 0x%x", read_data);
        // ql_I2cWrite(i2c_2, SalveAddr_w_8bit, 0x55, &data, 1);
        // read_data = 0;
        // ql_rtos_task_sleep_ms(200); 
    }
 
}

void ql_i2c_demo_init(void)
{
    QlI2CStatus err = QL_OSI_SUCCESS;
    ql_task_t i2c_task = NULL;
        
    err = ql_rtos_task_create(&i2c_task, QL_I2C_TASK_STACK_SIZE, QL_I2C_TASK_PRIO, "I2C DEMO", ql_i2c_demo_thread, NULL, QL_I2C_TASK_EVENT_CNT);
    if (err != QL_OSI_SUCCESS)
    {
        QL_APP_I2C_LOG("i2ctest1 demo task created failed");
    }
}


