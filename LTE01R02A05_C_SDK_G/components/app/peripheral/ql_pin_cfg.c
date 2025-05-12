/**  
  @file
  ql_pin_cfg.c

  @brief
  quectel pin cfg.

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
15/03/2021        Neo         Init version
=================================================================*/



/*===========================================================================
 * include files
 ===========================================================================*/
#include "ql_pin_cfg.h"

/*===========================================================================
 * GPIO Map
 ===========================================================================*/
const ql_pin_cfg_t ql_pin_cfg_map[] = /* pin initialize */
{            /* pin_num  default_func     gpio_func    gpio_num       gpio_dir         gpio_pull          gpio_lvl  */
/*PCM_CLK    */{  27 ,        1,              0,        GPIO_0,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*LCD_SIO    */{  125,        0,              1,        GPIO_0,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*PCM_SYNC   */{  26 ,        1,              0,        GPIO_1,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*LCD_SDC    */{  124,        0,              1,        GPIO_1,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*PCM_IN     */{  24 ,        1,              0,        GPIO_2,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*LCD_CLK    */{  123,        0,              1,        GPIO_2,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*PCM_OUT    */{  25 ,        1,              0,        GPIO_3,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*LCD_CS     */{  122,        0,              1,        GPIO_3,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*USIM_DET   */{  13 ,        1,              0,        GPIO_4,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*LCD_SEL    */{  121,        0,              1,        GPIO_4,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*WLAN_WAKE  */{  135,        1,              0,        GPIO_5,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*LCD_FMARK  */{  119,        0,              1,        GPIO_5,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*SDIO2_CLK  */{  133,        1,              0,        GPIO_7,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },// pay attention to USB
/*SLEEP_IND  */{  3  ,        0,              0,        GPIO_8,    GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*SDIO2_CMD  */{  134,        1,              5,        GPIO_8,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*SPI_1_CLK  */{  40 ,        0,              0,        GPIO_9,    GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*SDIO2_DATA0*/{  132,        1,              5,        GPIO_9,    QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*SPI_1_CS   */{  37 ,        0,              0,        GPIO_10,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*SDIO2_DATA1*/{  131,        1,              5,        GPIO_10,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*SPI_1_MOSI */{  38 ,        0,              0,        GPIO_11,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*SDIO2_DATA2*/{  130,        1,              5,        GPIO_11,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*SPI_1_MISO */{  39 ,        0,              0,        GPIO_12,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*SDIO2_DATA3*/{  129,        1,              5,        GPIO_12,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*NET_STATUS */{  5  ,        0,              0,        GPIO_13,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },// hardware pin name is NET_MODE
/*I2C2_SCL   */{  141,        1,              0,        GPIO_14,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*I2C2_SDA   */{  142,        1,              0,        GPIO_15,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*I2C_SCL    */{  41 ,        0,              4,        GPIO_16,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },// output only
/*I2C_SDA    */{  42 ,        0,              4,        GPIO_17,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*MAIN_RTS   */{  65 ,        1,              0,        GPIO_18,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*STATUS     */{  61 ,        0,              4,        GPIO_18,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*MAIN_CTS   */{  64 ,        1,              0,        GPIO_19,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*MAIN_RI    */{  62 ,        0,              4,        GPIO_19,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*WLAN_EN    */{  136,        0,              0,        GPIO_20,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*MAIN_DCD   */{  63 ,        0,              4,        GPIO_20,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*BT_EN      */{  139,        0,              0,        GPIO_21,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*MAIN_DTR   */{  66 ,        0,              4,        GPIO_21,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*GPIO1      */{  126,        0,              0,        GPIO_22,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*NET_MODE   */{  6  ,        0,              4,        GPIO_22,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },// hardware pin name is NET_STATUS
/*WLAN_PWR_EN*/{  127,        0,              0,        GPIO_23,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*SD_DET     */{  23 ,        0,              4,        GPIO_23,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*SDIO1_CMD  */{  33 ,        0,              1,        GPIO_24,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*SDIO1_DATA0*/{  31 ,        0,              1,        GPIO_25,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*SDIO1_DATA1*/{  30 ,        0,              1,        GPIO_26,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*SDIO1_DATA2*/{  29 ,        0,              1,        GPIO_27,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*SDIO1_DATA3*/{  28 ,        0,              1,        GPIO_28,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
#if defined(QL_APP_FEATURE_DSIM) || defined(QL_APP_FEATURE_DSSS) //resolved for CLK, DATA and RST pins of SIM2 in double sim scenario
/*WAKEUP_IN  */{  1  ,        0,              2,        GPIO_29,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*AP_READY   */{  2  ,        0,              2,        GPIO_30,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*W_DISABLE  */{  4  ,        0,              2,        GPIO_31,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
#else
/*WAKEUP_IN  */{  1  ,        2,              2,        GPIO_29,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*AP_READY   */{  2  ,        2,              2,        GPIO_30,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
/*W_DISABLE  */{  4  ,        2,              2,        GPIO_31,   GPIO_INPUT,       PULL_DOWN,        QUEC_PIN_NONE },
#endif // defined(QL_APP_FEATURE_DSIM) || defined(QL_APP_FEATURE_DSSS)
//pinmux(not gpio part)
/*SDIO1_CLK  */{  32 ,        0,  QUEC_PIN_NONE,  QUEC_PIN_NONE,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*KEYOUT4    */{  81 ,        4,  QUEC_PIN_NONE,  QUEC_PIN_NONE,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*KEYOUT5    */{  82 ,        4,  QUEC_PIN_NONE,  QUEC_PIN_NONE,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*AUX_RXD    */{  137,        3,  QUEC_PIN_NONE,  QUEC_PIN_NONE,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },
/*AUX_TXD    */{  138,        3,  QUEC_PIN_NONE,  QUEC_PIN_NONE,   QUEC_PIN_NONE,    QUEC_PIN_NONE,    QUEC_PIN_NONE },

//             should before here!
               {QUEC_PIN_NONE,-1,            -1,             -1,              -1,               -1,                -1},
};
