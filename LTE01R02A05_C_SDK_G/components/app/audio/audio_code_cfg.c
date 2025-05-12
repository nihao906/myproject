/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/

#include "ql_api_common.h"
#include "ql_audio.h"
#include "ql_codec_config.h"
#include "quec_customer_cfg.h"
#include "ql_i2c.h"
#include "ql_api_osi.h"
#include "codec_cfg.h"
/**********************************************************************************************
								        Struct
**********************************************************************************************/

typedef enum
{
    QL_EXT_CODEC_TYPE_NONE = 0,
    QL_EXT_CODEC_TYPE_ES8311,
    QL_EXT_CODEC_TYPE_ES8374,
	QL_EXT_CODEC_TYPE_MAX,
}ql_codec_type_e;

typedef struct
{
    ql_codec_reg_t *list;
    uint16_t list_size;
} ql_codec_reg_list_t;

typedef struct
{
    ql_codec_reg_list_t InitRegList;
    ql_codec_reg_list_t CloseRegList;
    ql_codec_reg_list_t PlayRegList;
    ql_codec_reg_list_t Samp8kRegList;
    ql_codec_reg_list_t Samp11kRegList;
    ql_codec_reg_list_t Samp16kRegList;
    ql_codec_reg_list_t Samp22kRegList;
    ql_codec_reg_list_t Samp32kRegList;
    ql_codec_reg_list_t Samp48kRegList;
    ql_codec_reg_list_t SampMclkList;
    ql_aud_adc_cfg  adc_cfg;
    ql_aud_dac_cfg  dac_cfg;
    uint8_t iic_addr; //从机地址
    ql_codec_type_e cur_type;
} ql_codec_cfg_t;

/*===========================================================================
 * Variables
 ===========================================================================*/	
/*
	ES8311寄存器组,用户可以对照ES8311手册在此修改寄存器
	g_es8311InitRegList: 初始化codec时会将此结构体中的配置发送给codec
	g_es8311PlayRegList: 开始播放时会将此结构体中的配置发送给codec
	g_es8311RecRegList:  开始录音时会将此结构体中的配置发送给codec
	g_es8311CloseRegList:关闭codec时会将此结构体中的配置发送给codec

	播放时的调用流程: g_es8311InitRegList --> g_es8311PlayRegList --> g_es8311CloseRegList
	录音时的调用流程: g_es8311InitRegList --> g_es8311RecRegList --> g_es8311CloseRegList
*/
static ql_codec_reg_t g_es8311InitRegList[] = ES8311_INIT_CONFIG;
static ql_codec_reg_t g_es8311CloseRegList[] = ES8311_CLOSE_CONFIG;
static ql_codec_reg_t g_es8311PlayRegList[] = ES8311_PLAY_CONFIG;
static ql_codec_reg_t g_es8311Samp8kRegList[] = ES8311_SAMP_8K_CONFIG;
static ql_codec_reg_t g_es8311Samp11kRegList[] = ES8311_SAMP_11K_CONFIG;
static ql_codec_reg_t g_es8311Samp16kRegList[] = ES8311_SAMP_16K_CONFIG;
static ql_codec_reg_t g_es8311Samp22kRegList[] = ES8311_SAMP_24K_CONFIG;
static ql_codec_reg_t g_es8311Samp32kRegList[] = ES8311_SAMP_32K_CONFIG;
static ql_codec_reg_t g_es8311Samp48kRegList[] = ES8311_SAMP_48K_CONFIG;
static ql_codec_reg_t g_es8311SampMclkList[] = ES8311_SAMP_BCLK;

