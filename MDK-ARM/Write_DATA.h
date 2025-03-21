#ifndef WRITE_DATA
#define WRITE_DATA

typedef struct {
    uint32_t canId;
    uint32_t baudRate_Prescaler;
}CAN_Config;
extern CAN_Config config;
extern CAN_Config config_last;

#define FLASH_BASE_ADDR 0x08007C00

HAL_StatusTypeDef Flash_Write(uint32_t address, uint32_t* data, uint32_t length);
void saveCANConfigToFlash(uint32_t canId, uint32_t baudRate);
CAN_Config loadCANConfigFromFlash(void);
int judge(CAN_Config* data, CAN_Config* data_last);

#endif
