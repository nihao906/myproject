/******************************************************************************
 ** File Name:      drv_spi_nand.h                                         *
 ** Author:         robert.luo1                                                *
 ** DATE:           07/05/2022                                                *
 ** Copyright:      2022 Spreatrum, Incoporated. All Rights Reserved.         *
 ** Description:                                                              *
 ******************************************************************************/

#ifndef _SPI_NAND_DRV_H_
#define _SPI_NAND_DRV_H_

#include "osi_compiler.h"

// flash chip ID list
#define DS35X4GM 0xA4E5
#define W25N01G 0x21BAEF

/*****************SPI NAND FLASH_FEATURE***********************/
#define MAX_SECTOR_PER_PAGE (4)  //By default all nand flash have a maxinum of 32 sectors per page
#define NAND_SECTOR_SIZE (512)   //Default nand flash one sector main spare have 1024 byte
#define NAND_MAX_SPARE_SIZE (16) //Default nand flash spare area have maxnum 128 byte
/******************************************************************************
                         Macro define
******************************************************************************/
typedef enum
{
    ERR_NF_SUCCESS = 0,   // Success,no error
    ERR_NF_TIMEOUT = 1,   // Oprate nand flash timeout;
    ERR_NF_ECC_RISK = 2,  // the error bit number is larger than the eccRiskVal
    ERR_NF_ECC_ERROR = 3, // the error number is out of the capacity of the ECC decoder
    ERR_NF_PARAM = 4,     //the paramter error, the top layer need check the paramter
    ERR_NF_FAIL = 5,      // Program or erase nand failed
    ERR_NF_BUSY = 6,      // write error;
    ERR_NF_PROTECTED = 7, // Current device is protected;
    ERR_NF_ECC_RAW = 8
} ERR_NF_E;

typedef enum
{
    NANDCTL_ABCD_ABCD = 0,
    NANDCTL_ABCD_DCBA = 1,
    NANDCTL_ABCD_CDAB = 2,
    NANDCTL_ABCD_BADC = 3,
    NANDCTL_ENDIAN_MAX
} NFC_DATA_ENDIAN;

/******************************************************************************
                          Struct define
******************************************************************************/
typedef struct
{
    //#if defined(_NANDC_V2_)
    uint32_t inftype;
    //#endif
    uint16_t acyCle;   //address cycle, which can be set 3,4,5
    uint16_t sctSize;  //sector size
    uint16_t pageSize; //page size, which can be set 512 or 2048, 4096, 8192, 16384
    uint16_t nPgPerBlk;
    uint16_t sctPerPage;
    uint16_t busWidth; //bus width, which can be set 0 or 1
    uint16_t advance;  //advance property, which can be set 0 or 1

    uint16_t spareSize; //spare size,
    uint16_t badPage;   // the page index of bad mark
    uint16_t badSct;
    uint16_t badPos; // the postion of bad mark
    //uint16_t spareSize;        //the spare size of one sector
    uint16_t eccMode;    // 1bit ECC, 4bit ECC,  8bit ECC, 12bit ECC,  16bit ECC, 24bit ECC,
    uint16_t eccPos;     //ECC position
    uint16_t eccRiskVal; //ECC risk value
                         //#if defined(_NANDC_V2_)
    uint16_t sInfoPos;   //spare info postion, when ecc enable this member tell nfc controller the postion of data is protected in spare part
    uint16_t sInfoSize;  //spare info postion, when ecc enable this member tell nfc controller the size of data is protected in spare part
    //#endif
} NAND_PARA_T;

typedef struct
{
    uint16_t nTAcsTime; //Setup time for both ALE and CLE
    uint16_t nTAchTime; //Hold Time for both ALE and CLE
    uint16_t nTRwlTime; //Active low pulse width for both RE and WE
    uint16_t nTRwhTime; //Hold time for both RE and WE
    uint16_t nTRrTime;  //Ready to RE low
} NAND_TIMING_T, *NAND_TIMING_PTR;

typedef struct
{
    uint16_t mEccLen; //bytes length
    uint8_t eccMode;
    bool apart;
    bool wpEn;              //write protect enable, if set 1, write is enable, else write is not enable
    NFC_DATA_ENDIAN endian; //data endian
} NAND_DEV_INFO;