ql_codec_cfg_t g_codec_cfg = 
{
    .InitRegList.list = g_es8311InitRegList,
    .InitRegList.list_size = sizeof(g_es8311InitRegList) / sizeof(ql_codec_reg_t),
    .CloseRegList.list = g_es8311CloseRegList,
    .CloseRegList.list_size = sizeof(g_es8311CloseRegList) / sizeof(ql_codec_reg_t),
    .PlayRegList.list = g_es8311PlayRegList,
    .PlayRegList.list_size = sizeof(g_es8311PlayRegList) / sizeof(ql_codec_reg_t),
    .Samp8kRegList.list = g_es8311Samp8kRegList,
    .Samp8kRegList.list_size = sizeof(g_es8311Samp8kRegList) / sizeof(ql_codec_reg_t),
    .Samp11kRegList.list = g_es8311Samp11kRegList,
    .Samp11kRegList.list_size = sizeof(g_es8311Samp11kRegList) / sizeof(ql_codec_reg_t),
    .Samp16kRegList.list = g_es8311Samp16kRegList,
    .Samp16kRegList.list_size = sizeof(g_es8311Samp16kRegList) / sizeof(ql_codec_reg_t),
    .Samp22kRegList.list = g_es8311Samp22kRegList,
    .Samp22kRegList.list_size = sizeof(g_es8311Samp22kRegList) / sizeof(ql_codec_reg_t),
    .Samp32kRegList.list = g_es8311Samp32kRegList,
    .Samp32kRegList.list_size = sizeof(g_es8311Samp32kRegList) / sizeof(ql_codec_reg_t),
    .Samp48kRegList.list = g_es8311Samp48kRegList,
    .Samp48kRegList.list_size = sizeof(g_es8311Samp48kRegList) / sizeof(ql_codec_reg_t),
    .SampMclkList.list = g_es8311SampMclkList,
    .SampMclkList.list_size = sizeof(g_es8311SampMclkList) / sizeof(ql_codec_reg_t),

    .adc_cfg = {QL_ADC_GAIN_LEVEL_12},
    .dac_cfg = {ES8311_DAC_DEFAULT},
    .iic_addr = ES8311_I2C_SLAVE_ADDR,
    .cur_type = QL_EXT_CODEC_TYPE_NONE,
};

/*===========================================================================
 * Extern Functions
 ===========================================================================*/
extern UINT8 ql_audio_iic_channel;
UINT8 ql_codec_clk_source = CLK_SOURCE_FROM_BCLK;
// static int quec_ext_codec_cfg_cb(ql_codec_cb_param_t *param);

/*===========================================================================
 * Functions
 ===========================================================================*/

ql_audio_errcode_e codec_read_reg(uint8 RegAddr, uint8 *p_value)
{
	uint8 retry_count = 5;

	while(retry_count--)
	{
        // QL_CODEC_LOG("codec read reg  0x%x 0x%x", i2c_2,g_codec_cfg.iic_addr);
		if(ql_I2cRead(i2c_2, g_codec_cfg.iic_addr, RegAddr, p_value, 1) == QL_I2C_SUCCESS)
		{
            QL_CODEC_LOG("codec read reg success 0x%x", RegAddr);
			return QL_AUDIO_SUCCESS;
		}
        ql_rtos_task_sleep_ms(100);
	}
	
	QL_CODEC_LOG("codec read reg err 0x%x", RegAddr);	 
	return QL_AUDIO_CODEC_RD_FAIL;
}

ql_audio_errcode_e codec_write_reg(uint8 RegAddr, uint8 RegData)
{
	uint8 retry_count = 5;

	while(retry_count--)
	{
		if(QL_I2C_SUCCESS == ql_I2cWrite(i2c_2, g_codec_cfg.iic_addr, RegAddr, &RegData, 1))
		{
			QL_CODEC_LOG("codec write 0x%x 0x%x",RegAddr, RegData);  
			return QL_AUDIO_SUCCESS;
		}
	}

	QL_CODEC_LOG("codec err 0x%x 0x%x",RegAddr, RegData);
	return QL_AUDIO_CODEC_WR_FAIL;
}

bool codec_write_list(ql_codec_reg_t *regList, uint16_t len)
{
	uint16_t regCount;

	if(regList == NULL)
	{
		return false;
	}

	for (regCount = 0; regCount < len; regCount++)
	{
		if(codec_write_reg(regList[regCount].addr, regList[regCount].data))
		{
            return false;
        }
        if(regList[regCount].delay)
        {
            ql_delay_us(1000 * regList[regCount].delay);
        }
	}
    return true;
}

