
#ifndef NFTL_CONF_H
#define NFTL_CONF_H
#include "fs_config.h"

#define NFTL_ERASED_VALUE 0xff
#define NFTL_ENABLE_ECC (true)

/* The page size, in bytes, of the flash used for NFTL storage. */
#ifndef CONFIG_NFTL_PAGE_SIZE
#define NFTL_PAGE_SIZE (2 * 1024)
#else
#define NFTL_PAGE_SIZE CONFIG_NFTL_PAGE_SIZE
#endif

/* page number per block */
#ifndef CONFIG_NFTL_PAGE_PER_BLOCK_NUMS
#define NFTL_PAGE_PER_BLOCK_NUMS 64
#else
#define NFTL_PAGE_PER_BLOCK_NUMS NFTL_CONFIG_PAGE_PER_BLOCK_NUMS
#endif

/* nftl logical partition numbers.
 * TODO: modify below lines if partition numbers changed.
 */
#define NFTL_LOGICAL_PARTITION_NUM CONFIG_FS_NFTL_PART_NUM

/*
 * convert physical block nums to raw block nums,
 * reserve 5% space for bad block(buffer blocks),
 * 1 physical block includes n rawpages + 1 tagpage
 * raw block nums aligned to 16
 */
#define NFTL_BLOCK_NUMS_PHY2RAW(phy_num)                                 \
    (((phy_num - (phy_num * 5 / 100)) * (NFTL_PAGE_PER_BLOCK_NUMS - 1) / \
      NFTL_PAGE_PER_BLOCK_NUMS) &                                        \
     0xfffff0)

// how many physical block needed.
#define NFTL_BLOCK_NUMS_RAW2PHY(raw_num)                                     \
    ((raw_num * NFTL_PAGE_PER_BLOCK_NUMS + (NFTL_PAGE_PER_BLOCK_NUMS - 2)) / \
     (NFTL_PAGE_PER_BLOCK_NUMS - 1))

/* ***************************************************************
 * TODO: Modify below lines if filesystem's partition size changed.
 * Below lines must be the same as nftl user's settings,
 * e.g littefs system/data size (in blocks).
 * ***************************************************************
 */
#define NFTL_PART0_ALLOC_BLOCK_NUMS (512)
#define NFTL_PART1_ALLOC_BLOCK_NUMS (512)
#define NFTL_PART2_ALLOC_BLOCK_NUMS (2048)

#define NFTL_PART0_RAW_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_PHY2RAW(NFTL_PART0_ALLOC_BLOCK_NUMS)
#define NFTL_PART1_RAW_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_PHY2RAW(NFTL_PART1_ALLOC_BLOCK_NUMS)
#define NFTL_PART2_RAW_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_PHY2RAW(NFTL_PART2_ALLOC_BLOCK_NUMS)

#define NFTL_PART0_LOGICAL_BLOCK_NUMS (NFTL_PART0_RAW_BLOCK_NUMS * NFTL_PAGE_PER_BLOCK_NUMS)
#define NFTL_PART1_LOGICAL_BLOCK_NUMS (NFTL_PART1_RAW_BLOCK_NUMS * NFTL_PAGE_PER_BLOCK_NUMS)
#define NFTL_PART2_LOGICAL_BLOCK_NUMS (NFTL_PART2_RAW_BLOCK_NUMS * NFTL_PAGE_PER_BLOCK_NUMS)

#define NFTL_PART0_PHY_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_RAW2PHY(NFTL_PART0_RAW_BLOCK_NUMS)

/* buffer blocks in nftl partition, used for bad block replacement.
 * It's proposed to be 5% used for bad block.
 */
#define NFTL_PART0_BUFFER_BLOCK_NUMS \
    (NFTL_PART0_ALLOC_BLOCK_NUMS - NFTL_PART0_PHY_BLOCK_NUMS)

/* nftl logical partition 1 blocks, i.e. after 'raw + tagpage' convertion. */
#define NFTL_PART1_PHY_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_RAW2PHY(NFTL_PART1_RAW_BLOCK_NUMS)

#define NFTL_PART1_BUFFER_BLOCK_NUMS \
    (NFTL_PART1_ALLOC_BLOCK_NUMS - NFTL_PART1_PHY_BLOCK_NUMS)

/* nftl logical partition 2 blocks, i.e. after 'raw + tagpage' convertion. */
#define NFTL_PART2_PHY_BLOCK_NUMS \
    NFTL_BLOCK_NUMS_RAW2PHY(NFTL_PART2_RAW_BLOCK_NUMS)

#define NFTL_PART2_BUFFER_BLOCK_NUMS \
    (NFTL_PART2_ALLOC_BLOCK_NUMS - NFTL_PART2_PHY_BLOCK_NUMS)

/* TODO: modify below line if more logical partition added */
#if NFTL_LOGICAL_PARTITION_NUM == 1
#define NFTL_LOGICAL_TOTAL_BLOCK_NUMS (NFTL_PART0_LOGICAL_BLOCK_NUMS)
#define NFTL_TOTAL_STORAGE_SIZE (NFTL_PART0_ALLOC_BLOCK_NUMS)
#elif NFTL_LOGICAL_PARTITION_NUM == 2
#define NFTL_LOGICAL_TOTAL_BLOCK_NUMS (NFTL_PART0_LOGICAL_BLOCK_NUMS + NFTL_PART1_LOGICAL_BLOCK_NUMS)
#define NFTL_TOTAL_STORAGE_SIZE (NFTL_PART0_ALLOC_BLOCK_NUMS + NFTL_PART1_ALLOC_BLOCK_NUMS)
#elif NFTL_LOGICAL_PARTITION_NUM == 3
#define NFTL_LOGICAL_TOTAL_BLOCK_NUMS (NFTL_PART0_LOGICAL_BLOCK_NUMS + NFTL_PART1_LOGICAL_BLOCK_NUMS + NFTL_PART2_LOGICAL_BLOCK_NUMS)
#define NFTL_TOTAL_STORAGE_SIZE (NFTL_PART0_ALLOC_BLOCK_NUMS + NFTL_PART1_ALLOC_BLOCK_NUMS + NFTL_PART2_ALLOC_BLOCK_NUMS)
#elif
#error Please define more NFTL logical paritions (now 3 but need more)!
#endif

/* The block size, in bytes, of the flash used for NFTL storage. */
#define NFTL_BLOCK_SIZE (NFTL_PAGE_PER_BLOCK_NUMS * NFTL_PAGE_SIZE)

/* The total nftl storage size. */
#define NFTL_TOTAL_SIZE (NFTL_BLOCK_NUMS * NFTL_BLOCK_SIZE)

#endif /* NFTL_CONF_H */
