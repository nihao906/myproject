
/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
    
#ifndef _CODEC_CFG_H_
#define _CODEC_CFG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ql_log.h"
#include "ql_api_common.h"

/**********************************************************************************************
								        Define
**********************************************************************************************/
#define QL_CODEC_LOG_LEVEL			  QL_LOG_LEVEL_INFO
#define QL_CODEC_LOG(msg, ...)		  QL_LOG_TAG(QL_EXT_CODEC_LOG_LEVEL, QL_LOG_TAG_CODEC, "ql_codec", msg, ##__VA_ARGS__)

//CODEC ES8311
#define ES8311_I2C_SLAVE_ADDR      	0x18
#define ES8311_VENDOR_ID	       	0x83
#define ES8311_VENDOR_ID_REG       	0XFD
#define ES8311_REG_SYSTEM			0x14
#define	ES8311_REG_ADC				0x17
#define	ES8311_REG_DAC				0x32
#define ES8311_DAC_DEFAULT			0xBF //0xBF:0dB,0xDF:+24dB,0.5dB/step,NOTE:大于0xCC时,音量调最大时会破音

#define ES8311_INIT_CONFIG	\
{							\
	{0x0D, 0x01},			\
	{0x45, 0x00},			\
	{0x01, 0x3F},			\
	{0x00, 0x80},			\
	{0x02, 0x00},			\
	{0x37, 0x08},			\
	{0x15, 0x10},			\
	{0x14, 0x10},			\
	{0x0E, 0x00},			\
	{0x32, 0xBF},			\
	{0x17, 0xBF},			\
    {0x09, 0x10},			\
    {0x0A, 0x10},			\
    {0x0B, 0x00},			\
    {0x0C, 0x00},			\
    {0x10, 0x03},			\
    {0x11, 0x7f},			\
	{0x13, 0x00},			\
	{0x0F, 0x44},			\
	{0x1B, 0x0A},			\
	{0x1C, 0x6A},			\
	{0x44, 0x00},			\
	{0x03, 0x10},			\
    {0x16, 0x24},			\
    {0x04, 0x20},			\
    {0x05, 0x00},			\
}

#define ES8311_CLOSE_CONFIG	\
{							\
	{0x32, 0x00},			\
	{0x17, 0x00},			\
	{0x0E, 0xFF},			\
	{0x12, 0x02},			\
	{0x14, 0x00},			\
	{0x0D, 0xFA},			\
	{0x15, 0x00},			\
	{0x37, 0x08},			\
	{0x02, 0x10},			\
	{0x00, 0x00},			\
	{0x00, 0x1F},			\
	{0x01, 0x30},			\
	{0x01, 0x00},			\
	{0x45, 0x00},			\
}

#define ES8311_PLAY_CONFIG	\
{							\
	{0x12, 0x00},			\
}

#define ES8311_SAMP_8K_CONFIG	\
{								\
    {0x02, 0xA0},				\
    {0x06, 0x15},				\
    {0x07, 0x05},				\
    {0x08, 0xFF},				\
}

#define ES8311_SAMP_11K_CONFIG	\
{								\
    {0x02, 0x60},				\
    {0x06, 0x0f},				\
    {0x07, 0x03},				\
    {0x08, 0xff},				\
}

#define ES8311_SAMP_16K_CONFIG	\
{								\
    {0x02, 0x40},				\
    {0x06, 0x43},				\
    {0x07, 0x02},				\
    {0x08, 0xFF},				\
}

#define ES8311_SAMP_24K_CONFIG	\
{								\
    {0x02, 0x20},				\
    {0x06, 0x07},				\
    {0x07, 0x01},				\
    {0x08, 0xff},				\
}

#define ES8311_SAMP_32K_CONFIG	\
{								\
    {0x02, 0x48},				\
    {0x06, 0x05},				\
    {0x07, 0x01},				\
    {0x08, 0x7f},				\
}

#define ES8311_SAMP_48K_CONFIG	\
{								\
	{0x31, 0x60},				\
	{0x37, 0x08},				\
	{0x38, 0x00},				\
	{0x39, 0xF7},				\
	{0x3A, 0xFD},				\
	{0x3B, 0xFF},				\
	{0x3C, 0x20},				\
	{0x3D, 0x4B},				\
	{0x3E, 0xE1},				\
	{0x3F, 0x5B},				\
	{0x40, 0x01},				\
	{0x41, 0x33},				\
	{0x42, 0x1F},				\
	{0x43, 0x4F},				\
	{0x37, 0x00},				\
	{0x31, 0x00},				\
    {0x02, 0x00},				\
    {0x06, 0x03},				\
    {0x07, 0x00},				\
    {0x08, 0xff},				\
}

#define ES8311_SAMP_BCLK		\
{								\
	{0x01, 0xBF},				\
    {0x02, 0x10},				\
    {0x03, 0x10},				\
    {0x16, 0x20},				\
    {0x04, 0x20},				\
    {0x05, 0x00},				\
    {0x06, 0x03},				\
    {0x07, 0x00},				\
    {0x08, 0x3f},				\
}




int codec_cfg_cb(ql_codec_cb_param_t *param);
/**********************************************************************************************
								        Struct
**********************************************************************************************/
// typedef struct
// {
//     uint8_t addr;
//     uint8_t data;
//     uint16_t delay; //The delay times after the register operation is performed.Unit: ms
// } ql_codec_reg_t;


#ifdef __cplusplus
} /*"C" */
#endif

#endif

