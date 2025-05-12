#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_flash_ex.h"
#include "stm32f0xx_hal_flash.h"
#include "stm32f0xx_hal_def.h"
#include "Write_DATA.h"

//#define FLASH_BASE_ADDR 0x08007C00

extern void  FLASH_PageErase(uint32_t PageAddress);

CAN_Config config = {0321, 1};
CAN_Config config_last = {0, 0};

HAL_StatusTypeDef Flash_Write(uint32_t address, uint32_t* data, uint32_t length)
{
    HAL_StatusTypeDef status;
    HAL_FLASH_Unlock();
    FLASH_PageErase(address);  
	
    for (uint32_t i = 0; i < length; i++) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + (i * 4), data[i]);
        if (status != HAL_OK) {
            HAL_FLASH_Lock();  
            return status;
        }
    }

    HAL_FLASH_Lock();
    return HAL_OK;
}

void saveCANConfigToFlash(uint32_t canId, uint32_t baudRate_Prescaler) {
    CAN_Config config;
    config.canId = canId;
    config.baudRate_Prescaler = baudRate_Prescaler;

    HAL_StatusTypeDef status = Flash_Write(FLASH_BASE_ADDR, (uint32_t*)&config, sizeof(config) / 4);
    if (status != HAL_OK) {
        //printf("Flash write failed\n");
    }
}

CAN_Config loadCANConfigFromFlash(void) {
    //CAN_Config config;
    uint32_t* data = (uint32_t*)FLASH_BASE_ADDR;

    config.canId = data[0];
    config.baudRate_Prescaler = data[1];

    return config;
}

int judge(CAN_Config* data, CAN_Config* data_last)
{
	if(data->canId != data_last -> canId)
	{
		data_last = data;
		return 1;
	}
	if(data->baudRate_Prescaler != data_last -> baudRate_Prescaler)
	{
		data_last = data;
		return 1;
	}
	return 0;
}
