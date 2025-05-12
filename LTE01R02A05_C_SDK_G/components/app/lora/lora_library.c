// #include <stdlib.h>
// #include <unistd.h>
// #include <stdio.h>
// #include <string.h>
// #include "ql_api_osi.h"
// #include "ql_api_spi.h"
// #include "ql_gpio.h"
// #include "ql_log.h"
// #include "lora_library.h"

// #define LOGI(msg, ...)  QL_LOG(QL_LOG_LEVEL_INFO, "lora_library", msg, ##__VA_ARGS__)

// ql_errcode_spi_e spi_error = 0;
// ql_errcode_gpio gpio_error = 0;
// unsigned char * tx_buf = NULL;
// unsigned char * tx_mal_buf = NULL;
// unsigned char * rx_buf = NULL;
// unsigned char * rx_mal_buf = NULL;
// unsigned short tx_len = 512;
// unsigned short rx_len = 512;

// static RadioOperatingModes_t OperatingMode;
// static RadioPacketTypes_t PacketType;
// static RadioPublicNetwork_t RadioPublicNetwork = { false };
// const FskBandwidth_t FskBandwidths[] =
// {
//     { 4800  , 0x1F },
//     { 5800  , 0x17 },
//     { 7300  , 0x0F },
//     { 9700  , 0x1E },
//     { 11700 , 0x16 },
//     { 14600 , 0x0E },
//     { 19500 , 0x1D },
//     { 23400 , 0x15 },
//     { 29300 , 0x0D },
//     { 39000 , 0x1C },
//     { 46900 , 0x14 },
//     { 58600 , 0x0C },
//     { 78200 , 0x1B },
//     { 93800 , 0x13 },
//     { 117300, 0x0B },
//     { 156200, 0x1A },
//     { 187200, 0x12 },
//     { 234300, 0x0A },
//     { 312000, 0x19 },
//     { 373600, 0x11 },
//     { 467000, 0x09 },
//     { 500000, 0x00 }, // Invalid Bandwidth
// };

// /// ////////////////////////////////////////////////////////////////////////////////////////////

// //等待busy引脚被从机置低
// int ql_wait_busy_low(void)
// {
//     int count = 1000;
//     ql_LvlMode level =2;
//     while(count--){
//         ql_gpio_get_level(GPIO_0, &level);
//         if(level == LVL_LOW){
//             return 1;
//         }
//     }
//     LOGI("ql_wait_busy_low timeout\n");
//     return 0;
// }

// /*发送cmd*/
// int ql_spi_write_command(uint8_t command, uint16_t data,uint8_t size)
// {
//     if(size == 1){ //传输一字节的数据
//         tx_buf[1] = data;
//     }else if(size == 2){
//         tx_buf[1] = (data & 0xFF00) >> 8;
//         tx_buf[2] = (data & 0xFF);
//     }

//     spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,size+1);
//     if(spi_error != 0){
//         LOGI("ql_spi_write_command fail,err = %d\n",spi_error);
//         return 0;
//     }

//     if(!ql_wait_busy_low()){
//         LOGI("ql_spi_write_command timeout,busy not low\n");
//         return 0;
//     }

//     return 1;
// }

// /*向寄存器发送数据*/
// int ql_spi_write_register_16bit(uint16_t address,uint16_t data)
// {
//     tx_buf[0] = RADIO_WRITE_REGISTER;
//     tx_buf[1] = ( address & 0xFF00 ) >> 8;
//     tx_buf[2] = ( address & 0x00FF );
//     tx_buf[3] = ( data & 0xFF00 ) >> 8;
//     tx_buf[4] = ( data & 0x00FF );
//     spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,5);
//     if(spi_error != 0){
//         LOGI("ql_spi_write_register fail,err = %d\n",spi_error);
//         return 0;
//     }
//     if(!ql_wait_busy_low()){
//         LOGI("ql_spi_write_register timeout,busy not low\n");
//         return 0;
//     }
//     return 1;
// }

// void ql_spi_write_register_8bit(uint16_t address,uint8_t data)
// {
//     tx_buf[0] = RADIO_WRITE_REGISTER;
//     tx_buf[1] = ( address & 0xFF00 ) >> 8;
//     tx_buf[2] = ( address & 0x00FF );
//     tx_buf[3] = data;
//     spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,4);
//     ql_wait_busy_low();
// }

