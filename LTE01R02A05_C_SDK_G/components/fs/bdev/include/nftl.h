#ifndef __NFTL_H
#define __NFTL_H

#include "nftl_conf.h"
#include "nftl_util.h"
#include "drv_spi_nand.h"
#include "block_device.h"

#ifdef __cplusplus
extern "C" {
#endif

/* FTL error codes */
typedef enum
{
    NFTL_ERR_OK,
    NFTL_ERR_INVALID_ARGUMENT,
    NFTL_ERR_NAND_BAD,
    NFTL_ERR_NAND_IO,
    NFTL_ERR_NAND_NO_PAGE,
    NFTL_ERR_NAND_ERASED_PENDING_WRITE,
    NFTL_ERR_NAND_UNKNOWN,
} nftl_err_t;

typedef uint32_t nftl_block_t;
typedef uint32_t nftl_offset_t;
typedef uint32_t nftl_size_t;

typedef struct
{
    uint32_t page_id;
    uint32_t reversion;
} nftl_logic_info_t;

typedef struct
{
    uint32_t logic_block;
    uint32_t reversion;
} nftl_block_tag;

typedef struct
{
    NANDCTL_HANDLE(*_open)
    (void);
    ERR_NF_E(*_close)
    (NANDCTL_HANDLE handler);
    ERR_NF_E(*_eraseBlk)
    (NANDCTL_HANDLE handler, uint32_t blkId, bool checkBadFirst);
    ERR_NF_E(*_read)
    (
        NANDCTL_HANDLE handler,
        uint32_t blkId,
        uint32_t pageId,
        uint8_t *pMBuf,
        nftl_block_tag *pBlockTag,
        uint8_t eccEn);
    ERR_NF_E(*_write)
    (
        NANDCTL_HANDLE handler,
        uint32_t blkId,
        uint32_t pageId,
        uint8_t *pMBuf,
        nftl_block_tag *pBlockTag,
        uint8_t eccEn);
    ERR_NF_E(*_verifyBlk)
    (NANDCTL_HANDLE handler, uint32_t blkId, uint32_t *ifGood);
    ERR_NF_E(*_setBadBlk)
    (NANDCTL_HANDLE handler, uint32_t blkId);
} nftl_flash_ops_t;

typedef struct
{
    blockDevice_t *bdev;
    uint8_t *page_buff;
    nftl_block_tag block_tag;
    uint32_t phy_block_nums;
    uint8_t *phy_blocks;
    uint32_t logic_block_nums;
    nftl_logic_info_t *logic_blocks;
    uint32_t allocating_blk;
    uint32_t allocating_page;
    nftl_block_tag *allocating_blk_tags;
    NANDCTL_HANDLE chip_hdlr;
    uint32_t start_block;
    osiMutex_t *par_lock;
    uint8_t initialized;
    uint32_t bad_block_nums;
    uint32_t gc_times;
} nftl_block_mgr_t;

typedef struct
{
    uint32_t part_num; //total partition number
    uint32_t page_size;
    uint32_t pages_per_block;
    uint32_t page_bitmap_len;
    bool enable_ecc;
    nftl_block_mgr_t *block_mgrs;
    nftl_flash_ops_t ops;
} nftl_config_t;

bool nftl_init(nftl_config_t *cfg);
blockDevice_t *nftl_device_get(uint32_t part);

#ifdef NFTL_YES_TRACE
void nftl_dump(nftl_block_mgr_t *mgr);
#endif

#ifdef NFTL_GC_NOTIFY
nftl_err_t nftl_flash_notify_gabage(uint32_t p, nftl_block_t blk);
#endif /* NFTL_GC_NOTIFY */

#ifdef __cplusplus
}
#endif

#endif