typedef struct nand_device NAND_DEV_T;

typedef struct
{
    ERR_NF_E(*nandInPageWrite)
    (NAND_DEV_T *ndev, uint32_t blkId, uint32_t pageId, uint32_t sctId, uint32_t nScts, uint8_t *pMBuf, uint8_t *pSBuf, uint8_t *pStatus, uint8_t eccEn);
    ERR_NF_E(*nandInPageRead)
    (NAND_DEV_T *ndev, uint32_t blkId, uint32_t pageId, uint32_t sctId, uint32_t nScts, uint8_t *pMBuf, uint8_t *pSBuf, uint8_t *pStatus, uint8_t eccEn);
    ERR_NF_E(*nandBlockErase)
    (NAND_DEV_T *ndev, uint32_t blkId);
    ERR_NF_E(*nandReset)
    (NAND_DEV_T *ndev);
    ERR_NF_E(*nandSetFeature)
    (NAND_DEV_T *ndev, uint8_t addr, uint8_t data);
    uint32_t (*nandGetFeature)(NAND_DEV_T *ndev, uint8_t addr);
    ERR_NF_E(*nandSetBadBlk)
    (NAND_DEV_T *ndev, uint32_t blkId);
    ERR_NF_E(*nandVerifyBlk)
    (NAND_DEV_T *ndev, uint32_t blkId, uint32_t *ifGood);
} NandOps_t;

typedef struct nandInfo
{
    uint32_t ID;

    uint32_t nBlkNum;
    uint32_t nNumOfPlane;
    uint32_t nPlaneSelBits; // select plane bits in the column address
    uint32_t nBlkNumInRsv;  // NVB

    uint16_t nSctSize;   //sector size
    uint32_t nPageSize;  //page size
    uint32_t nBlkSize;   //block size
    uint16_t nSpareSize; // relate with nSctSize
    uint8_t nSInfoSize;  //  spare info size
    uint16_t nPgPerBlk;
    uint8_t nSctPerPg;
    uint8_t nBlkShift;     //blk shift in address map
    uint8_t nPageShift;    //page shift in address map
    uint8_t nSctShift;     //sector size shift
    uint8_t nApart;        //if the main data and sprare data is apart
    uint8_t nOnlyInfoData; //only spare info data move to dst spare buffer
    uint8_t nWCntPerPg;    // partial program times per page
    uint8_t badPgId;
    uint8_t badSctId;
    uint8_t badPos;      // unit is bytes, lenth is 1 bytes default
    uint8_t badLen;      // bad flag byes lenth
    int8_t safeDataPos;  // data that be protected by ecc(-1 is all reserved area)
    uint8_t safeDataLen; // data that be protected by ecc(0 is none)

    uint8_t mEccPos;        // unit is bytes
    uint8_t mEccBitLen;     // unit is bits
    uint8_t mEccRiskBitNum; // unit is bits,This value must small to ecc capability
    uint8_t sEccPos;        // unit is bytes
    uint8_t sEccBitLen;     // unit is bits
    uint8_t sEccRiskBitNum; // unit is bits,This value must small to ecc capability

    //    host driver need next infomation
    uint8_t nBWidth;   // Nand Organization X8 or X16
    uint8_t nCycleDev; // 3 Cycle, 4 Cycle, 5 Cycle device
    uint8_t nAdvance;  // the property of Advance

    uint16_t nTrTime; // read-time of NAND device (unit: usec)
    uint16_t nTwTime; //write-time of NAND device (unit: usec)
    uint16_t nTeTime; // erase-time of NAND device (unit : usec)
    uint16_t nTfTime; // transfer-time from NAND device to host

    void (*fGetOps)(NandOps_t *ops);
    uintptr_t hwp;
} NandInfo_t;

struct nand_device
{
    NandInfo_t *pNandInfo;
    NandOps_t *pNandOpsApi;
};

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t NANDCTL_HANDLE;

