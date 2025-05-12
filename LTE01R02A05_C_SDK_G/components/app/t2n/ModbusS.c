#include "ModbusS.h"
#include "ql_gpio.h"
#include <string.h>
#include "osi_api.h"
#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_fs.h"
#include "udp_ota_shell.h"
#include "../t2n/include/cfg.h"
#include "../ec600u_bl0939/ac_current.h"

static const uint8_t auchCRCHi[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40};
/* CRC??????*/
static const uint8_t auchCRCLo[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04,
    0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8,
    0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC,
    0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11, 0xD1, 0xD0, 0x10,
    0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,
    0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38,
    0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C,
    0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0,
    0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4,
    0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,
    0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C,
    0xB4, 0x74, 0x75, 0xB5, 0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0,
    0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54,
    0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98,
    0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80, 0x40};

uint16_t crc16(uint8_t *puchMsg, uint16_t usDataLen)
{
    uint8_t uchCRCHi = 0xFF; /* ?CRC????? */
    uint8_t uchCRCLo = 0xFF; /* ?CRC ????? */
    uint32_t uIndex;         /* CRC?????? */
    while (usDataLen--)      /* ??????? */
    {
        uIndex = uchCRCHi ^ *puchMsg++; /* ??CRC */
        uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex];
        uchCRCLo = auchCRCLo[uIndex];
    }
    return (uchCRCHi << 8 | uchCRCLo);
} // uint16 crc16(uint8 *puchMsg, uint16 usDataLen)

uint16_t gWordVar[gWORD_SIZE];
uint8_t gBitVar[(gBIT_SIZE + 7) / 8];

int isBitHI(uint16_t Add)
{
    if (gBitVar[(Add) >> 3] & (1 << ((Add) & 0x07)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void ModBusBitWriteHook(uint16_t addr, uint16_t length)
{
}
extern osiThread_t *rtk_thread;
extern osiThread_t *modbus_slave_thread;
#define SHAKE_CFG_FILE_PATH "SD:/shake/shake_cfg.json"
// void save_shake_cfg(float feq_min, float feq_max, float a_th, const char *filePath);
// extern float *feq_min;
// extern float *feq_max;
// extern float *a_th;
#include "../esp32_flash/inc/esp32_flash.h"
extern void set_status(uint8_t status);
int zb_ModBusWordWriteHook(uint16_t addr, uint16_t length);
void ModBusWordWriteHook(uint16_t addr, uint16_t length)
{

    // if (addr == 511 && gWordVar[511] == 0x55AA)
    // {
    // }
    // else{
    zb_ModBusWordWriteHook(addr, length);
    // }
    if (addr == 32)
    {
        ql_gpio_set_level(GPIO_12, gWordVar[addr] & 0x01);
        ql_gpio_set_level(GPIO_7, (gWordVar[addr] >> 1) & 0x01);
        ql_gpio_set_level(GPIO_0, gWordVar[addr] & 0x01);
        ql_gpio_set_level(GPIO_2, (gWordVar[addr] >> 1) & 0x01);
    }
    else if (addr == REG_NET_STATUS)
    {
        set_status(gWordVar[addr]);
    }
    if (addr == 511 && gWordVar[511] == 0x55AA)
    {
        osiEvent_t event;
        event.id = 9999;
        osiEventSend(rtk_thread, &event);
        gWordVar[511] = 0;
    }
    if (addr == REG_SHAKE_WRITE_MAGIC && gWordVar[REG_SHAKE_WRITE_MAGIC] == 0x55AA) // 保存震动检测的参数
    {
        QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "save cfg");
        // save_shake_cfg(*feq_min, *feq_max, *a_th, SHAKE_CFG_FILE_PATH);
    }
    // if(addr == REG_CFG_SERSER_SAVE  && gWordVar[REG_CFG_SERSER_SAVE] == SENSER_CFG_MAGIC)
    // {
    //     // LOGI("config->dosing_up %d",config->dosing_up);
    //     QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "config->work_up %d",config->work_up);
    //     int ret = ql_nvm_fwrite("current_cfg", config, sizeof(config_t), 1);
    //     // int ret1 = ql_nvm_fwrite("hammer_cfg", &dosing, sizeof(hammer_config_t), 1);
    //     // int ret2 = ql_nvm_fwrite("work_cfg", &dosing, sizeof(total_config_t), 1);
    //     // // com_poll_init();
    //     gWordVar[REG_CFG_SERSER_SAVE] = ret;

    // }



    // if (addr == REG_ESP32_FLASH_COMMAND && esp32_flash_command_is_valid())  // flash相关操作
    // {
    //     osiEvent_t event;
    //     event.id = esp32_flash_get_command();
    //     osiEventSend(modbus_slave_thread, &event);
    //     esp32_flash_set_command(ESP32_FLASH_COMMAND_NONE);
    // }
}

void ModbusSetLocalReg(uint16_t addr, uint16_t value)
{
    QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "enter ModbusSetLocalReg\n");
    if (addr >= gWORD_SIZE)
    {
        QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "addr >= gWORD_SIZE\n");
        return;
    }
    gWordVar[addr] = value;
    int bb = zb_ModBusWordWriteHook(addr, 1);

    if (bb)
    {
        QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "zb_ModBusWordWriteHook sucess\n");
    }
    else
    {
        QL_LOG(QL_LOG_LEVEL_INFO, "[uart1_modbus_master]:", "zb_ModBusWordWriteHook fail\n");
    }
}
// uint16_t add_tlv(TLV *temp_tlv,uint8_t *txbuf)
// {
// static uint16_t distance1_last,distance2_last;
// uint8_t cnt = 0;
// if(temp_tlv->distance[0].is_effective)
// {
// 	if(abs(distance1_last - temp_tlv->distance[0].uwb.distance) >= flash_data.uwb_limit)//cm
// 	{
// 		cnt +=
// tlv_fill_data((txbuf+cnt),TAG_DISTANCE_1,&temp_tlv->distance[0].uwb,sizeof(temp_tlv->distance[0].uwb));
// 	}
// 	distance1_last = temp_tlv->distance[0].uwb.distance;
// }
// if(temp_tlv->distance[1].is_effective)
// {
// 	if(abs(distance2_last - temp_tlv->distance[1].uwb.distance) >= flash_data.uwb_limit)//cm
// 	{
// 		cnt +=
// tlv_fill_data((txbuf+cnt),TAG_DISTANCE_2,&temp_tlv->distance[1].uwb,sizeof(temp_tlv->distance[1].uwb));
// 	}
// 	distance2_last = temp_tlv->distance[1].uwb.distance;
// }
// if(temp_tlv->heartbeat.is_effective)
// {
// 	cnt +=
// tlv_fill_data((txbuf+cnt),TAG_HEARBEAT,&temp_tlv->heartbeat.heartbeat,sizeof(temp_tlv->heartbeat.heartbeat));
// }
// if(temp_tlv->shock.is_effective)
// {
// 	cnt +=
// tlv_fill_data((txbuf+cnt),TAG_SHOCK,&temp_tlv->shock.shock,sizeof(temp_tlv->shock.shock));
// }
// if(temp_tlv->voltage.is_effective)
// {
// 	cnt +=
// tlv_fill_data((txbuf+cnt),TAG_VOLTAGE,&temp_tlv->voltage.voltage,sizeof(temp_tlv->voltage.voltage));
// }

