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

#ifndef _LFS_VFS_H_
#define _LFS_VFS_H_
#include "block_device.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_FS_SUPPORT_LFS_ON_NAND
#define LFS_NFTL_PROG_SIZE NFTL_PAGE_SIZE
#endif

#ifdef CONFIG_FS_SUPPORT_LFS_ON_SDCARD
#define LFS_SDCARD_PROG_SIZE 512
#define LFS_SDCARD_BLOCK_SIZE 4096
#define LFS_SDCARD_BLOCK_CYCLES_SIZE 500

#define LFS_BLOCK_DIV (LFS_SDCARD_BLOCK_SIZE / LFS_SDCARD_PROG_SIZE)
#define LFS_PHY_BLKCNT_TO_LFS_BLKCNT(phy_blkcnt) (phy_blkcnt / LFS_BLOCK_DIV)
#endif

#ifdef CONFIG_FS_SUPPORT_LFS_ON_NAND
int lfs_nftl_vfs_mount(void);
void lfs_nftl_vfs_unmount(void);
int lfs_nftl_format(const char *partition);
int lfs_nftl_set_format_flag(const char *partition);
int lfs_nftl_clean_partition(const char *partition);
#endif

#ifdef CONFIG_FS_SUPPORT_LFS_ON_SDCARD
bool lfs_sdcard_deinit(int part_no);
bool lfs_sdcard_init(int part_no, char *root, blockDevice_t *bdev);
int lfs_sdcard_vfs_mount(int part_no);
int lfs_sdcard_format_and_mount(int part_no);
int lfs_sdcard_format(const char *partition);
int lfs_sdcard_set_format_flag(const char *partition);
int lfs_sdcard_clean_partition(const char *partition);
#endif

#ifdef __cplusplus
}
#endif

#endif
