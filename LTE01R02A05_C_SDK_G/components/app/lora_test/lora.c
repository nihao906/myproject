
#include "./inc/user.h"
#include "./inc/lora.h"

#include "ql_api_osi.h"
#include "ql_log.h"
#define LOGI(msg, ...)  QL_LOG(QL_LOG_LEVEL_INFO, "library", msg, ##__VA_ARGS__)

extern unsigned char * write_buf;
extern unsigned char * read_buf;
uint16_t syncword;
uint8_t PacketParams[6] = {0};

uint16_t lora_get_syncword(void)
{
    lora_read_register(LORA_SYNCWORD,2);
    uint16_t syncword = read_buf[4] << 8 | read_buf[5];
    return syncword;
}

void lora_set_standby(uint8_t data)
{
    lora_write_cmd(0x80,&data,1);
}

void lora_set_RfSwitchCtrl(uint8_t enable)
{
    lora_write_cmd(LORA_SET_RFSWITCHMODE,&enable,1);
}

void lora_set_calibrate(uint8_t param)
{
    lora_write_cmd(LORA_SET_CALIBRATE,&param,1);
}

void lora_set_regulator(uint8_t param)
{
    lora_write_cmd(LORA_SET_REGULATORMODE,&param,1);
}

void lora_set_bufferBaseAddress(uint8_t tx_base_add,uint8_t rx_base_add)
{
    uint8_t data[2] = {tx_base_add,rx_base_add};
    lora_write_cmd(LORA_SET_BUFFERBASEADDRESS,data,2);
}

void lora_set_PaConfig(uint8_t paDutyCycle,uint8_t hpMax,uint8_t deviceSel,uint8_t paLut)
{
    uint8_t data[4] = {paDutyCycle,hpMax,deviceSel,paLut};
    lora_write_cmd(LORA_SET_PACONFIG,data,4);
}

void lora_set_OverCurrentProtection(float max_current)
{
    if((max_current >= 0.0)&&(max_current <= 140.0)){
        uint8_t data = (uint8_t)(max_current*2/5);
        lora_wrtie_register(LORA_SET_OCP,&data,1);
    }
}

void lora_set_TX(uint8_t power,uint8_t rampTime)
{
    if(power > 22) power = 22;
    if(power < -3) power = -3;
    uint8_t data[2] = {power,rampTime};
    lora_write_cmd(LORA_SET_TX_PARAMS,data,2);
}

void lora_set_calibrateImage(uint32_t freq)
{
    uint8_t calFreq[2] = {0,0};
    if( freq > 900000000 ){
        calFreq[0] = 0xE1;
        calFreq[1] = 0xE9;
    }else if( freq > 850000000 ){
        calFreq[0] = 0xD7;
        calFreq[1] = 0xDB;
    }else if( freq > 770000000 ){
        calFreq[0] = 0xC1;
        calFreq[1] = 0xC5;
    }else if( freq > 460000000 ){
        calFreq[0] = 0x75;
        calFreq[1] = 0x81;
    }else if( freq > 425000000 ){
        calFreq[0] = 0x6B;
        calFreq[1] = 0x6F;
    }
    lora_write_cmd(LORA_SET_CALIBRATEIMAGE,calFreq,2);
}

void lora_set_RfFreq(uint32_t freq)
{
    lora_set_calibrateImage(freq);
    uint32_t frequence = (uint32_t)((double)freq/(double)FREQ_STEP);
    uint8_t data[4] = {0,0,0,0};
    data[0] = (uint8_t)((frequence>>24)&0xff);
    data[1] = (uint8_t)((frequence>>16)&0xff);
    data[2] = (uint8_t)((frequence>>8)&0xff);
    data[3] = (uint8_t)(frequence&0xff);
    lora_write_cmd(LORA_SET_RFFREQ,data,4);
}

void lora_stop_rxTimerRonpreamble(uint8_t enable)
{
    //取反
    enable = !enable;
    lora_write_cmd(LORA_SET_STOPRXTIMERONPREAMBLE,&enable,1);
}

void lora_set_symbNumTimeout(uint8_t symbNum)
{
    lora_write_cmd(LORA_SET_SYMBNUMTIMEOUT,&symbNum,1);
}

void lora_set_packetType(uint8_t type)
{
    lora_write_cmd(LORA_SET_PACKETTYPE,&type,1);
}

void lora_set_modulationParams(uint8_t spreadingFactor,uint8_t bandwidth,uint8_t codingRate,uint8_t lowDataRateOptimize)
{
    uint8_t data[4] = {spreadingFactor,bandwidth,codingRate,lowDataRateOptimize};
    lora_write_cmd(LORA_SET_MODULATIONPARAMS,data,4);
}

uint8_t* lora_get_packetParams(uint16_t preambleLength,uint8_t payloadLen,uint8_t crcOn,uint8_t invertIrq)
{
    
    PacketParams[0] = (preambleLength >> 8)&0xFF;
    PacketParams[1] = preambleLength;

    if(payloadLen){
        PacketParams[2] = 0x01;
        PacketParams[3] = payloadLen;
    }else{
        PacketParams[2] = 0x00;
        PacketParams[3] = 0xFF;
    }

    if(crcOn){
        PacketParams[4] = 0x01; //inverted
    }else{
        PacketParams[4] = 0x00; //standard
    }

    if(invertIrq){
        PacketParams[5] = 0x01;
    }else{
        PacketParams[5] = 0x00;
    }

    return PacketParams;
}

