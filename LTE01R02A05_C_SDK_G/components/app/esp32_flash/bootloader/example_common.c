/* Copyright 2020-2023 Espressif Systems (Shanghai) CO LTD
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/param.h>
#include "ec600u_port.h"
#include "esp_loader.h"
#include "example_common.h"

#include "ql_fs.h"
#include "ql_log.h"

#define QL_GPIODEMO_LOG_LEVEL   QL_LOG_LEVEL_INFO
#define LOGI(msg, ...)          QL_LOG(QL_GPIODEMO_LOG_LEVEL, "example_common", msg, ##__VA_ARGS__)


// #ifndef SINGLE_TARGET_SUPPORT


// For esp8266, esp32, esp32s2
#define BOOTLOADER_ADDRESS_V0       0x1000
// For esp32s3 and later chips
#define BOOTLOADER_ADDRESS_V1       0x0
#define PARTITION_ADDRESS           0x8000
#define APPLICATION_ADDRESS         0x10000

// extern const uint8_t  ESP32_bootloader_bin[];
// extern const uint32_t ESP32_bootloader_bin_size;
// extern const uint8_t  ESP32_hello_world_bin[];
// extern const uint32_t ESP32_hello_world_bin_size;
// extern const uint8_t  ESP32_partition_table_bin[];
// extern const uint32_t ESP32_partition_table_bin_size;

// extern const uint8_t  ESP32_S2_bootloader_bin[];
// extern const uint32_t ESP32_S2_bootloader_bin_size;
// extern const uint8_t  ESP32_S2_hello_world_bin[];
// extern const uint32_t ESP32_S2_hello_world_bin_size;
// extern const uint8_t  ESP32_S2_partition_table_bin[];
// extern const uint32_t ESP32_S2_partition_table_bin_size;

// uint8_t  ESP32_S3_bootloader_bin[] = {0x11, 0x12};
// uint32_t ESP32_S3_bootloader_bin_size = 2;
// uint8_t  ESP32_S3_hello_world_bin[] = {0x21, 0x22, 0x23};
// uint32_t ESP32_S3_hello_world_bin_size = 3;
// uint8_t  ESP32_S3_partition_table_bin[] = {0x31, 0x32, 0x33, 0x44};
// uint32_t ESP32_S3_partition_table_bin_size = 4;



// extern const uint8_t  ESP8266_bootloader_bin[];
// extern const uint32_t ESP8266_bootloader_bin_size;
// extern const uint8_t  ESP8266_hello_world_bin[];
// extern const uint32_t ESP8266_hello_world_bin_size;
// extern const uint8_t  ESP8266_partition_table_bin[];
// extern const uint32_t ESP8266_partition_table_bin_size;

// extern const uint8_t  ESP32_H4_bootloader_bin[];
// extern const uint32_t ESP32_H4_bootloader_bin_size;
// extern const uint8_t  ESP32_H4_hello_world_bin[];
// extern const uint32_t ESP32_H4_hello_world_bin_size;
// extern const uint8_t  ESP32_H4_partition_table_bin[];
// extern const uint32_t ESP32_H4_partition_table_bin_size;

// extern const uint8_t  ESP32_H2_bootloader_bin[];
// extern const uint32_t ESP32_H2_bootloader_bin_size;
// extern const uint8_t  ESP32_H2_hello_world_bin[];
// extern const uint32_t ESP32_H2_hello_world_bin_size;
// extern const uint8_t  ESP32_H2_partition_table_bin[];
// extern const uint32_t ESP32_H2_partition_table_bin_size;

// extern const uint8_t  ESP32_C2_bootloader_bin[];
// extern const uint32_t ESP32_C2_bootloader_bin_size;
// extern const uint8_t  ESP32_C2_hello_world_bin[];
// extern const uint32_t ESP32_C2_hello_world_bin_size;
// extern const uint8_t  ESP32_C2_partition_table_bin[];
// extern const uint32_t ESP32_C2_partition_table_bin_size;

// extern const uint8_t  ESP32_C3_bootloader_bin[];
// extern const uint32_t ESP32_C3_bootloader_bin_size;
// extern const uint8_t  ESP32_C3_hello_world_bin[];
// extern const uint32_t ESP32_C3_hello_world_bin_size;
// extern const uint8_t  ESP32_C3_partition_table_bin[];
// extern const uint32_t ESP32_C3_partition_table_bin_size;

// extern const uint8_t  ESP32_C6_bootloader_bin[];
// extern const uint32_t ESP32_C6_bootloader_bin_size;
// extern const uint8_t  ESP32_C6_hello_world_bin[];
// extern const uint32_t ESP32_C6_hello_world_bin_size;
// extern const uint8_t  ESP32_C6_partition_table_bin[];
// extern const uint32_t ESP32_C6_partition_table_bin_size;

void get_example_binaries(target_chip_t target, example_binaries_t *bins)
{
    // if (target == ESP8266_CHIP) {
    //     bins->boot.data = ESP8266_bootloader_bin;
    //     bins->boot.size = ESP8266_bootloader_bin_size;
    //     bins->boot.addr = BOOTLOADER_ADDRESS_V0;
    //     bins->part.data = ESP8266_partition_table_bin;
    //     bins->part.size = ESP8266_partition_table_bin_size;
    //     bins->part.addr = PARTITION_ADDRESS;
    //     bins->app.data  = ESP8266_hello_world_bin;
    //     bins->app.size  = ESP8266_hello_world_bin_size;
    //     bins->app.addr  = APPLICATION_ADDRESS;
    // } else if (target == ESP32_CHIP) {
    //     bins->boot.data = ESP32_bootloader_bin;
    //     bins->boot.size = ESP32_bootloader_bin_size;
    //     bins->boot.addr = BOOTLOADER_ADDRESS_V0;
    //     bins->part.data = ESP32_partition_table_bin;
    //     bins->part.size = ESP32_partition_table_bin_size;
    //     bins->part.addr = PARTITION_ADDRESS;
    //     bins->app.data  = ESP32_hello_world_bin;
    //     bins->app.size  = ESP32_hello_world_bin_size;
    //     bins->app.addr  = APPLICATION_ADDRESS;
    // } else if (target == ESP32S2_CHIP) {
    //     bins->boot.data = ESP32_S2_bootloader_bin;
    //     bins->boot.size = ESP32_S2_bootloader_bin_size;
    //     bins->boot.addr = BOOTLOADER_ADDRESS_V0;
    //     bins->part.data = ESP32_S2_partition_table_bin;
    //     bins->part.size = ESP32_S2_partition_table_bin_size;
    //     bins->part.addr = PARTITION_ADDRESS;
    //     bins->app.data  = ESP32_S2_hello_world_bin;
    //     bins->app.size  = ESP32_S2_hello_world_bin_size;
    //     bins->app.addr  = APPLICATION_ADDRESS;
    // } else if (target == ESP32H2_CHIP) {
    //     bins->boot.data = ESP32_H2_bootloader_bin;
    //     bins->boot.size = ESP32_H2_bootloader_bin_size;
    //     bins->boot.addr = BOOTLOADER_ADDRESS_V1;
    //     bins->part.data = ESP32_H2_partition_table_bin;
    //     bins->part.size = ESP32_H2_partition_table_bin_size;
    //     bins->part.addr = PARTITION_ADDRESS;
    //     bins->app.data  = ESP32_H2_hello_world_bin;
    //     bins->app.size  = ESP32_H2_hello_world_bin_size;
    //     bins->app.addr  = APPLICATION_ADDRESS;
    // } else if (target == ESP32C2_CHIP) {
    //     bins->boot.data = ESP32_C2_bootloader_bin;
    //     bins->boot.size = ESP32_C2_bootloader_bin_size;
    //     bins->boot.addr = BOOTLOADER_ADDRESS_V1;
    //     bins->part.data = ESP32_C2_partition_table_bin;
    //     bins->part.size = ESP32_C2_partition_table_bin_size;
    //     bins->part.addr = PARTITION_ADDRESS;
    //     bins->app.data  = ESP32_C2_hello_world_bin;
    //     bins->app.size  = ESP32_C2_hello_world_bin_size;
    //     bins->app.addr  = APPLICATION_ADDRESS;
    // } else if (target == ESP32C3_CHIP) {
    //     bins->boot.data = ESP32_C3_bootloader_bin;
    //     bins->boot.size = ESP32_C3_bootloader_bin_size;
    //     bins->boot.addr = BOOTLOADER_ADDRESS_V1;
    //     bins->part.data = ESP32_C3_partition_table_bin;
    //     bins->part.size = ESP32_C3_partition_table_bin_size;
    //     bins->part.addr = PARTITION_ADDRESS;
    //     bins->app.data  = ESP32_C3_hello_world_bin;
    //     bins->app.size  = ESP32_C3_hello_world_bin_size;
    //     bins->app.addr  = APPLICATION_ADDRESS;
    // } else if (target == ESP32C6_CHIP) {
    //     bins->boot.data = ESP32_C6_bootloader_bin;
    //     bins->boot.size = ESP32_C6_bootloader_bin_size;
    //     bins->boot.addr = BOOTLOADER_ADDRESS_V1;
    //     bins->part.data = ESP32_C6_partition_table_bin;
    //     bins->part.size = ESP32_C6_partition_table_bin_size;
    //     bins->part.addr = PARTITION_ADDRESS;
    //     bins->app.data  = ESP32_C6_hello_world_bin;
    //     bins->app.size  = ESP32_C6_hello_world_bin_size;
    //     bins->app.addr  = APPLICATION_ADDRESS;

    // } 
    if (target == ESP32S3_CHIP) {
        // bins->boot.data = ESP32_S3_bootloader_bin;
        // bins->boot.size = ESP32_S3_bootloader_bin_size;
        bins->boot.addr = BOOTLOADER_ADDRESS_V1;
        // bins->part.data = ESP32_S3_partition_table_bin;
        // bins->part.size = ESP32_S3_partition_table_bin_size;
        bins->part.addr = PARTITION_ADDRESS;
        // bins->app.data  = ESP32_S3_hello_world_bin;
        // bins->app.size  = ESP32_S3_hello_world_bin_size;
        bins->app.addr  = APPLICATION_ADDRESS;
    } else {
        LOGI("not has target chip!");
        //abort();
    }
}


// extern const uint8_t  ESP32_app_bin[];
// extern const uint32_t ESP32_app_bin_size;
// extern const uint8_t  ESP32_C2_app_bin[];
// extern const uint32_t ESP32_C2_app_bin_size;
// extern const uint8_t  ESP32_C3_app_bin[];
// extern const uint32_t ESP32_C3_app_bin_size;
// extern const uint8_t  ESP32_H2_app_bin[];
// extern const uint32_t ESP32_H2_app_bin_size;
// extern const uint8_t  ESP32_H4_app_bin[];
// extern const uint32_t ESP32_H4_app_bin_size;
// uint8_t  ESP32_S3_app_bin[] = {0x01, 0x02};
// uint32_t ESP32_S3_app_bin_size = 2;
// extern const uint8_t  ESP32_C6_app_bin[];
// extern const uint32_t ESP32_C6_app_bin_size;

void get_example_ram_app_binary(target_chip_t target, example_ram_app_binary_t *bin)
{
    switch (target) {
    // case ESP32_CHIP: {
    //     bin->ram_app.data = ESP32_app_bin;
    //     bin->ram_app.size = ESP32_app_bin_size;
    //     break;
    // }
    // case ESP32C2_CHIP: {
    //     bin->ram_app.data = ESP32_C2_app_bin;
    //     bin->ram_app.size = ESP32_C2_app_bin_size;
    //     break;
    // }
    // case ESP32C3_CHIP: {
    //     bin->ram_app.data = ESP32_C3_app_bin;
    //     bin->ram_app.size = ESP32_C3_app_bin_size;
    //     break;
    // }
    // case ESP32H2_CHIP: {
    //     bin->ram_app.data = ESP32_H2_app_bin;
    //     bin->ram_app.size = ESP32_H2_app_bin_size;
    //     break;
    // }
    case ESP32S3_CHIP: {
        // bin->ram_app.data = ESP32_S3_app_bin;
        // bin->ram_app.size = ESP32_S3_app_bin_size;
        break;
    }
    // case ESP32C6_CHIP: {
    //     bin->ram_app.data = ESP32_C6_app_bin;
    //     bin->ram_app.size = ESP32_C6_app_bin_size;
    //     break;
    // }
    default: {
        LOGI("not have target chip!");
     //   abort();
    }
    }
}

// #endif

#include "esp32_flash.h"
esp_loader_error_t connect_to_target(uint32_t higher_transmission_rate)
{
    esp_loader_error_t err = 0;
    esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_CONNECT_TARGET, ESP32_FLASH_STATUS_DOING, 0);

    esp_loader_connect_args_t connect_config = ESP_LOADER_CONNECT_DEFAULT();
    err = esp_loader_connect(&connect_config);
    if (err != ESP_LOADER_SUCCESS) {
        LOGI("Cannot connect to target. Error: %u", err);
        goto exit;
    }

// #ifdef SERIAL_FLASHER_INTERFACE_UART
    if (higher_transmission_rate && esp_loader_get_target() != ESP8266_CHIP) {
        err = esp_loader_change_transmission_rate(higher_transmission_rate);
        if (err == ESP_LOADER_ERROR_UNSUPPORTED_FUNC) {
            LOGI("ESP8266 does not support change transmission rate command.");
            goto exit;
        } else if (err != ESP_LOADER_SUCCESS) {
            LOGI("Unable to change transmission rate on target.");
            goto exit;
        } else {
            err = loader_port_change_transmission_rate(higher_transmission_rate);
            if (err != ESP_LOADER_SUCCESS) {
                LOGI("Unable to change transmission rate.");
                goto exit;
            }
            LOGI("Transmission rate changed changed");
        }
    }
// #endif /* SERIAL_FLASHER_INTERFACE_UART */
exit:
    if (err == 0)
    {
        esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_CONNECT_TARGET, ESP32_FLASH_STATUS_FINISH, 0);
    }
    else 
    {
        esp32_flash_set_phase_status_message(ESP32_FLASH_PHASE_CONNECT_TARGET, ESP32_FLASH_STATUS_ERR, (uint16_t)err);
    }
    return err;
}