//根据读到的VENDOR ID选择codec
bool codec_id_check(void)
{
    uint8 vendor_id = 0;

    for(uint8_t i = 0; i < 2; i++)
    {
        if(i == QL_EXT_CODEC_TYPE_ES8311)
        {
            g_codec_cfg.iic_addr = ES8311_I2C_SLAVE_ADDR;
            if(QL_AUDIO_SUCCESS == codec_read_reg(ES8311_VENDOR_ID_REG, &vendor_id))
            {
                QL_CODEC_LOG("codec id 0x%x 0x%x",ES8311_VENDOR_ID_REG, vendor_id);
                if(vendor_id == ES8311_VENDOR_ID)
                {
                    g_codec_cfg.cur_type = QL_EXT_CODEC_TYPE_ES8311;
                    g_codec_cfg.dac_cfg.dac_gain = ES8311_DAC_DEFAULT;

                    return true;
                }
            }
        }
        // else if(i == QL_EXT_CODEC_TYPE_ES8374)
        // {
        //     g_codec_cfg.iic_addr = ES8374_I2C_SLAVE_ADDR;
        //     //复位codec
        //     quec_ext_codec_write_list(g_es8374ResetRegList, sizeof(g_es8374ResetRegList) / sizeof(ql_codec_reg_t));
        //     if(QL_AUDIO_SUCCESS == ql_ext_codec_read_reg(ES8374_VENDOR_ID_REG, &vendor_id))
        //     {
        //         QL_EXT_CODEC_LOG("codec id 0x%x 0x%x",ES8374_VENDOR_ID_REG, vendor_id);
        //         if(vendor_id == ES8374_VENDOR_ID)
        //         {
        //             ql_codec_clk_source = CLK_SOURCE_FROM_MCLK;  //ES8374只能使用这种方式提供时钟
        //             g_codec_cfg.cur_type = QL_EXT_CODEC_TYPE_ES8374;
        //             g_codec_cfg.dac_cfg.dac_gain = ES8374_DAC_DEFAULT;
        //             g_codec_cfg.InitRegList.list = g_es8374InitRegList;
        //             g_codec_cfg.InitRegList.list_size = sizeof(g_es8374InitRegList) / sizeof(ql_codec_reg_t);
        //             g_codec_cfg.CloseRegList.list = g_es8374CloseRegList;
        //             g_codec_cfg.CloseRegList.list_size = sizeof(g_es8374CloseRegList) / sizeof(ql_codec_reg_t);
        //             g_codec_cfg.PlayRegList.list = g_es8374PlayRegList;
        //             g_codec_cfg.PlayRegList.list_size = sizeof(g_es8374PlayRegList) / sizeof(ql_codec_reg_t);
        //             g_codec_cfg.Samp8kRegList.list = g_es8374Samp8kRegList;
        //             g_codec_cfg.Samp8kRegList.list_size = sizeof(g_es8374Samp8kRegList) / sizeof(ql_codec_reg_t);
        //             g_codec_cfg.Samp11kRegList.list = g_es8374Samp11kRegList;
        //             g_codec_cfg.Samp11kRegList.list_size = sizeof(g_es8374Samp11kRegList) / sizeof(ql_codec_reg_t);
        //             g_codec_cfg.Samp16kRegList.list = g_es8374Samp16kRegList;
        //             g_codec_cfg.Samp16kRegList.list_size = sizeof(g_es8374Samp16kRegList) / sizeof(ql_codec_reg_t);
        //             g_codec_cfg.Samp22kRegList.list = g_es8374Samp22kRegList;
        //             g_codec_cfg.Samp22kRegList.list_size = sizeof(g_es8374Samp22kRegList) / sizeof(ql_codec_reg_t);
        //             g_codec_cfg.Samp32kRegList.list = g_es8374Samp32kRegList;
        //             g_codec_cfg.Samp32kRegList.list_size = sizeof(g_es8374Samp32kRegList) / sizeof(ql_codec_reg_t);
        //             g_codec_cfg.Samp48kRegList.list = g_es8374Samp48kRegList;
        //             g_codec_cfg.Samp48kRegList.list_size = sizeof(g_es8374Samp48kRegList) / sizeof(ql_codec_reg_t);
        //             g_codec_cfg.SampMclkList.list = NULL;
        //             g_codec_cfg.SampMclkList.list_size = 0;

        //             return true;
        //         }
        //     }
        // }
    }
// #ifndef CONFIG_QL_OPEN_EXPORT_PKG
// //因为内部测试夹具的原因，读ID失败后释放IIC，在使用时再打开此路IIC通道
//     if(ql_audio_iic_channel==i2c_2)
//     {
//         ql_I2cRelease(ql_audio_iic_channel);
//     }
// #endif
    return false;
}