/******************************************************************************/
//  Description:   Open nand controller
//  Parameter:
//      csIndex    the nand flash cs index
//  Return:
//      0:failed
//  other:sucessful, the nand flash handler
/******************************************************************************/
NANDCTL_HANDLE NANDCTL_Open(void);

/******************************************************************************/
//  Description:   Set NF parameter according to pNandPara and pNandTiming
//  Parameter:
//            handler                IN  a nandctl handler, return from NANDCTL_Open
//            pNandPara     IN    the parameter of nandflash
//            pNandTiming        IN the nand  timing paramter
//  Return:
//      ERR_NF_SUCCESS    Set NF parameter successfully
//            ERR_NF_PARAM            Error input paramter
/******************************************************************************/
ERR_NF_E NANDCTL_SetParam(NANDCTL_HANDLE handler, NAND_PARA_T *pNandPara, NAND_TIMING_PTR pNandTiming, uint8_t chipidx);

/******************************************************************************/
//  Description:   Get NF parameter according to pNandPara and pNandTiming
//  Parameter:
//            handler                IN  a nandctl handler, return from NANDCTL_Open
//            pNandPara     IN    the parameter of nandflash
//            pNandTiming        IN the nand  timing paramter
//  Return:
//      ERR_NF_SUCCESS    Set NF parameter successfully
//            ERR_NF_PARAM            Error input paramter
/******************************************************************************/
ERR_NF_E NANDCTL_GetParam(NANDCTL_HANDLE handler, NAND_DEV_INFO *info);

/******************************************************************************/
//  Description:   GetNandInfo
//  Parameter:
//      handler        IN  a nandctl handler, return from NANDCTL_Open
//  Return: NandInfo_t pointer

/******************************************************************************/
NandInfo_t *NANDCTL_GetNandInfo(NANDCTL_HANDLE handler);

/**
 * This function.is an operation to setfeature for nand
 * Parameter:
 * IN handler: nandctl handler, return from NANDCTL_Open
 * IN faddr The address required for the Setfeature operation
 * IN fdata  The data required for the Setfeature operation
 * Return:
 * ERR_NF_SUCCESS: Set feature success
 * ERR_NF_PARAM: Error handler or The Open operation was not perfomed
 * ERR_NF_FAIL: Oprate nand flash fail
 * ERR_NF_TIMEOUT: Oprate nand flash timeout;
**/
ERR_NF_E NANDCTL_Setfeature(NANDCTL_HANDLE handler, uint8_t faddr, uint32_t fdata);

/**
 * This function.is an operation to getfeature for nand
 * Parameter:
 * IN handler: nandctl handler, return from NANDCTL_Open
 * IN faddr The address required for the getfeature operation
 * OUT fdata return the feature data
 * Return:
 * ERR_NF_SUCCESS: Get feature success
 * ERR_NF_PARAM: Error handler or The Open operation was not perfomed
 * ERR_NF_FAIL: Oprate nand flash fail
 * ERR_NF_TIMEOUT: Oprate nand flash timeout;
**/
ERR_NF_E NANDCTL_Getfeature(NANDCTL_HANDLE handler, uint8_t faddr, uint32_t *fdata);

/******************************************************************************/
//  Description:   Close nand controller
//  Parameter:
//      handler        IN  a nandctl handler, return from NANDCTL_Open
//  Return:
//      ERR_NF_SUCCESS    Close nand controller successfully
//            ERR_NF_PARAM            error handler
/******************************************************************************/
ERR_NF_E NANDCTL_Close(NANDCTL_HANDLE handler);

/******************************************************************************/
//  Description:   Read nandflash ID
//  Parameter:
//            handler     IN     a nandctl handler, return from NANDCTL_Open
//      pID      OUT   the address of nandflash id
//  Return:
//      ERR_NF_SUCCESS     Get the ID successfully
//            ERR_NF_PARAM             Error handler or pID is 0
/******************************************************************************/
ERR_NF_E NANDCTL_ReadID(NANDCTL_HANDLE handler, uint8_t *pID);

/******************************************************************************/
//  Description:   Reset NandFlash
//  Parameter:
//      handler     IN     a nandctl handler, return from NANDCTL_Open
//  Return:
//      ERR_NF_SUCCESS     Reset nandflash successfully
//            ERR_NF_PARAM             Error handler
/******************************************************************************/
ERR_NF_E NANDCTL_Reset(NANDCTL_HANDLE handler);

