#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "osi_api.h"
#include "ql_api_osi.h"
#include "ql_fs.h"
#include "ql_log.h"

#include "esp32_flash.h"
#include "ec600u_port.h"
#include "esp_loader.h"
#include "example_common.h"
#include "ec600u_port.h"
#include "http_download.h"
#include "udp_ota_shell.h"

#define QL_GPIODEMO_LOG_LEVEL   QL_LOG_LEVEL_INFO
#define LOGI(msg, ...)          QL_LOG(QL_GPIODEMO_LOG_LEVEL, "esp32_iap_main", msg, ##__VA_ARGS__)

#define HIGHER_BAUDRATE 230400

int esp32_flash_http_download_command(char *url, char *filepath)
{
    int ret = 0;


    LOGI("Download http file %s...", filepath);
    ret = http_dowload(url, filepath);
    if (ret) goto exit;

exit:
    return ret;
}

int esp32_flash_flash_command(void)
{
    int ret = 0;
    example_binaries_t bin;

    LOGI("flash init...");
    ret = loader_port_ec600u_init();
    if (ret) return;

    LOGI("conect to target...");
    ret = connect_to_target(HIGHER_BAUDRATE);
    if (ret) goto exit;

    get_example_binaries(esp_loader_get_target(), &bin);

    LOGI("Loading bootloader...");
    ret = flash_binary1(ESP32_FLASH_SDMMC_BOOTLOADER_FILENAME, bin.boot.addr, ESP32_FLASH_PHASE_FLASHING_BOOTLOADER);
    if (ret) goto exit;

    LOGI("Loading partition table...");
    ret = flash_binary1(ESP32_FLASH_SDMMC_PARTITION_TABLE_FILENAME, bin.part.addr, ESP32_FLASH_PHASE_FLASHING_PART_TABLE);
    if (ret) goto exit;

    LOGI("Loading app...");
    ret = flash_binary1(ESP32_FLASH_SDMMC_APP_FILENAME,  bin.app.addr, ESP32_FLASH_PHASE_FLASHING_APP);
    if (ret) goto exit;

    LOGI("Reset target!");
    loader_port_reset_target();

    // 成功烧录，写入配置文件
    LOGI("esp32 auto flash success! write nvm now...");
    esp32_ota_log(LINFO, "esp32 auto flash success! write nvm now...");
    ret = ql_nvm_fwrite("esp32_flash.config", "esp32 flash ok!", strlen("esp32 flash ok!") + 1, 1);  //nvm 配置文件不需要输入路径
	if(ret != strlen("esp32 flash ok!") + 1)
	{
		LOGI("failed! ret=0x%x", ret);
        esp32_ota_log(LINFO, "failed! ret=0x%x", ret);
		goto exit;
	}
    else 
    {
        LOGI("success!");
        esp32_ota_log(LINFO, "success!");
    }

exit:
    loader_port_ec600u_deinit();
    return ret;
}


int esp32_flash_reset_target_command(void)
{
    loader_port_ec600u_init_gpio();
    loader_port_reset_target();
    loader_port_ec600u_deinit_gpio();
    return 0;
}

extern osiThread_t *modbus_slave_thread;
void esp32_flash_task(void)
{
    ql_rtos_task_sleep_ms(5);   // 等待其他资源初始化完毕，等modbus_slave_thread
    int ret;
    int err;
    // 判断是否烧录过，没烧录过则自动从默认url下载烧录
    LOGI("check esp32 is need flash...");
    esp32_ota_log(LINFO, "check esp32 is need flash...");
    char buf[100] = {0};
	err = ql_nvm_fread("esp32_flash.config", buf, strlen("esp32 flash ok!") + 1, 1);
	if(err == strlen("esp32 flash ok!") + 1 && strncmp(buf, "esp32 flash ok!", strlen("esp32 flash ok!")) == 0)
	{
		LOGI("aleadly flash, skip auto flash!");
        esp32_ota_log(LINFO, "aleadly flash, skip auto flash!");
        goto exit;
	}	
    LOGI("begin auto flash...");
    esp32_ota_log(LINFO, "begin auto flash...");

    LOGI("Check data call...");
    ret = check_data_call();
    if (ret) goto exit;
    // 自动烧录
    err = esp32_flash_http_download_command(ESP32_FLASH_SDMMC_BOOTLOADER_URL_DEFAULT, ESP32_FLASH_SDMMC_BOOTLOADER_FILENAME);
    if (err) goto exit;
    err = esp32_flash_http_download_command(ESP32_FLASH_SDMMC_PARTITION_TABLE_URL_DEFAULT, ESP32_FLASH_SDMMC_PARTITION_TABLE_FILENAME);
    if (err) goto exit;
    err = esp32_flash_http_download_command(ESP32_FLASH_SDMMC_APP_URL_DEFAULT, ESP32_FLASH_SDMMC_APP_FILENAME);
    if (err) goto exit;

    if (modbus_slave_thread)    // 不能直接调用esp32_flash_flash_command，因为存在串口复用问题
    {
      osiEvent_t event;
      event.id = ESP32_FLASH_COMMAND_ESP32_FLASH;
      osiEventSend(modbus_slave_thread, &event);
    }

exit:
    ql_rtos_task_delete(NULL);
}


void esp32_flash_init(void)
{
    QlOSStatus err = 0;
    ql_task_t task_ref;
    err = ql_rtos_task_create(&task_ref, 4096, APP_PRIORITY_NORMAL, "esp32_flash_task", esp32_flash_task, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        LOGI("creat fft_task fail err = %d", err);
    }
    LOGI("creat esp32_flash_task sucess!");
}