// cnt += tlv_fill_data((txbuf+cnt),TAG_MAC_ADDR,&mac_addr.addr,sizeof(mac_addr.addr));

// temp_tlv->distance[0].is_effective = false;
// temp_tlv->distance[1].is_effective = false;
// temp_tlv->heartbeat.is_effective = false;
// temp_tlv->shock.is_effective = false;
// temp_tlv->shock.shock.is_shock = false;
// temp_tlv->voltage.is_effective = false;
// return cnt;
// }
uint8_t slave = 1;
int ModbusSlaveProcess(uint8_t *txbuf, uint8_t *rxbuf, uint16_t rxLen, int is_crc)
{
    uint16_t crcData, crcChk;
    uint8_t Offset, ByteAdd;
    uint16_t ByteNumber;
    uint16_t add;
    uint16_t length;
    uint16_t temp_cnt;
    int out_len = 0;
    int i;
    uint8_t *pDat;
    uint8_t *pVar;
    uint16_t *pWordVar;
    uint8_t tlv_len = 0;
    if (rxLen < 4)
        return 0;
    if ((rxbuf[0] == slave) || (rxbuf[0] == 255))
    {
        if (is_crc != 0)
        {
            crcData = rxbuf[rxLen - 1] + (rxbuf[rxLen - 2] << 8);
            crcChk = crc16(rxbuf, rxLen - 2);
            if (crcData != crcChk)
            {
                return 0;
            }
        }

        add = ((uint16_t)rxbuf[2] << 8) + rxbuf[3];
        // if(Length !=
        length = (rxbuf[4] << 8) | rxbuf[5];
        txbuf[0] = rxbuf[0];
        switch (rxbuf[1])
        {
        case 0x01: // Read Coils
            if ((add + length) > gBIT_SIZE)
            {
                txbuf[1] = 0x81;
                txbuf[2] = 0x02; // ILLEGAL DATA ADDRESS
                out_len = 3;
                break;
            }
            txbuf[1] = 0x01;
            ByteNumber = (length + 7) / 8;
            txbuf[2] = ByteNumber;
            ByteAdd = add >> 3;  // add/8
            Offset = add & 0x07; // add%8
            for (i = 0; i < (ByteNumber - 1); i++)
            {
                txbuf[3 + i] = gBitVar[ByteAdd + i] >> Offset;
                txbuf[3 + i] |= gBitVar[ByteAdd + i + 1] << (8 - Offset);
            }
            txbuf[3 + ByteNumber - 1] = gBitVar[ByteAdd + ByteNumber - 1] >> Offset;
            txbuf[3 + ByteNumber - 1] &= 0xff >> Offset;
            out_len = ByteNumber + 3;
            break;
        case 0x03:
        case 0x04: // Read Holding Registers
            // esp32_ota_log(LINFO, "read addr:%d, length:%d", add, length);
            // esp32_ota_log(LINFO, "rssi:%d", gWordVar[REG_CSQ]);
            if ((add + length) > gWORD_SIZE)
            {
                txbuf[1] = 0x80 + rxbuf[1];
                txbuf[2] = 0x02; // ILLEGAL DATA ADDRESS
                out_len = 3;
                ;
                break;
            }
            if (length > 512)
            {
                length = 512;
            }
            if (rxbuf[1] == 4)
            {
                add += 64;
            }
            txbuf[1] = rxbuf[1];
            ByteNumber = length * 2;
            txbuf[2] = ByteNumber & 0xff;
            pDat = &txbuf[3];
            pWordVar = &gWordVar[add];
            for (i = 0; i < length; i++)
            {
                *pDat++ = *pWordVar >> 8;
                *pDat++ = *pWordVar & 0xff;
                pWordVar++;
            }
            out_len = ByteNumber + 3;
            break;
        case 0x05: // Write Single Coil
            if (add >= gBIT_SIZE)
            {
                txbuf[1] = 0x85;
                txbuf[2] = 0x02; // ILLEGAL DATA ADDRESS
                out_len = 3;
                break;
            }
            ByteAdd = add >> 3;  // same add/8
            Offset = add & 0x07; // same add%8
            if (rxbuf[4] == 0)
            {
                clrBit(add);
            }
            else if (rxbuf[4] == 0xff)
            {
                setBit(add);
            }
            else
            {
                txbuf[1] = 0x85;
                txbuf[2] = 0x03; // ILLEGAL DATA VALUE
                out_len = 3;
                break;
            }
            memcpy(txbuf, rxbuf, 6);
            ModBusBitWriteHook(add, 1);
            out_len = 6;
            break;
        case 0x06: // Write Single Register
            esp32_ota_log(LINFO, "write addr:%d, length:%d", add, 1);
            if (add >= gWORD_SIZE)
            {
                txbuf[1] = 0x86;
                txbuf[2] = 0x02; // ILLEGAL DATA ADDRESS
                out_len = 3;
                break;
            }
            gWordVar[add] = (rxbuf[4] << 8) + rxbuf[5];
            memcpy(txbuf, rxbuf, 6);
            ModBusWordWriteHook(add, 1);
            out_len = 6;
            break;
        case 0x0F: // Write Multiple Coil
            if ((add + length) > gBIT_SIZE)
            {
                txbuf[1] = 0x8F;
                txbuf[2] = 0x02; // ILLEGAL DATA ADDRESS
                out_len = 3;
            }
            txbuf[1] = 0x0F;
            txbuf[2] = rxbuf[2];
            txbuf[3] = rxbuf[3];
            txbuf[4] = 0;
            txbuf[5] = length & 0xff;
            pDat = rxbuf + 7;
            for (i = 0; i < length; i++)
            {
                if (*(pDat + (i >> 3)) & (1 << (i & 0x07)))
                    setBit(i + add);
                else
                    clrBit(i + add);
            }
            ModBusBitWriteHook(add, length);
            out_len = 6;
            break;
        case 0x10: // Write Multiple registers
            esp32_ota_log(LINFO, "write addr:%d, length:%d", add, length);
            if ((add + length) > gWORD_SIZE)
            {
                txbuf[1] = 0x90;
                txbuf[2] = 0x02; // ILLEGAL DATA ADDRESS
                crcChk = crc16(txbuf, 3);
                txbuf[3] = crcChk >> 8;
                txbuf[4] = crcChk;
                out_len = 3;
            }
            txbuf[1] = 0x10;
            txbuf[2] = rxbuf[2];
            txbuf[3] = rxbuf[3];
            txbuf[4] = 0;
            txbuf[5] = length & 0xff;
            pDat = rxbuf + 7;
            pWordVar = &gWordVar[add];
            for (i = 0; i < length; i++)
            {
                *pWordVar++ = (*pDat << 8) | *(pDat + 1);
                pDat += 2;
            }
            // esp32_ota_log(LINFO, "modbus write addr:%d, len=%d", add, length);
            ModBusWordWriteHook(add, length);
            out_len = 6;
            break;
        case 33:

            // txbuf[1] = 33;
            // txbuf[2] = add_tlv(&t_tlv,&txbuf[3]);
            // out_len  = txbuf[2]+3;
            // sys_para.normal_mode_para.gprs_timeout_cnt = 0;

            // if(DEBUG_FLG)
            // NRF_LOG_INFO("modbus ");
            break;
        } // switch(rxbuf[1])

        if (is_crc && out_len > 0)
        {
            crcChk = crc16(txbuf, out_len);
            txbuf[out_len++] = crcChk >> 8;
            txbuf[out_len++] = crcChk & 0xff;
        }
    }
    return out_len;
}