// /*从寄存器读取数据*/
// int ql_spi_read_register(uint16_t address,uint16_t size)
// {
//     tx_buf[0] = RADIO_READ_REGISTER;
//     tx_buf[1] = ( address & 0xFF00 ) >> 8;
//     tx_buf[2] = ( address & 0x00FF );
//     spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,3);
//     if(spi_error != 0){
//         LOGI("ql_spi_read_register write cmd fail,err = %d\n",spi_error);
//         return 0;
//     }
//     spi_error = ql_spi_read(QL_SPI_PORT1,rx_buf,size);
//     if(spi_error != 0){
//         LOGI("ql_spi_read_register read data fail,err = %d\n",spi_error);
//         return 0;
//     }
//     if(!ql_wait_busy_low()){
//         LOGI("ql_spi_write_register timeout,busy not low\n");
//         return 0;
//     }
//     return 1;
// }
// /// ////////////////////////////////////////////////////////////////////////////////////////////

// //设置standby模式
// void lora_set_standby_mode(RadioStandbyModes_t mode)
// {
//     LOGI("enter lora_set_standby_mode");
//     tx_buf[0] = RADIO_SET_STANDBY;
//     tx_buf[1] = mode;
//     //spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,2);
//     spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,2);
//     if(spi_error != 0){
//         LOGI("set standby mode cmd fail, err = %d\n",spi_error);
//     }
//     ql_wait_busy_low();
//     if(mode == STDBY_RC){
//         OperatingMode = MODE_STDBY_RC;
//     }else if(mode == STDBY_XOSC){
//         OperatingMode = MODE_STDBY_XOSC;
//     }
    
// }

// //设置电源管理模式
// void lora_set_regulator_mode(RadioRegulatorMode_t mode)
// {
//     LOGI("enter lora_set_regulator_mode");
//     tx_buf[0] = RADIO_SET_REGULATORMODE;
//     tx_buf[1] = mode;
//     spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,2);
//     if(spi_error != 0){
//         LOGI("set regulator mode cmd fail.err = %d\n",spi_error);
//     }
//     ql_wait_busy_low();
// }

// void lora_set_buffer_base_address(uint8_t txBaseAddress, uint8_t rxBaseAddress)
// {
//     LOGI("enter lora_set_buffer_base_address");
//     tx_buf[0] = RADIO_SET_BUFFERBASEADDRESS;
//     tx_buf[1] = txBaseAddress;
//     tx_buf[2] = rxBaseAddress;
//     spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,3);
//     if(spi_error != 0){
//         LOGI("set buffer base address cmd fail.err = %d\n",spi_error);
//     }
//     ql_wait_busy_low();
// }

// void ql_lora_set_PaConfig(uint8_t paDutyCycle, uint8_t hpMax, uint8_t deviceSel, uint8_t paLut)
// {
//     LOGI("enter ql_lora_set_PaConfig");
//     tx_buf[0] = RADIO_SET_PACONFIG;
//     tx_buf[1] = paDutyCycle;
//     tx_buf[2] = hpMax;
//     tx_buf[3] = deviceSel;
//     tx_buf[4] = paLut;
//     spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,5);
//     if(spi_error != 0){
//         LOGI("set Paconfig cmd fail.err = %d\n",spi_error);
//     }
//     ql_wait_busy_low();
// }

// void ql_lora_set_TxParams(int8_t power, RadioRampTimes_t rampTime)
// {  
//     LOGI("enter ql_lora_set_TxParams");
//     ql_spi_read_register(0x08D8,1);
//     rx_buf[0] = rx_buf[0] |( 0x0F << 1 ) ;
//     ql_spi_write_register_8bit(0x08D8,rx_buf[0]);

//     ql_lora_set_PaConfig(0x04, 0x07, 0x00, 0x01);

//     if(power > 22) power = 22;
//     else if(power < -9) power = -9;
//     tx_buf[0] = RADIO_SET_TXPARAMS;
//     tx_buf[1] = power;
//     tx_buf[2] = rampTime;
//     spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,3);
//     if(spi_error != 0){
//         LOGI("set TxParams cmd fail.err = %d\n",spi_error);
//     }
//     ql_wait_busy_low();
// }