/* 传入待烧写文件的路径，以及烧写地址 */
esp_loader_error_t flash_binary1(char *file_path, size_t address, esp32_flash_phase_t phase)
{
    esp_loader_error_t err = 0;
    static uint8_t payload[1024];
    int payload_size = 1024;
    int fd = 0;
    int size = 0;
    uint32_t read_len = 0;

    esp32_flash_set_phase_status_message(phase, ESP32_FLASH_STATUS_DOING, 0);

    fd = ql_fopen(file_path, "rb");
    if(fd < 0)
    {
    	LOGI("%s file open failed:0x%x!", file_path, (unsigned int)fd);
        err = -11;
        goto exit;
    }

    size = ql_fseek(fd, 0, QL_SEEK_END);
    ql_fseek(fd, 0, QL_SEEK_SET);

    LOGI("Erasing flash (this may take a while)...");
    err = esp_loader_flash_start(address, size, sizeof(payload));
    if (err != ESP_LOADER_SUCCESS) {
        LOGI("Erasing flash failed with error %d.", err);
        goto exit;
    }
    LOGI("Start programming");

    size_t binary_size = size;
    size_t written = 0;

    while (size > 0) {
        size_t to_read = MIN(size, payload_size);
        read_len = ql_fread(payload, 1, to_read, fd);
        to_read = read_len;

        err = esp_loader_flash_write(payload, to_read);
        if (err != ESP_LOADER_SUCCESS) {
            LOGI("\nPacket could not be written! Error %d.", err);
            goto exit;
        }

        size -= to_read;
        written += to_read;

        int progress = (int)(((float)written / binary_size) * 100);
        LOGI("\rProgress: %d %%", progress);
        esp32_flash_set_phase_status_message(phase, ESP32_FLASH_STATUS_DOING, (uint16_t)progress);
        // fflush(stdout);
    };
    ql_fclose(fd);
    LOGI("\nFinished programming");

// #if MD5_ENABLED
    // err = esp_loader_flash_verify();s
    // if (err == ESP_LOADER_ERROR_UNSUPPORTED_FUNC) {
    //     LOGI("ESP8266 does not support flash verify command.");
    //     return err;
    // } else if (err != ESP_LOADER_SUCCESS) {
    //     LOGI("MD5 does not match. err: %d", err);
    //     return err;
    // }
    // LOGI("Flash verified");
// #endif
exit:
    if (err == 0)
    {
        esp32_flash_set_phase_status_message(phase, ESP32_FLASH_STATUS_FINISH, 0);
    }
    else 
    {
        esp32_flash_set_phase_status_message(phase, ESP32_FLASH_STATUS_ERR, (uint16_t)err);
    }
    return err;
}