//内核会调用此函数初始化ext codec
void codec_cb_init(void)
{
	ql_extcodec_info_t codec_cfg = {0};

	codec_cfg.extcodec_enable = true;
	codec_cfg.callback = codec_cfg_cb;
	codec_cfg.protocal = QL_DATA_PROROCOL_IIS;
	codec_cfg.data_bits = QL_AUD_DATA_BIT_32;
	codec_cfg.rx_delay = QL_AUD_RXDELAY_DEFAULT;
	codec_cfg.tx_delay = QL_AUD_RXDELAY_DEFAULT;
	ql_aud_ext_codec_cfg(&codec_cfg);

	//如果客户没有初始化iic,这里会获取不到codec id,在quec_ext_codec_cfg_cb中再次获取
	if(!codec_id_check())
	{
		QL_CODEC_LOG("check codec id err");
	}
}

//static
void codec_config_dac(bool mute_dac)
{
	if(mute_dac)
	{
		if(QL_EXT_CODEC_TYPE_ES8311 == g_codec_cfg.cur_type)
		{
			codec_write_reg(ES8311_REG_DAC, 0); //音量为0,将dac寄存器关闭做到彻底静音
		}
		else if(QL_EXT_CODEC_TYPE_ES8374 == g_codec_cfg.cur_type)
		{
			//方式1：配置REG36的bit5:DACMUTE为0 – normal,1 – mute DAC volume to -96dB
			//ql_ext_codec_write_reg(0x36, 0x24);
			//方式2：配置REG38:DACVOLUME大小为-96dB,相当于mute
			codec_write_reg(ES8374_REG_DAC, 0xC0); //音量为0,set volume mute
		}
	}
	else
	{
		//if(QL_EXT_CODEC_TYPE_ES8374 == g_codec_cfg.cur_type)
		//{
		//	方式1：配置REG36的bit5:DACMUTE为0 – normal,1 – mute DAC volume to -96dB
		//	ql_ext_codec_write_reg(0x36, 0x04); //解除静音
		//}
		set_dac_gain(&g_codec_cfg.dac_cfg);//音量不为0,根据ql_aud_get_adc_gain接口设置的增益来配置dac寄存器
	}
}

static ql_codec_reg_t *get_samp_reg_list(int sampreate, int *size)
{
	if(CLK_SOURCE_FROM_BCLK == ql_codec_clk_source)
	{
		*size = g_codec_cfg.SampMclkList.list_size;
		return g_codec_cfg.SampMclkList.list;		
	}
	else
	{
		switch(sampreate)
		{
			case 11025:
			case 12000:
				*size = g_codec_cfg.Samp11kRegList.list_size;
				return g_codec_cfg.Samp11kRegList.list;
			case 16000:
				*size = g_codec_cfg.Samp16kRegList.list_size;
				return g_codec_cfg.Samp16kRegList.list;
			case 24000:
			case 22050:
				*size = g_codec_cfg.Samp22kRegList.list_size;
				return g_codec_cfg.Samp22kRegList.list;
			case 32000:
				*size = g_codec_cfg.Samp32kRegList.list_size;
				return g_codec_cfg.Samp32kRegList.list;
			case 44100:
			case 48000:
				*size = g_codec_cfg.Samp48kRegList.list_size;
				return g_codec_cfg.Samp48kRegList.list;
			default:
			case 8000:
				*size = g_codec_cfg.Samp8kRegList.list_size;
				return g_codec_cfg.Samp8kRegList.list;
		}
	}
}