void lora_set_IQ_Polarity(uint8_t iqConfig)
{
    //获取当前寄存器数据
    lora_read_register(0x0736,1);
    uint8_t iqConfigCurrent = read_buf[0];

    if(iqConfig == 0x01){//inverted
        iqConfigCurrent &= 0xFB;
    }else{
        iqConfigCurrent |= 0x04; //standard
    }

    lora_wrtie_register(0x0736,&iqConfigCurrent,1);
}

void lora_set_packetParams(uint8_t * packetParams)
{
    lora_write_cmd(LORA_SET_PACKETPARAMS,packetParams,6);
}

void lora_set_DioRiqConfig(uint16_t irqMask, uint16_t dio1, uint16_t dio2, uint16_t dio3)
{
    uint8_t buf[8] = {0};
    buf[0] = (uint8_t)((irqMask >>8)&0x00FF);
    buf[1] = (uint8_t)(irqMask & 0x00FF);
    buf[2] = (uint8_t)((dio1 >>8)&0x00FF);
    buf[3] = (uint8_t)(dio1 & 0x00FF);
    buf[4] = (uint8_t)((dio2 >>8)&0x00FF);
    buf[5] = (uint8_t)(dio2 & 0x00FF);
    buf[6] = (uint8_t)((dio3 >>8)&0x00FF);
    buf[7] = (uint8_t)(dio3 & 0x00FF);
    lora_write_cmd(LORA_SET_DIO_IRQ_CONFIG,buf,8);
}

uint8_t lora_get_status(void)
{
    lora_read_cmd(LORA_GET_STATUS,1);
    return (uint8_t)read_buf[0];
}

int lora_set_Rx(uint32_t timeout)
{
    lora_set_standby(0x00);
    uint8_t buf[3] = {0};
    buf[0] = (uint8_t)((timeout >> 16) & 0xFF);
    buf[1] = (uint8_t)((timeout >> 8) & 0xFF);
    buf[2] = (uint8_t)(timeout & 0xFF);
    lora_write_cmd(LORA_SET_RX,buf,3);

    for(int i = 0; i < 10; i++){
        if((lora_get_status()&0x70) == 0x50){ //01110000,
            return 1;                         //01010000  所以状态为0x20
        }
        ql_delay_us(1000);
    }
    return 0;
}

uint16_t lora_get_irq_status(void)
{
    lora_read_cmd(LORA_GET_IRQSTATUS,3);
    return (uint16_t)((read_buf[2] << 8) | read_buf[3]);
}

void lora_clear_irq_status(uint16_t irq_status)
{
    uint8_t buf[2] = {0};
    buf[0] = (uint8_t)((irq_status >>8)&0x00FF);
    buf[1] = (uint8_t)(irq_status & 0x00FF);
    lora_write_cmd(LORA_CLEAR_IRQSTATUS,buf,2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int lora_init(void)
{
    transfer_buf_init();
    int err = lora_gpio_init();
    if(err != 1){
        LOGI("lora_gpio_init fail,err = %d\n",err);
        return 0;
    }
    err = lora_spi_init();
    if(err != 1){
        LOGI("lora_spi_init fail,err = %d\n",err);
        return 0;
    }
    lora_reset();
    return 1;
}

int lora_start(void)
{
    /*获取同步字并判断*/
    syncword = lora_get_syncword();
    if(syncword != LORA_SYNCWORD_1 && syncword != LORA_SYNCWORD_2){
        LOGI("syncword 0x%02X is error\n",syncword);
        return 0;
    }

    /*设置standby为RC模式*/
    lora_set_standby(LORA_STANDBY_RC);

    /*启用RF开关控制*/
    lora_set_RfSwitchCtrl(1);

    /*设置射频校准*/
    lora_set_calibrate(0b01111111);

    /*设置电源为DCDC模式*/
    lora_set_regulator(REGULATOR_DCDC);

    /*设置缓冲区基地址*/
    lora_set_bufferBaseAddress(0,0);

    /*配置功率放大器*/
    lora_set_PaConfig(0x04,0x07,0x00,0x01); //+22dBM

    /*设置过流保护*/
    lora_set_OverCurrentProtection(60.0);

    /*设置发送参数*/
    lora_set_TX(20,0x04);//默认20发送功率，渐变200us

    /*设置射频频率*/
    lora_set_RfFreq(RF_FREQ);

    return 1;
}

int lora_radioConfig(void)
{
    /*不禁用接收定时器*/
    lora_stop_rxTimerRonpreamble(0);
    
    /*设置超时参数*/
    lora_set_symbNumTimeout(0);

    /*设置数据包类型*/
    lora_set_packetType(0x01);//lora模式

    /*设置射频调制参数*/
    lora_set_modulationParams(7,4,1,0);

    /*配置数据包参数*/
    uint8_t *packetParams = lora_get_packetParams(8,0,1,0);

    /*设置射频接收IQ极性*/
    lora_set_IQ_Polarity(packetParams[5]);

    /*设置数据包参数*/
    lora_set_packetParams(packetParams);

    /*配置数字输入输出中断*/
    lora_set_DioRiqConfig(1023,2,0,0);//0b1111111111,0b0000000010
    
    /*设置接收模式为RX连续模式*/
    if(lora_set_Rx(0xFFFFFF) == 0) return 0;

    return 1;
}