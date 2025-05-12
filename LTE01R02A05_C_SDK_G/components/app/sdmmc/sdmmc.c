#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ql_log.h"
#include "osi_api.h"
#include "ql_sdmmc.h"
#include "ql_api_osi.h"
#include "ql_fs.h"
#include "ql_gpio.h"
#include "sdmmc.h"


#define SDMMC_CMD_PIN   0
#define SDMMC_D0_PIN    0
#define SDMMC_D1_PIN    0
#define SDMMC_D2_PIN    0
#define SDMMC_D3_PIN    0
// sdmmc clk 是pin134的固定功能，不用配置

#define SDMMC_CMD_FUN   0
#define SDMMC_D0_FUN    0
#define SDMMC_D1_FUN    0
#define SDMMC_D2_FUN    0
#define SDMMC_D3_FUN    0

#define QL_FM_FAT32                     0x02

#define QL_GPIODEMO_LOG_LEVEL       QL_LOG_LEVEL_INFO
#define LOGI(msg, ...)              QL_LOG(QL_GPIODEMO_LOG_LEVEL, "sdmmc", msg, ##__VA_ARGS__)


static void sdmmc_pin_init(void)
{
	ql_pin_set_func(SDMMC_CMD_PIN, SDMMC_CMD_FUN);    
	ql_pin_set_func(SDMMC_D0_PIN , SDMMC_D0_FUN);     
	ql_pin_set_func(SDMMC_D1_PIN , SDMMC_D1_FUN);    
	ql_pin_set_func(SDMMC_D2_PIN , SDMMC_D2_FUN);     
	ql_pin_set_func(SDMMC_D3_PIN , SDMMC_D3_FUN);     

	ql_sdmmc_cfg_t cfg = {
        .dev = QL_SDMMC_SD_CARD_ONLY,  //只以sd卡方式进行初始化
        .sd_mv = 0,     //SD卡默认电压域3.2v
        .emmc_mv = 0,   //emmc默认电压域1.8v
	};
    ql_sdmmc_set_dev_cfg(cfg);
    ql_sdmmc_set_clk(1000000);
}


/* 将文件写入sdmmc */
int sdmmc_write_bin_file(char *file_path, char *sdmmc_path)
{
    FILE *fp;
    int fd = 0;
    unsigned char buf[1024];
    int buf_len = 1024;
    int read_len = 0, write_len = 0;

    fp = fopen(file_path, "rb");
    if (fp == NULL)
    {
        LOGI("%s file open failed!", file_path);
        return  -1;
    }

    fd = ql_fopen(sdmmc_path, "wb+");
    if(fd < 0)
    {
    	LOGI("%s file open failed!", sdmmc_path);
        return -1;
    }

    do 
    {
        read_len = fread(buf, 1, buf_len, fp);
        write_len = ql_fwrite(buf, 1, read_len, fd);  
        if (write_len != read_len)
        {
            LOGI("write(%d/%d) loss", write_len, read_len);
            return -2;
        }
    } while (read_len >= buf_len);
    
    fclose(fp);
    ql_fclose(fd);
    return 0;
}


int sdmmc_get_file_size(char *file_path)
{
    int fd = 0;
    int file_size = 0;

    fd = ql_fopen(file_path, "rb");
    if(fd < 0)
    {
    	LOGI("%s file open failed:0x%x!", file_path, (unsigned int)fd);
        return -1;
    }
    file_size = ql_fseek(fd, 0, QL_SEEK_END);
    ql_fclose(fd);
    return file_size;
}

int sdmmc_read_bin_file(char *file_path)
{
    int fd = 0;
    uint32_t read_len = 0;
    uint8_t buf[5];
    uint32_t buf_len = 5;

    fd = ql_fopen(file_path, "rb");
    if(fd < 0)
    {
    	LOGI("%s file open failed:0x%x!", file_path, (unsigned int)fd);
        return -1;
    }

    read_len = ql_fread(buf, 1, buf_len, fd);
    LOGI("read %u bytes:", read_len);
    int i = 0;
    for (i = 0; i < read_len && i < buf_len; i++)
        LOGI("0x%x", buf[i]);

    ql_fclose(fd);
    return 0;
}

void get_dirname(char *filename, char *dirname)
{
    memcpy(dirname, filename, strlen(filename));
    char *p = strrchr(dirname, '/');
    if (p) *p = '\0';
    else memcpy(dirname, "SD:/", sizeof("SD:/"));   
}


// 传入文件名，函数检查目录是否存在，不存在则创建
void sdmmc_create_file_dir(char *filename)
{
    char dirname[100];
    get_dirname(filename, dirname);
    QDIR *dirp = ql_opendir(dirname);
	if(dirp == NULL)
    {
		LOGI("create dir %s", dirname);
        ql_mkdir(dirname, 0);
    }
    else
        ql_closedir(dirp);
}

void waiting_sdmmc_mount(void)
{
    while (ql_sdmmc_is_mount() == 0)
    {
        LOGI("waiting sdmmc mount");
        ql_rtos_task_sleep_s(2);
    } 
}



int sdmmc_init(void)
{
	QlOSStatus err = QL_SDMMC_SUCCESS;

	sdmmc_pin_init();
    
    if(QL_SDMMC_SUCCESS != ql_sdmmc_mount())
	{
		LOGI("Mount failed");
		return -1;
	}
	else LOGI("Mount succeed");

    // /* 格式化为一个分区 */
    // if(QL_SDMMC_SUCCESS != ql_sdmmc_mkfs(QL_FM_FAT32))
    // {
    //     LOGI("mkfs failed");
    //     return -1;
    // }
    // else LOGI("mkfs succeed");
    
    return 0;
}