int codec_cfg_cb(ql_codec_cb_param_t *param)
{
    QL_CODEC_LOG("text 1");
	ql_codec_reg_t *samp_list = NULL;
	int size = 0;
	int ret=0;

	//如果在quec_ext_codec_cb_init的时候没有获取到codec类型,这里再次尝试获取codec类型
	if(QL_EXT_CODEC_TYPE_NONE == g_codec_cfg.cur_type)
	{
		if(!codec_id_check())
		{
			QL_CODEC_LOG("check codec id err");
            return -1;//读 codec id 出错直接 return 防止codec通讯异常频繁读写I2C致使看门狗超时DUMP
		}
	}

	QL_CODEC_LOG("codec_cb %d %d", param->stage, param->samprate);
	switch(param->stage)
	{
		case QL_EXT_CODEC_INIT:
            QL_CODEC_LOG("codec init success");
			samp_list = get_samp_reg_list(param->samprate, &size);
			if(!codec_write_list(g_codec_cfg.InitRegList.list, g_codec_cfg.InitRegList.list_size))
			{
                QL_CODEC_LOG("codec init fail");
                ret = -1;
                break;
            }
			codec_write_list(samp_list, size);
			break;

		case QL_EXT_CODEC_REC:
			set_adc_gain(&g_codec_cfg.adc_cfg);//配置增益
			break;

		//开始播放的同时,也同步设置增益
		case QL_EXT_CODEC_PLAY:
			codec_write_list(g_codec_cfg.PlayRegList.list, g_codec_cfg.PlayRegList.list_size);
		case QL_EXT_CODEC_SET_VOLUME:
			codec_config_dac(AUDIOHAL_SPK_MUTE == ql_get_volume() ? true : false);
			break;

		case QL_EXT_CODEC_DEINIT:
			codec_write_list(g_codec_cfg.CloseRegList.list, g_codec_cfg.CloseRegList.list_size);
			break;		
		default:
			break;		
	}

	return ret;
}

/*
	读取/配置ext codec的adc/dac增益
*/
ql_audio_errcode_e set_adc_gain(ql_aud_adc_cfg * adc_cfg)
{ 
    if((NULL == adc_cfg) || (adc_cfg->adc_gain < QL_ADC_GAIN_LEVEL_1) || (adc_cfg->adc_gain > QL_ADC_GAIN_LEVEL_12))
    {
    	return QL_AUDIO_INVALID_PARAM;
    }

    if(QL_EXT_CODEC_TYPE_ES8311 == g_codec_cfg.cur_type)
    {
        if(QL_ADC_GAIN_LEVEL_1 == adc_cfg->adc_gain)
        {
            if(codec_write_reg(ES8311_REG_ADC, 0x00))// reg 0x17 set volume mute
            {
            	return QL_AUDIO_CODEC_WR_FAIL;
            }
        }
        else
        {
            uint16_t REG14 = ((1 << 4) | adc_cfg->adc_gain);// 0-3 bit  adc pga gain value,the default value is +30dB Max
            if(codec_write_reg(ES8311_REG_SYSTEM,REG14))// 8311 change adc pga gain reg 0x14 
            {
            	return QL_AUDIO_CODEC_WR_FAIL;
            }
            if(codec_write_reg(ES8311_REG_ADC, 0xBF))//ADC_VOLUME0xBF:0dB,0.5dB/step
            {
            	return QL_AUDIO_CODEC_WR_FAIL;
            }
        }
    }
    
    g_codec_cfg.adc_cfg.adc_gain = adc_cfg->adc_gain;
    return QL_AUDIO_SUCCESS;
}

ql_audio_errcode_e get_adc_gain(ql_aud_adc_cfg * adc_cfg)
{
    if(NULL == adc_cfg)
    {
    	return QL_AUDIO_INVALID_PARAM;
    }	
    adc_cfg->adc_gain =  g_codec_cfg.adc_cfg.adc_gain;
    return QL_AUDIO_SUCCESS;
}

ql_audio_errcode_e set_dac_gain(ql_aud_dac_cfg * dac_cfg)
{
    if(QL_EXT_CODEC_TYPE_ES8311 == g_codec_cfg.cur_type)
    {
        if(codec_write_reg(ES8311_REG_DAC,dac_cfg->dac_gain))// 8311 change dac volume reg 0x32 
        {
            return QL_AUDIO_CODEC_WR_FAIL;
        }
    }

    g_codec_cfg.dac_cfg.dac_gain = dac_cfg->dac_gain;
    return QL_AUDIO_SUCCESS;
}

ql_audio_errcode_e get_dac_gain(ql_aud_dac_cfg * dac_cfg)
{
    if(dac_cfg == NULL)
    {
    	return QL_AUDIO_INVALID_PARAM;
    }
    dac_cfg->dac_gain =  g_codec_cfg.dac_cfg.dac_gain;
    return QL_AUDIO_SUCCESS;
}

