/* Copyright (C) 2016 RDA Technologies Limited and/or its affiliates("RDA").
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

#ifndef _TFLASH_BLOCK_DEVICE_H_
#define _TFLASH_BLOCK_DEVICE_H_

#include "block_device.h"

#include "quec_proj_config.h"
#include "quec_internal_cfg.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CONFIG_FS_SUPPORT_LFS_ON_SDCARD)
#define TFLASH_BLOCK_DEV_COUNT CONFIG_FS_SDCARD_PART_NUM
#elif defined(CONFIG_FS_SUPPORT_FAT_ON_SDCARD)
#define TFLASH_BLOCK_DEV_COUNT 1 // FAT seen the device only as a whole partition
#else
#define TFLASH_BLOCK_DEV_COUNT 1 // default
#endif

// according to the sdcard storage size, this sample has 1909200
// 15523840: 8GB sdcard, 512bytes/sector
#define LFS_SDCARD_BLOCK_COUNT_PART0 3880960
#define LFS_SDCARD_BLOCK_COUNT_PART1 3880960
#define LFS_SDCARD_BLOCK_COUNT_PART2 3880960

#ifdef QUEC_PROJECT_FEATURE_SD_EMMC
blockDevice_t *tflash_device_create(uint32_t name);
#else
int tflash_device_create(uint32_t name);
#endif

blockDevice_t *tflash_device_get(int dev_no);
void tflash_destroy_device(void);

#ifdef __cplusplus
}
#endif

#endif // H
