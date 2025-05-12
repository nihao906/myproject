/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#ifndef __AT_CMD_NW_H__
#define __AT_CMD_NW_H__

#ifdef __cplusplus
extern "C" {
#endif
#define AT_UHSGL_CMD 1
#define AT_SWITCH_DEFAULT_SIM (0)
//switch network mode
#define AT_NW_LTE_MODE 4
#define AT_NW_GSM_MODE 2

//detach network type
#define AT_STOP_TYPE_NORMAL 0
#define AT_STACK_CLOSE_SIM 1
#define AT_STACK_LOCAL_DETACH 8

//boot mode
#define AT_NST_MODE 0x0F
#define AT_UNNORMAL_MODE 0x0E //start boot mode isn't normal mode

//0x4453 is executed switch
#define AT_GSM_LTE_SWITCH_VALUE (0x4453)

//NASTIMER command
typedef enum
{
    AT_T3302,         //     gsm 12min, 5-30min
    AT_T3311,         //     gsm 15s,  5~60s
    AT_T3211,         //     gsm 15s,  5~60s
    AT_T3240,         //     gsm 10s,  5~60s
    AT_T3380,         //     gsm 30s,  5~60s
    AT_T3390,         //     gsm 30s,  5~60s
    AT_GSM_TIMER_MAX, // 6
    AT_T3402,         //          EMM 12min, 0~65535s
    AT_T3417_EXT,     //          EMM 10s, 0~65535s
    AT_T3410,         //          EMM 15s, 0~65535s
    AT_T3411,         //          EMM 10s, 0~65535s
    AT_T3421,         //          EMM 15s, 0~65535s
    AT_T3440,         //          EMM 10s, 0~65535s
    AT_T3412,         //          EMM 54min, 0~65535s
    AT_T3430,         //          EMM 15s, 0~65535s
    AT_T3346,         //          EMM N/As, 0~65535s
    AT_EMM_TIMER_MAX, // 16
    AT_T3480,         //          ESM 8s, 0~65535s
    AT_T3481,         //          ESM 8s, 0~65535s
    AT_T3482,         //          ESM 8s, 0~65535s
    AT_T3492,         //          ESM 6s, 0~65535s
} AT_NAS_TIMER;
typedef enum
{
    AT_GSM_TYPE,
    AT_EMM_TYPE,
    AT_ESM_TYPE,
} AT_NAS_TYPE;

/************************************
**Obtain the network mode loaded by the
**current modem
************************************/
uint8_t atGetGsmOrLteMode(uint8_t nSim);
void atSwitchGsmLteCpFix(void);
void atSendEventFixGsmLteSwitch(void);

void atResetJammingStatus(uint8_t nSim);
// uint8_t atGetFlightModeFlag(CFW_SIM_ID nSimID);

#ifdef CONFIG_NBIOT_RPM_SUPPORT
uint8_t sdb_nvGetDefaultPdnSettingApn(uint8_t *outputApn);
bool nbiot_RpmAllowRebooting(uint8_t trb);
bool sdb_rpmAllowPdpActivating(uint8_t *apn, uint32_t apnLen, uint8_t trb);
#endif

#ifdef __cplusplus
}
#endif
#endif