// void ql_lora_set_DioTrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask)
// {
//     LOGI("enter ql_lora_set_DioTrqParams");
//     tx_buf[0] = RADIO_CFG_DIOIRQ;
//     tx_buf[1] = ( uint8_t )( ( irqMask >> 8 ) & 0x00FF );
//     tx_buf[2] = ( uint8_t )( irqMask & 0x00FF );
//     tx_buf[3] = ( uint8_t )( ( dio1Mask >> 8 ) & 0x00FF );
//     tx_buf[4] = ( uint8_t )( dio1Mask & 0x00FF );
//     tx_buf[5] = ( uint8_t )( ( dio2Mask >> 8 ) & 0x00FF );
//     tx_buf[6] = ( uint8_t )( dio2Mask & 0x00FF );
//     tx_buf[7] = ( uint8_t )( ( dio3Mask >> 8 ) & 0x00FF );
//     tx_buf[8] = ( uint8_t )( dio3Mask & 0x00FF );
//     spi_error = ql_spi_write(QL_SPI_PORT1,tx_buf,9);
//     if(spi_error != 0){
//         LOGI("set DioTrqParams cmd fail.err = %d\n",spi_error);
//     }
//     ql_wait_busy_low();
// }

// uint8_t RadioGetFskBandwidthRegValue( uint32_t bandwidth )
// {
//     uint8_t i;

//     if( bandwidth == 0 )
//     {
//         return( 0x1F );
//     }

//     for( i = 0; i < ( sizeof( FskBandwidths ) / sizeof( FskBandwidth_t ) ) - 1; i++ )
//     {
//         if( ( bandwidth >= FskBandwidths[i].bandwidth ) && ( bandwidth < FskBandwidths[i + 1].bandwidth ) )
//         {
//             return FskBandwidths[i+1].RegValue;
//         }
//     }
//     // ERROR: Value not found
//     while( 1 );
// }

// void RadioSetModem( RadioModems_t modem )
// {
//     switch( modem )
//     {
//     default:
//     case MODEM_FSK:
//         //LLCC68SetPacketType( PACKET_TYPE_GFSK );
//         PacketType = PACKET_TYPE_GFSK;
//         tx_buf[0] = RADIO_SET_PACKETTYPE;
//         tx_buf[1] = PACKET_TYPE_GFSK;
//         ql_spi_write(QL_SPI_PORT1,tx_buf,2);
//         ql_wait_busy_low();

//         // When switching to GFSK mode the LoRa SyncWord register value is reset
//         // Thus, we also reset the RadioPublicNetwork variable
//         RadioPublicNetwork.Current = false;
//         break;
//     case MODEM_LORA:
//         //LLCC68SetPacketType( PACKET_TYPE_LORA );
//         PacketType = PACKET_TYPE_LORA;
//         tx_buf[0] = RADIO_SET_PACKETTYPE;
//         tx_buf[1] = PACKET_TYPE_LORA;
//         ql_spi_write(QL_SPI_PORT1,tx_buf,2);
//         ql_wait_busy_low();

//         // Public/Private network register is reset when switching modems
//         if( RadioPublicNetwork.Current != RadioPublicNetwork.Previous )
//         {
//             RadioPublicNetwork.Current = RadioPublicNetwork.Previous;
//             lora_RadioSetPublicNetwork( RadioPublicNetwork.Current );
//         }
//         break;
//     }
// }

// /// ////////////////////////////////////////////////////////////////////////////////////////////
// void dio1IrqCallback(void *param)
// {
//     LOGI("enter dio1IrqCallback");
// }

// void lora_gpio_init(void)
// {
//     LOGI("enter lora_gpio_init\n");
//     ql_pin_set_func(1,0x01);//clk
//     ql_pin_set_func(2,0x01);//miso di_1
//     ql_pin_set_func(3,0x01);//mosi dio_0
//     ql_pin_set_func(4,0x01); //ncs_pin