/******************************************************************************/
//  Description:   Read main part and spare part of nand, ECC value can be get
//                 if necessary.
//  Parameter:
//            handler     IN    a nandctl handler, return from NANDCTL_Open
//            blkId            IN the block id
//      pageId    IN   the nand page id.
//            sctId:            IN    the sector index of one page
//            nScts                IN    the sector number
//      pMBuf:       OUT  the address of Main part
//      pSBuf:       OUT  the address of Spare part
//       pStatus         OUT    the sector read status
//      eccEn:       IN  the ecc on or off flag
//  Return:
//    ERR_NF_SUCCESS
//    ERR_NF_TIMEOUT
//    ERR_NF_PARAM
/******************************************************************************/
ERR_NF_E NANDCTL_Read(
    NANDCTL_HANDLE handler,
    uint32_t blkId,
    uint32_t pageId,
    uint32_t sctId,
    uint32_t nScts,
    uint8_t *pMBuf,
    uint8_t *pSBuf,
    uint8_t *pStatus,
    uint8_t eccEn);
/******************************************************************************/
//  Description:   Write main part and spare part of nand
//  Parameter:
//            handler     IN    a nandctl handler, return from NANDCTL_Open
//            blkId            IN the block id
//      pageId    IN   the nand page id.
//            sctId:            IN   the sector index of one page
//            nScts                IN sector number
//      pMBuf:      IN   the address of main part
//      pSBuf:      IN   the address of spare part
//      eccEn:      IN   the flag to enable ECC
//  Return:
//    ERR_NF_SUCCESS
//    ERR_NF_TIMEOUT
//    ERR_NF_FAIL
//    ERR_NF_PARAM
/******************************************************************************/
ERR_NF_E NANDCTL_Write(
    NANDCTL_HANDLE handler,
    uint32_t blkId,
    uint32_t pageId,
    uint32_t sctId,
    uint32_t nScts,
    uint8_t *pMBuf,
    uint8_t *pSBuf,
    uint8_t eccEn,
    uint8_t *pStatus);
/******************************************************************************/
//  Description:   Erase a block of nandflash
//  Parameter:
//            handler     IN    a nandctl handler, return from NANDCTL_Open
//      blkId:  IN   the nand block ID.
//      checkBadFirst: IN check bad block first.
//  Return:
//      ERR_NF_SUCCESS    Control nand successfully
//      ERR_NF_TIMEOUT    nand is busy
//      ERR_NF_FAIL       Program or erase nand failed
//            ERR_NF_PARAM            error input paramter
/******************************************************************************/
ERR_NF_E NANDCTL_EraseBlock(NANDCTL_HANDLE handler, uint32_t blkId, bool checkBadFirst);

/******************************************************************************/
//  Description:   check, if the block is bad
//  Parameter:
//            handler     IN    a nandctl handler, return from NANDCTL_Open
//      blkId:  IN   the nand block ID.
//            ifGood   OUT    the good or bad flag address
//  Return:
//      ERR_NF_SUCCESS
//      ERR_NF_TIMEOUT
//            ERR_NF_PARAM            error input paramter
/******************************************************************************/
ERR_NF_E NANDCTL_VerifyBlk(NANDCTL_HANDLE handler, uint32_t blkId,
                           uint32_t *ifGood);
/******************************************************************************/
//  Description:   mark the physical block as bad block
//  Parameter:
//            handler     IN    a nandctl handler, return from NANDCTL_Open
//      blkId:  IN   the nand block ID.
//  Return:
//            ERR_NF_SUCCESS
//            ERR_NF_TIMEOUT
//            ERR_NF_FAIL
//            ERR_NF_PARAM            error input paramter
/******************************************************************************/
ERR_NF_E NANDCTL_SetBadBlk(NANDCTL_HANDLE handler, uint32_t blkId);

uint8_t foundOneBitCount(char val);

#ifdef __cplusplus
}
#endif

#endif //DRV_SPI_H_