// #ifdef SERIAL_FLASHER_INTERFACE_UART
esp_loader_error_t flash_binary(const uint8_t *bin, size_t size, size_t address)
{
    esp_loader_error_t err;
    static uint8_t payload[1024];
    const uint8_t *bin_addr = bin;

    LOGI("Erasing flash (this may take a while)...");
    err = esp_loader_flash_start(address, size, sizeof(payload));
    if (err != ESP_LOADER_SUCCESS) {
        LOGI("Erasing flash failed with error %d.", err);
        return err;
    }
    LOGI("Start programming");

    size_t binary_size = size;
    size_t written = 0;

    while (size > 0) {
        size_t to_read = MIN(size, sizeof(payload));
        memcpy(payload, bin_addr, to_read);

        err = esp_loader_flash_write(payload, to_read);
        if (err != ESP_LOADER_SUCCESS) {
            LOGI("\nPacket could not be written! Error %d.", err);
            return err;
        }

        size -= to_read;
        bin_addr += to_read;
        written += to_read;

        int progress = (int)(((float)written / binary_size) * 100);
        LOGI("\rProgress: %d %%", progress);
        fflush(stdout);
    };

    LOGI("\nFinished programming");

// #if MD5_ENABLED
    err = esp_loader_flash_verify();
    if (err == ESP_LOADER_ERROR_UNSUPPORTED_FUNC) {
        LOGI("ESP8266 does not support flash verify command.");
        return err;
    } else if (err != ESP_LOADER_SUCCESS) {
        LOGI("MD5 does not match. err: %d", err);
        return err;
    }
    LOGI("Flash verified");
// #endif

    return ESP_LOADER_SUCCESS;
}
// #endif /* SERIAL_FLASHER_INTERFACE_UART */