//     // //将busy，int，rst引脚设置为gpio功能
//     ql_pin_set_gpio(59);//rst_pin
//     ql_pin_set_gpio(60);//int_pin
//     gpio_error = ql_pin_set_gpio(61);//busy_pin
//     if(gpio_error != 0){
//         LOGI("set busy pin to gpio fail,err = %d\n",gpio_error);
//     }

//     //busy设置为上拉输入模式
//     gpio_error = ql_gpio_set_direction(GPIO_0,GPIO_INPUT);//busy_io_num
//     if(gpio_error != 0){
//         LOGI("set busy pin to input fail,err = %d\n",gpio_error);
//     }
//     gpio_error = ql_gpio_set_pull(GPIO_0,PULL_UP);
//     if(gpio_error != 0){
//         LOGI("set busy pin to PULL_UP fail,err = %d\n",gpio_error);
//     }

//     //int设置为中断引脚,下拉上升沿触发，启用防抖动
//     ql_gpio_set_direction(GPIO_3,GPIO_INPUT);//int_io_num
//     ql_gpio_set_pull(GPIO_3,PULL_DOWN);
//     gpio_error = ql_int_register(GPIO_3,EDGE_TRIGGER,DEBOUNCE_EN,EDGE_RISING,PULL_DOWN,dio1IrqCallback,NULL);//int_io_num
//     if(gpio_error != 0){
//         LOGI("ql_int_register fail,err = %d\n",gpio_error);
//     }

//     //cs设置为输出
//     ql_gpio_set_direction(GPIO_10,GPIO_OUTPUT);//ncs_io_num
// }

// void lora_spi_init(void)
// {
//     LOGI("enter lora_spi_init\n");
//     ql_spi_config_s lora_spi_config = {0};
//     lora_spi_config.input_mode =        QL_SPI_INPUT_TRUE;
//     lora_spi_config.port =              QL_SPI_PORT1;
//     lora_spi_config.framesize =         8;
//     lora_spi_config.spiclk =            QL_SPI_CLK_6_25MHZ;
//     lora_spi_config.cs_polarity0 =      QL_SPI_CS_ACTIVE_LOW;
//     lora_spi_config.cs_polarity1 =      QL_SPI_CS_ACTIVE_LOW;
//     lora_spi_config.cpol =              QL_SPI_CPOL_LOW;
//     lora_spi_config.cpha =              QL_SPI_CPHA_1Edge;
//     lora_spi_config.input_sel =         QL_SPI_DI_1;//?
//     lora_spi_config.transmode =         QL_SPI_DMA_IRQ;
//     lora_spi_config.cs =                QL_SPI_CS0;
    
//     lora_spi_config.clk_delay =         QL_SPI_CLK_DELAY_0;
//     lora_spi_config.release_flag =      QL_SPI_NOT_RELEASE;

//     spi_error = ql_spi_init_ext(lora_spi_config);
//     if(spi_error != QL_SPI_SUCCESS) {
//         LOGI("ql_spi_init_ext fail, err = %d\n",spi_error);
        
//     }
//     spi_error = ql_spi_cs_auto(QL_SPI_PORT1);
//     if(spi_error != QL_SPI_SUCCESS){
//         LOGI("ql_spi_cs_auto fail, err = %d\n",spi_error);
//     }

//     spi_error = ql_spi_request_sys_clk(QL_SPI_PORT1);
//     if(spi_error != QL_SPI_SUCCESS){
//         LOGI("ql_spi_request_sys_clk fail, err = %d\n",spi_error);
//     }
// }

// void lora_tx_rx_buf_init(void)
// {
//     LOGI("enter lora_tx_rx_buf_init\n");
//     tx_mal_buf = (unsigned char *)malloc(QL_SPI_DMA_ADDR_ALIN+tx_len);
//     rx_mal_buf = (unsigned char *)malloc(QL_SPI_DMA_ADDR_ALIN+rx_len);

//     //32对齐
//     tx_buf = (unsigned char *)OSI_ALIGN_UP(tx_mal_buf, QL_SPI_DMA_ADDR_ALIN);
//     rx_buf = (unsigned char *)OSI_ALIGN_UP(rx_mal_buf, QL_SPI_DMA_ADDR_ALIN);

//     //清零
//     memset(tx_buf, 0x00, tx_len);
//     memset(rx_buf, 0x00, rx_len);
// }
