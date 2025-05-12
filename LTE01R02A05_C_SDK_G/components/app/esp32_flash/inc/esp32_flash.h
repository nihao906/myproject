#ifndef _ESP32_FLASH_H
#define _ESP32_FLASH_H

// #include "ModbusS.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "udp_ota_shell.h"

#define ESP32_FLASH_SDMMC_BOOTLOADER_URL_DEFAULT        "http://git.mcxa.cn:8100/rtk_led_v3/esp32s3/bootloader.bin"
#define ESP32_FLASH_SDMMC_PARTITION_TABLE_URL_DEFAULT   "http://git.mcxa.cn:8100/rtk_led_v3/esp32s3/partition-table.bin" 
#define ESP32_FLASH_SDMMC_APP_URL_DEFAULT               "http://git.mcxa.cn:8100/rtk_led_v3/esp32s3/lcd_lvgl.bin" 
#define ESP32_FLASH_SDMMC_BOOTLOADER_FILENAME           "SD:/esp32_flash/bootloader.bin"
#define ESP32_FLASH_SDMMC_PARTITION_TABLE_FILENAME      "SD:/esp32_flash/partition-table.bin"
#define ESP32_FLASH_SDMMC_APP_FILENAME                  "SD:/esp32_flash/app.bin"


typedef enum {
    ESP32_FLASH_COMMAND_NONE        = 0,        // none表示未设置
    ESP32_FLASH_COMMAND_HTTP_DOWNLOAD = 0x3500,
    ESP32_FLASH_COMMAND_ESP32_FLASH,            
    ESP32_FLASH_COMMAND_ESP32_RESET                
} esp32_flash_command_t;


typedef enum {
    ESP32_FLASH_PHASE_NONE = 0,
    ESP32_FLASH_PHASE_INIT,
    ESP32_FLASH_PHASE_WAITING_NET,
    ESP32_FLASH_PHASE_DOWNLOADING,
    ESP32_FLASH_PHASE_FLASH_INIT,
    ESP32_FLASH_PHASE_CONNECT_TARGET,
    ESP32_FLASH_PHASE_FLASHING_BOOTLOADER,
    ESP32_FLASH_PHASE_FLASHING_PART_TABLE,
    ESP32_FLASH_PHASE_FLASHING_APP,
    ESP32_FLASH_PHASE_RESET_TARGET
} esp32_flash_phase_t;

typedef enum {
    ESP32_FLASH_STATUS_NONE     = 0,
    ESP32_FLASH_STATUS_DOING,
    ESP32_FLASH_STATUS_FINISH,
    ESP32_FLASH_STATUS_ERR
} esp32_flash_status_t;

// static inline bool esp32_flash_command_is_valid(void)
// {
//     return gWordVar[REG_ESP32_FLASH_COMMAND] >= ESP32_FLASH_COMMAND_AUTO_FLASH && gWordVar[REG_ESP32_FLASH_COMMAND] <= ESP32_FLASH_COMMAND_ESP32_RESET;
// }

// static inline esp32_flash_command_t esp32_flash_get_command(void)
// {
//     return (esp32_flash_command_t)gWordVar[REG_ESP32_FLASH_COMMAND];
// }

// static inline void esp32_flash_set_command(esp32_flash_command_t command)
// {
//     gWordVar[REG_ESP32_FLASH_COMMAND] = command;
// }


static char phase_str[10][25] = {  "none", "init", 
                            "waiting net", "http downloading", 
                            "flash init", "connect target", 
                            "flashing bootloader", "flashing partition table", "flashing app",
                            "reset target"};

static char status_str[5][10] = {"none", "doing", "finish", "error"};


static inline void esp32_flash_set_phase_status_message(esp32_flash_phase_t phase, esp32_flash_status_t status, uint16_t message)
{
    esp32_flash_phase_t last_phase = ESP32_FLASH_PHASE_INIT;
    if (phase == ESP32_FLASH_PHASE_NONE)    
    {
        phase = last_phase;
    }

    char buf[256];
    switch (phase)
    {
        case ESP32_FLASH_PHASE_WAITING_NET:
        case ESP32_FLASH_PHASE_FLASH_INIT:
        case ESP32_FLASH_PHASE_CONNECT_TARGET:
        case ESP32_FLASH_PHASE_RESET_TARGET:
            sprintf(buf, "phase: %s   status: %s    message: %d.", phase_str[phase], status_str[status], message);
            break;
        case ESP32_FLASH_PHASE_DOWNLOADING:
        case ESP32_FLASH_PHASE_FLASHING_BOOTLOADER:
        case ESP32_FLASH_PHASE_FLASHING_PART_TABLE:
        case ESP32_FLASH_PHASE_FLASHING_APP:
            if (status == ESP32_FLASH_STATUS_DOING)
            {
                sprintf(buf, "phase: %s   status: %s    message: %d %%.", phase_str[phase], status_str[status], message);
            }
            else 
            {
                sprintf(buf, "phase: %s   status: %s    message: %d.", phase_str[phase], status_str[status], message);
            }
            break;
    }
    if (status == ESP32_FLASH_STATUS_ERR)
    {
        esp32_ota_log(LERROR, buf);
    }
    else 
    {
        esp32_ota_log(LINFO, buf);
    }

    // if (phase != ESP32_FLASH_PHASE_NONE)
    //     gWordVar[REG_ESP32_FLASH_PHASE] = phase;
    // if (status != ESP32_FLASH_STATUS_NONE)
    //     gWordVar[REG_ESP32_FLASH_STATUS] = status;
    // gWordVar[REG_ESP32_FLASH_MESSAGE] = message;
}


void esp32_flash_init(void);
int esp32_flash_http_download_command(char *url, char *filepath);
int esp32_flash_flash_command(void);
int esp32_flash_reset_target_command(void);
// int esp32_flash_auto_flash_command(void);

#endif