esp_loader_error_t load_ram_binary(const uint8_t *bin)
{
    LOGI("Start loading");
    esp_loader_error_t err;
    const esp_loader_bin_header_t *header = (const esp_loader_bin_header_t *)bin;
    esp_loader_bin_segment_t segments[header->segments];

    // Parse segments
    uint32_t seg;
    uint32_t *cur_seg_pos;
    for (seg = 0, cur_seg_pos = (uint32_t *)(&bin[BIN_FIRST_SEGMENT_OFFSET]);
            seg < header->segments;
            seg++) {
        segments[seg].addr = *cur_seg_pos++;
        segments[seg].size = *cur_seg_pos++;
        segments[seg].data = (uint8_t *)cur_seg_pos;
        cur_seg_pos += (segments[seg].size) / 4;
    }

    // Download segments
    for (seg = 0; seg < header->segments; seg++) {
    //    LOGI("Downloading %"PRIu32" bytes at 0x%08"PRIx32"...", segments[seg].size, segments[seg].addr);

        err = esp_loader_mem_start(segments[seg].addr, segments[seg].size, ESP_RAM_BLOCK);
        if (err != ESP_LOADER_SUCCESS) {
            LOGI("Loading ram start with error %d.", err);
            return err;
        }

        size_t remain_size = segments[seg].size;
        uint8_t *data_pos = segments[seg].data;
        while (remain_size > 0) {
            size_t data_size = MIN(ESP_RAM_BLOCK, remain_size);
            err = esp_loader_mem_write(data_pos, data_size);
            if (err != ESP_LOADER_SUCCESS) {
                LOGI("\nPacket could not be written! Error %d.", err);
                return err;
            }
            data_pos += data_size;
            remain_size -= data_size;
        }
    }

    err = esp_loader_mem_finish(header->entrypoint);
    if (err != ESP_LOADER_SUCCESS) {
        LOGI("\nLoad ram finish with Error %d.", err);
        return err;
    }
    LOGI("\nFinished loading");

    return ESP_LOADER_SUCCESS;
}
