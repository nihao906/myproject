/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_usb.h"
#include "usb_demo.h"

#define QL_USB_LOG_LEVEL	            QL_LOG_LEVEL_INFO
#define QL_USB_LOG(msg, ...)			QL_LOG(QL_USB_LOG_LEVEL, "ql_usb", msg, ##__VA_ARGS__)
#define QL_USB_LOG_PUSH(msg, ...)		QL_LOG_PUSH("ql_usb", msg, ##__VA_ARGS__)

/*
模块USB 只能在 HOST 和 DEVICE中选一个，因此：
    QL_USB_DEVICE_MASS_STORAGE 和 QL_USB_HOST_MASS_STORAGE 不可同时开启
*/
#define QL_USB_DEVICE_MASS_STORAGE	0 //验证usb device 模式下的mass storage功能打开此宏
#define QL_USB_HOST_MASS_STORAGE    0 //验证usb host模式下的 U盘功能打开此宏
#define QL_USB_DEVICE_CCID_BLOCK_MSG_TEST     0 //验证USB CCID BLOCK msg

#if QL_USB_HOST_MASS_STORAGE
#include "ql_fs.h"
#include "ql_power.h"
#endif

#if QL_USB_DEVICE_CCID_BLOCK_MSG_TEST
static int ql_usb_ccid_user_recv_data_cb(void *buff,uint16_t len)
{
    uint8 read_buff[512];
    uint8 write_buff[2] = {0x90,0x00};
    QL_USB_LOG("recive data len:%d",len);
    ql_usb_ccid_read(read_buff,len);
    ql_usb_ccid_write(write_buff,2);
    return 0;
}

static uint8_t ql_usb_ccid_user_poweron(void)
{
    QL_USB_LOG("power on cb");
    return 0;
}

static uint8_t ql_usb_ccid_user_poweroff(void)
{
    QL_USB_LOG("power off cb");
    return 0;
}

#endif

static void ql_usb_demo_thread(void *param)
{
	QL_USB_LOG("enter usb demo");

#if QL_USB_DEVICE_MASS_STORAGE
	ql_usb_msc_cfg_t msc_cfg = {0};
/*
    模块USB 作为device 在PC虚拟出U盘
	配置映射模块的内置flash/sd卡/6线flash等存储器到PC端, 模块作为虚拟U盘
	注意：
		  1. 在target.config中,打开CONFIG_QUEC_PROJECT_FEATURE_USB_MASS_STORAGE宏
		  2. 烧录代码后第一次开机进app可能会比较慢,如果sd卡/外置flash在app挂载,则烧录后第一次开机可能虚拟不出sd卡/外置flash,后续开机不会无法映射
*/
	msc_cfg.msc_device = QL_USB_MSC_SDCARD; //如需映射2个存储器,可用"|"连接,如同时映射外置6线flash和sd卡: QL_USB_MSC_EFS | QL_USB_MSC_SDCARD

/*
	1. MTP协议和MSC协议的差异见ql_usb_protocol_e定义处
	2. 注意MSC协议下PC枚举出虚拟U盘后,模组端将无法访问SD卡,SD卡的读写权限全部交给PC
*/
	msc_cfg.protocol = QL_USB_PROTOCOL_MTP;

/*
	MTP协议下, 已经加载过一次虚拟U盘的PC,如果要修改msc_cfg.dev_name,可能需要卸载重装一次QUECTEL的USB驱动,因为部分windows系统会记住第一次识别到的
	设备名称,只有卸载重装一次USB驱动才会去刷新虚拟U盘设备名称, MSC协议下无此问题
*/
	strcpy(msc_cfg.dev_name, "ANDROID");

	//msc_cfg.option.hide_option = QL_USB_MSC_HIDE_PART2; //MSC协议下,SD卡有2个分区时,选择是否隐藏一个分区,只映射一个分区到PC上
	//msc_cfg.option.protect_dsiable = false; //MSC协议下,是否取消SD卡保护(PC枚举出虚拟U盘后,模组端将无法访问SD卡)

	ql_usb_set_enum_mode(QL_USB_ENUM_MASS_STORAGE); //重启生效
	ql_usb_msc_config_set(&msc_cfg); //重启生效

/*
	//若需要立即生效,可使能此处代码,调用后模块的usb会先断开连接然后重新连接
	//ql_usb_disable();
	//ql_rtos_task_sleep_s(1);
	//ql_usb_enable();
*/
#endif

#if QL_USB_DEVICE_CCID_BLOCK_MSG_TEST

    ql_usb_set_enum_mode(QL_USB_ENUM_USBNET_COM_CCID);

    ql_usb_ccid_usr_cb_t cb = {0};
    cb.usrccidXfrBlockCb = ql_usb_ccid_user_recv_data_cb;
    cb.usrccidGetSlotStatusCb = NULL;
    cb.usrccidIccPowerOffCb = ql_usb_ccid_user_poweroff;
    cb.usrCcidIccPowerOnCb = ql_usb_ccid_user_poweron;
    ql_usb_ccid_usr_cb_register(cb);
    ql_usb_disable();
	ql_rtos_task_sleep_s(1);
	ql_usb_enable();

#endif

#if QL_USB_HOST_MASS_STORAGE
/*
    模块作为USB HOST,挂载外置U盘,并读写U盘文件
*/
    int fd = 0 ,err=0;
    char * str = "123456789abcdefghijklmnABCDEFGHIJKLMN";
    char buff[128]={0};
    if(ql_usb_get_enum_mode() != QL_USB_ENUM_HOST_MASS_STORAGE)
    {
        ql_usb_set_enum_mode(QL_USB_ENUM_HOST_MASS_STORAGE);  //设置HOST 模式 重启生效
        QL_USB_LOG("set host mode,rebooting");
        ql_rtos_task_sleep_s(1);
        ql_power_reset(RESET_NORMAL);
    }
    while(1)  //当前只能不能自动挂载，后续支持，demo 在此等待U盘插入，并挂载成功
    {
        ql_rtos_task_sleep_s(3);
        QL_USB_LOG("waiting udisk...");
        if(ql_usb_host_udisk_mount() == QL_USB_SUCCESS)
        {
            QL_USB_LOG("mount udisk success");
            break;
        }
    }
    
    //"UDISK:" for udisk 
    fd = ql_fopen("UDISK:test.txt", "w+");
    if(fd < 0)
    {
        QL_USB_LOG("open fail");
        goto exit;
    }
	err = ql_fwrite(str, strlen(str) + 1, 1, fd);   //strlen not include '\0'
	if(err < 0)
	{
		QL_USB_LOG("write fail");
		goto exit;		
	}
    ql_fseek(fd, 0, 0);
    err = ql_fread(buff, ql_fsize(fd), 1, fd);
    if(err<0)
    {
        QL_USB_LOG("read fail");
		goto exit;
    }
    QL_USB_LOG("read:%s",buff);
    
exit:
    ql_fclose(fd);
#endif

/*
	使用默认的端口组合,配置枚举哪些USB端口,也可以直接在quec_usb_serial_create中配置,该函数开源在app_start.c中
	ql_usb_set_enum_mode(QL_USB_ENUM_USBNET_COM);
*/
	ql_rtos_task_delete(NULL);
}

void ql_usb_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;
	ql_task_t ql_usb_task = NULL;
	
    QL_USB_LOG("usb demo enter");
	
    err = ql_rtos_task_create(&ql_usb_task, QL_USB_TASK_STACK, APP_PRIORITY_NORMAL, "ql_usb_demo", ql_usb_demo_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		QL_USB_LOG("usb task create failed");
	}
}


