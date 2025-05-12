/*!
 * \file      radio.c
 *
 * \brief     Radio driver API definition
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include <math.h>
#include <string.h>
//#include "delay.h"
#include "radio.h"
#include "LLCC68.h"
#include "llcc68_board.h"
#include "project_config.h"
#include "ql_api_osi.h"
#include "ql_api_spi.h"
#include "ql_log.h"
#define LOGI(msg, ...)  QL_LOG(QL_LOG_LEVEL_INFO, "lora_radio", msg, ##__VA_ARGS__)
/*!
 * Defines the time required for the TCXO to wakeup [ms].
 */
#define BOARD_TCXO_WAKEUP_TIME                      0

extern unsigned char * tx_buf;
extern unsigned char * rx_buf;
int IrqFired = 0;
/*!
 * \brief Initializes the radio
 *
 * \param [IN] events Structure containing the driver callback functions
 */
int RadioInit( RadioEvents_t *events );

/*!
 * Return current radio status
 *
 * \param status Radio status.[RF_IDLE, RF_RX_RUNNING, RF_TX_RUNNING]
 */
RadioState_t RadioGetStatus( void );

/*!
 * \brief Configures the radio with the given modem
 *
 * \param [IN] modem Modem to be used [0: FSK, 1: LoRa]
 */
void RadioSetModem( RadioModems_t modem );

/*!
 * \brief Sets the channel frequency
 *
 * \param [IN] freq         Channel RF frequency
 */
void RadioSetChannel( uint32_t freq );

/*!
 * \brief Checks if the channel is free for the given time
 *
 * \remark The FSK modem is always used for this task as we can select the Rx bandwidth at will.
 *
 * \param [IN] freq                Channel RF frequency in Hertz
 * \param [IN] rxBandwidth         Rx bandwidth in Hertz
 * \param [IN] rssiThresh          RSSI threshold in dBm
 * \param [IN] maxCarrierSenseTime Max time in milliseconds while the RSSI is measured
 *
 * \retval isFree         [true: Channel is free, false: Channel is not free]
 */
bool RadioIsChannelFree( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime );

/*!
 * \brief Generates a 32 bits random value based on the RSSI readings
 *
 * \remark This function sets the radio in LoRa modem mode and disables
 *         all interrupts.
 *         After calling this function either Radio.SetRxConfig or
 *         Radio.SetTxConfig functions must be called.
 *
 * \retval randomValue    32 bits random value
 */
uint32_t RadioRandom( void );

/*!
 * \brief Sets the reception parameters
 *
 * \param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
 * \param [IN] bandwidth    Sets the bandwidth
 *                          FSK : >= 2600 and <= 250000 Hz
 *                          LoRa: [0: 125 kHz, 1: 250 kHz,
 *                                 2: 500 kHz, 3: Reserved]
 * \param [IN] datarate     Sets the Datarate
 *                          FSK : 600..300000 bits/s
 *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
 *                                10: 1024, 11: 2048, 12: 4096  chips]
 * \param [IN] coderate     Sets the coding rate (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * \param [IN] bandwidthAfc Sets the AFC Bandwidth (FSK only)
 *                          FSK : >= 2600 and <= 250000 Hz
 *                          LoRa: N/A ( set to 0 )
 * \param [IN] preambleLen  Sets the Preamble length
 *                          FSK : Number of bytes
 *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
 * \param [IN] symbTimeout  Sets the RxSingle timeout value
 *                          FSK : timeout in number of bytes
 *                          LoRa: timeout in symbols
 * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
 * \param [IN] payloadLen   Sets payload length when fixed length is used
 * \param [IN] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
 * \param [IN] FreqHopOn    Enables disables the intra-packet frequency hopping
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: OFF, 1: ON]
 * \param [IN] HopPeriod    Number of symbols between each hop
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: Number of symbols
 * \param [IN] iqInverted   Inverts IQ signals (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: not inverted, 1: inverted]
 * \param [IN] rxContinuous Sets the reception in continuous mode
 *                          [false: single mode, true: continuous mode]
 */
void RadioSetRxConfig( RadioModems_t modem, uint32_t bandwidth,
                          uint32_t datarate, uint8_t coderate,
                          uint32_t bandwidthAfc, uint16_t preambleLen,
                          uint16_t symbTimeout, bool fixLen,
                          uint8_t payloadLen,
                          bool crcOn, bool FreqHopOn, uint8_t HopPeriod,
                          bool iqInverted, bool rxContinuous );

/*!
 * \brief Sets the transmission parameters
 *
 * \param [IN] modem        Radio modem to be used [0: FSK, 1: LoRa]
 * \param [IN] power        Sets the output power [dBm]
 * \param [IN] fdev         Sets the frequency deviation (FSK only)
 *                          FSK : [Hz]
 *                          LoRa: 0
 * \param [IN] bandwidth    Sets the bandwidth (LoRa only)
 *                          FSK : 0
 *                          LoRa: [0: 125 kHz, 1: 250 kHz,
 *                                 2: 500 kHz, 3: Reserved]
 * \param [IN] datarate     Sets the Datarate
 *                          FSK : 600..300000 bits/s
 *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
 *                                10: 1024, 11: 2048, 12: 4096  chips]
 * \param [IN] coderate     Sets the coding rate (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * \param [IN] preambleLen  Sets the preamble length
 *                          FSK : Number of bytes
 *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
 * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
 * \param [IN] crcOn        Enables disables the CRC [0: OFF, 1: ON]
 * \param [IN] FreqHopOn    Enables disables the intra-packet frequency hopping
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: OFF, 1: ON]
 * \param [IN] HopPeriod    Number of symbols between each hop
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: Number of symbols
 * \param [IN] iqInverted   Inverts IQ signals (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [0: not inverted, 1: inverted]
 * \param [IN] timeout      Transmission timeout [ms]
 */
void RadioSetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
                          uint32_t bandwidth, uint32_t datarate,
                          uint8_t coderate, uint16_t preambleLen,
                          bool fixLen, bool crcOn, bool FreqHopOn,
                          uint8_t HopPeriod, bool iqInverted, uint32_t timeout );

/*!
 * \brief Checks if the given RF frequency is supported by the hardware
 *
 * \param [IN] frequency RF frequency to be checked
 * \retval isSupported [true: supported, false: unsupported]
 */
bool RadioCheckRfFrequency( uint32_t frequency );

/*!
 * \brief Computes the packet time on air in ms for the given payload
 *
 * \Remark Can only be called once SetRxConfig or SetTxConfig have been called
 *
 * \param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
 * \param [IN] bandwidth    Sets the bandwidth
 *                          FSK : >= 2600 and <= 250000 Hz
 *                          LoRa: [0: 125 kHz, 1: 250 kHz,
 *                                 2: 500 kHz, 3: Reserved]
 * \param [IN] datarate     Sets the Datarate
 *                          FSK : 600..300000 bits/s
 *                          LoRa: [6: 64, 7: 128, 8: 256, 9: 512,
 *                                10: 1024, 11: 2048, 12: 4096  chips]
 * \param [IN] coderate     Sets the coding rate (LoRa only)
 *                          FSK : N/A ( set to 0 )
 *                          LoRa: [1: 4/5, 2: 4/6, 3: 4/7, 4: 4/8]
 * \param [IN] preambleLen  Sets the Preamble length
 *                          FSK : Number of bytes
 *                          LoRa: Length in symbols (the hardware adds 4 more symbols)
 * \param [IN] fixLen       Fixed length packets [0: variable, 1: fixed]
 * \param [IN] payloadLen   Sets payload length when fixed length is used
 * \param [IN] crcOn        Enables/Disables the CRC [0: OFF, 1: ON]
 *
 * \retval airTime        Computed airTime (ms) for the given packet payload length
 */
uint32_t RadioTimeOnAir( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn );

/*!
 * \brief Sends the buffer of size. Prepares the packet to be sent and sets
 *        the radio in transmission
 *
 * \param [IN]: buffer     Buffer pointer
 * \param [IN]: size       Buffer size
 */
void RadioSend( uint8_t *buffer, uint8_t size );

/*!
 * \brief Sets the radio in sleep mode
 */
void RadioSleep( void );

/*!
 * \brief Sets the radio in standby mode
 */
void RadioStandby( void );

/*!
 * \brief Sets the radio in reception mode for the given time
 * \param [IN] timeout Reception timeout [ms]
 *                     [0: continuous, others timeout]
 */
void RadioRx( uint32_t timeout );

/*!
 * \brief Start a Channel Activity Detection
 */
void RadioStartCad( void );

/*!
 * \brief Sets the radio in continuous wave transmission mode
 *
 * \param [IN]: freq       Channel RF frequency
 * \param [IN]: power      Sets the output power [dBm]
 * \param [IN]: time       Transmission mode timeout [s]
 */
void RadioSetTxContinuousWave( uint32_t freq, int8_t power, uint16_t time );

/*!
 * \brief Reads the current RSSI value
 *
 * \retval rssiValue Current RSSI value in [dBm]
 */
int16_t RadioRssi( RadioModems_t modem );

/*!
 * \brief Writes the radio register at the specified address
 *
 * \param [IN]: addr Register address
 * \param [IN]: data New register value
 */
void RadioWrite( uint32_t addr, uint8_t data );

/*!
 * \brief Reads the radio register at the specified address
 *
 * \param [IN]: addr Register address
 * \retval data Register value
 */
uint8_t RadioRead( uint32_t addr );

/*!
 * \brief Writes multiple radio registers starting at address
 *
 * \param [IN] addr   First Radio register address
 * \param [IN] buffer Buffer containing the new register's values
 * \param [IN] size   Number of registers to be written
 */
void RadioWriteBuffer( uint32_t addr, uint8_t *buffer, uint8_t size );

/*!
 * \brief Reads multiple radio registers starting at address
 *
 * \param [IN] addr First Radio register address
 * \param [OUT] buffer Buffer where to copy the registers data
 * \param [IN] size Number of registers to be read
 */
void RadioReadBuffer( uint32_t addr, uint8_t *buffer, uint8_t size );

/*!
 * \brief Sets the maximum payload length.
 *
 * \param [IN] modem      Radio modem to be used [0: FSK, 1: LoRa]
 * \param [IN] max        Maximum payload length in bytes
 */
void RadioSetMaxPayloadLength( RadioModems_t modem, uint8_t max );

/*!
 * \brief Sets the network to public or private. Updates the sync byte.
 *
 * \remark Applies to LoRa modem only
 *
 * \param [IN] enable if true, it enables a public network
 */
void RadioSetPublicNetwork( bool enable );

/*!
 * \brief Gets the time required for the board plus radio to get out of sleep.[ms]
 *
 * \retval time Radio plus board wakeup time in ms.
 */
uint32_t RadioGetWakeupTime( void );

/*!
 * \brief Process radio irq
 */
void RadioIrqProcess( void );

/*!
 * \brief Sets the radio in reception mode with Max LNA gain for the given time
 * \param [IN] timeout Reception timeout [ms]
 *                     [0: continuous, others timeout]
 */
void RadioRxBoosted( uint32_t timeout );

/*!
 * \brief Sets the Rx duty cycle management parameters
 *
 * \param [in]  rxTime        Structure describing reception timeout value
 * \param [in]  sleepTime     Structure describing sleep timeout value
 */
void RadioSetRxDutyCycle( uint32_t rxTime, uint32_t sleepTime );

/*!
 * Radio driver structure initialization
 */
const struct Radio_s Radio =
{
    .Init = RadioInit,
    .GetStatus = RadioGetStatus,
    .SetModem = RadioSetModem,
    .SetChannel = RadioSetChannel,
    .IsChannelFree = RadioIsChannelFree,
    .Random = RadioRandom,  
    .SetRxConfig = RadioSetRxConfig,
    .SetTxConfig = RadioSetTxConfig,
    .CheckRfFrequency = RadioCheckRfFrequency,
    .TimeOnAir = RadioTimeOnAir,
    .Send = RadioSend,
    .Sleep = RadioSleep,
    .Standby = RadioStandby,
    .Rx = RadioRx,
    .StartCad = RadioStartCad,
    .SetTxContinuousWave = RadioSetTxContinuousWave,
    .Rssi = RadioRssi,
    .Write = RadioWrite,
    .Read = RadioRead,
    .WriteBuffer = RadioWriteBuffer,
    .ReadBuffer = RadioReadBuffer,
    .SetMaxPayloadLength = RadioSetMaxPayloadLength,
    .SetPublicNetwork = RadioSetPublicNetwork,
    .GetWakeupTime = RadioGetWakeupTime,
    .IrqProcess = RadioIrqProcess,
    // Available on LLCC68 only
    .RxBoosted = RadioRxBoosted,
    .SetRxDutyCycle = RadioSetRxDutyCycle
};

/*
 * Local types definition
 */


 /*!
 * FSK bandwidth definition
 */
typedef struct
{
    uint32_t bandwidth;
    uint8_t  RegValue;
}FskBandwidth_t;

/*!
 * Precomputed FSK bandwidth registers values
 */
const FskBandwidth_t FskBandwidths[] =
{
    { 4800  , 0x1F },
    { 5800  , 0x17 },
    { 7300  , 0x0F },
    { 9700  , 0x1E },
    { 11700 , 0x16 },
    { 14600 , 0x0E },
    { 19500 , 0x1D },
    { 23400 , 0x15 },
    { 29300 , 0x0D },
    { 39000 , 0x1C },
    { 46900 , 0x14 },
    { 58600 , 0x0C },
    { 78200 , 0x1B },
    { 93800 , 0x13 },
    { 117300, 0x0B },
    { 156200, 0x1A },
    { 187200, 0x12 },
    { 234300, 0x0A },
    { 312000, 0x19 },
    { 373600, 0x11 },
    { 467000, 0x09 },
    { 500000, 0x00 }, // Invalid Bandwidth
};

const RadioLoRaBandwidths_t Bandwidths[] = { LORA_BW_125, LORA_BW_250, LORA_BW_500 };

uint8_t MaxPayloadLength = 0xFF;

uint32_t TxTimeout = 0;
uint32_t RxTimeout = 0;

bool RxContinuous = false;


PacketStatus_t RadioPktStatus;
uint8_t RadioRxPayload[255];



/*
 * LLCC68 DIO IRQ callback functions prototype
 */

/*!
 * \brief DIO 0 IRQ callback
 */
// void RadioOnDioIrq( void* context );
void RadioOnDioIrq( void);

/*
 * Private global variables
 */


/*!
 * Holds the current network type for the radio
 */
typedef struct
{
    bool Previous;
    bool Current;
}RadioPublicNetwork_t;

static RadioPublicNetwork_t RadioPublicNetwork = { false };

/*!
 * Radio callbacks variable
 */
static RadioEvents_t* RadioEvents;

/*
 * Public global variables
 */

/*!
 * Radio hardware and global parameters
 */
LLCC68_t LLCC68;

/*!
 * \brief Holds the internal operating mode of the radio
 */
static RadioOperatingModes_t OperatingMode;
ql_mutex_t mutex;

//------------------------ board code start 锟斤拷 board 锟狡讹拷锟斤拷锟斤拷锟侥达拷锟斤拷 -----------------
RadioOperatingModes_t LLCC68GetOperatingMode( void )
{
    return OperatingMode;
}

int LLCC68SetOperatingMode( RadioOperatingModes_t mode )
{
    OperatingMode = mode;
    return 1;
}

void LLCC68IoTcxoInit( void )
{
    // No TCXO component available on this board design.
}

uint32_t LLCC68GetBoardTcxoWakeupTime( void )
{
    return BOARD_TCXO_WAKEUP_TIME;
}


//锟斤拷锟斤拷DIO2位RF锟斤拷锟狡斤拷
void LLCC68IoRfSwitchInit( void )
{
    LLCC68SetDio2AsRfSwitchCtrl( true );
}

//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟节诧拷锟皆讹拷锟斤拷锟狡ｏ拷锟斤拷锟斤拷专锟斤拷锟斤拷锟斤拷
int LLCC68AntSwOn( void )
{
    //GpioInit( &AntPow, RADIO_ANT_SWITCH_POWER, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 1 );
    return 1;
}
//锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟节诧拷锟皆讹拷锟斤拷锟狡ｏ拷锟斤拷锟斤拷专锟斤拷锟斤拷锟斤拷
void LLCC68AntSwOff( void )
{
    //GpioInit( &AntPow, RADIO_ANT_SWITCH_POWER, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0 );
}

uint8_t LLCC68GetDeviceId( void )
{
    return SX1262;
}

void LLCC68SetRfTxPower( int8_t power )
{
    LLCC68SetTxParams( power, RADIO_RAMP_40_US );
}

int LLCC68WriteRegister( uint16_t address, uint8_t value )
{
    return LLCC68WriteRegisters( address, &value, 1 );
}

uint8_t LLCC68ReadRegister( uint16_t address )
{
    //uint8_t data;
    LLCC68ReadRegisters( address, rx_buf, 1 );
    //rx_buf[0] = 2;
    return rx_buf[0];
}

int LLCC68Wakeup( void )
{
	QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68Wakeup start");
    int flag = -1;

    //ql_rtos_mutex_create(&mutex);
    //LOGI("ql_rtos_mutex_create end\n");

	// flag = LLCC68SetNss(0);
    // if(flag == -1){
    //     //LOGI("LLCC68SetNss 0 fail\n");
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68SetNss 0 fail");
    //     return flag;
    // }else{
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68SetNss 0 ok");
    // }

	// flag = LLCC68SpiInOut(RADIO_GET_STATUS );
    // if(flag == -1){
    //     //LOGI("LLCC68SpiInOut RADIO_GET_STATUS fail\n");
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68SpiInOut RADIO_GET_STATUS fail");
    //     return flag;
    // }else{
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68SpiInOut RADIO_GET_STATUS ok");
    // }


	// flag = LLCC68SpiInOut(0x00 );
    // if(flag == -1){
    //     //LOGI("LLCC68SpiInOut 0x00 fail\n");
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68SpiInOut 0x00 fail");
    //     return flag;
    // }else{
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68SpiInOut 0x00 ok");
    // }

    tx_buf[0] = RADIO_GET_STATUS;
    tx_buf[1] = 0x00;
    ql_spi_write(QL_SPI_PORT1,tx_buf,2);
	// flag = LLCC68SetNss(1);
    // if(flag == -1){
    //     //LOGI("LLCC68SetNss 1 fail\n");
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68SetNss 1 fail");
    //     return flag;
    // }else{
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68SetNss 1 ok");
    // }

	// Wait for chip to be ready.
	flag = LLCC68WaitOnBusy(); //error:没有返回值
    if(flag == -1){
        //LOGI("LLCC68WaitOnBusy timeout\n");
        QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68WaitOnBusy  fail");
        return flag;
    }else{
        QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68WaitOnBusy ok");
    }
    

	// Update operating mode context variable
	flag = LLCC68SetOperatingMode( MODE_STDBY_RC );
    // if(flag == -1){
    //     //LOGI("LLCC68SetOperatingMode MODE_STDBY_RC error\n");
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68SetOperatingMode MODE_STDBY_RC error");
    // }else{
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68SetOperatingMode MODE_STDBY_RC ok");
    // }
    //LOGI("LLCC68SetOperatingMode MODE_STDBY_RC end\n");

	//CRITICAL_SECTION_END( );
    //ql_rtos_mutex_delete(mutex);
    //QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68Wakeup","LLCC68Wakeup end,flag = %d\n",flag);
    return 1;
}

int LLCC68WriteCommand( RadioCommands_t command, uint8_t *buffer, uint16_t size )
{
    QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68WriteCommand","LLCC68WriteCommand start");
    //int flag = -1;
	uint16_t i = 0;
	
	// flag = LLCC68CheckDeviceReady(); //error
    // if(flag == -1){
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68WriteCommand","LLCC68CheckDeviceReady fail");
    //     return flag;
    // }
    

	// flag = LLCC68SetNss(0);
    // if(flag == -1){
    //     //LOGI("LLCC68SetNss 0 fail\n");
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68WriteCommand","LLCC68SetNss 0 fail");
    //     return flag;
    // }
    

	// flag = LLCC68SpiInOut(( uint8_t )command);
    // if(flag == -1){
    //     //LOGI("LLCC68SpiInOut command fail\n");
    //     QL_LOG(QL_LOG_LEVEL_INFO,"inLLCC68WriteCommand","LLCC68SpiInOut command fail");
    //     return flag;
    // }
    

	// for( i = 0; i < size; i++ ){
	// 	flag = LLCC68SpiInOut(buffer[i]);
    //     if(flag == -1){
    //         //LOGI("LLCC68SpiInOut buffer fail\n");
    //         QL_LOG(QL_LOG_LEVEL_INFO,"LLCC68WriteCommand","LLCC68SpiInOut buffer fail");
    //         return flag;
    //     }
	// }
    tx_buf[0] = (uint8_t)command;
    for( i = 0; i < size; i++ ){ 
        tx_buf[i+1] = buffer[i];
    }
    ql_spi_write(QL_SPI_PORT1,tx_buf,size+1);
    

	// flag = LLCC68SetNss(1);
    // if(flag == -1){
    //     //LOGI("LLCC68SetNss 1 fail\n");
    //     QL_LOG(QL_LOG_LEVEL_INFO,"LLCC68WriteCommand","LLCC68SetNss 1 fail");
    //     return flag;
    // }

	if( command != RADIO_SET_SLEEP ){
		LLCC68WaitOnBusy();
    }

    QL_LOG(QL_LOG_LEVEL_INFO,"LLCC68WriteCommand","LLCC68WriteCommand end");
    return 1;
}

void LLCC68ReadCommand( RadioCommands_t command, unsigned char *rx_buf, uint16_t size )
{
	//uint8_t status = 0;
	//uint16_t i = 0;
	
	// LLCC68CheckDeviceReady( );

	// LLCC68SetNss(0);

	// LLCC68SpiInOut(( uint8_t )command);
	// status = LLCC68SpiInOut(0x00);
	// for( i = 0; i < size; i++ ){
	// 	buffer[i] = LLCC68SpiInOut(0);
	// }

    tx_buf[0] = (uint8_t)command;
    ql_spi_write(QL_SPI_PORT1,tx_buf,1);
    ql_spi_read(QL_SPI_PORT1,rx_buf,size);
	// LLCC68SetNss(1);

	LLCC68WaitOnBusy( );

	//return status;
}

int LLCC68WriteRegisters( uint16_t address, uint8_t *buffer, uint16_t size )
{
	uint16_t i = 0;
	int flag = -1;
	// flag = LLCC68CheckDeviceReady();
    // if(flag == -1){
    //     LOGI("LLCC68CheckDeviceReady fail\n");
    //     return flag;
    // }

	// flag = LLCC68SetNss(0);
    // if(flag == -1){
    //     LOGI("LLCC68SetNss 0 fail\n");
    //     return flag;
    // }
    
	// flag = LLCC68SpiInOut(RADIO_WRITE_REGISTER );
    // if(flag == -1){
    //     LOGI("LLCC68SpiInOut RADIO_WRITE_REGISTER fail\n");
    //     return flag;
    // }
	// flag = LLCC68SpiInOut(( address & 0xFF00 ) >> 8 );
    // if(flag == -1){
    //     LOGI("LLCC68SpiInOut address & 0xFF00 fail\n");
    //     return flag;
    // }
	// flag = LLCC68SpiInOut( address & 0x00FF );
    // if(flag == -1){
    //     LOGI("LLCC68SpiInOut address & 0x00FF fail\n");
    //     return flag;
    // }
    
	// for( i = 0; i < size; i++ ){
	// 	flag = LLCC68SpiInOut(buffer[i] );
    //     if(flag == -1){
    //         LOGI("LLCC68SpiInOut buffer fail\n");
    //         return flag;
    //     }
	// }
    tx_buf[0] = RADIO_WRITE_REGISTER;
	tx_buf[1] = ( address & 0xFF00 ) >> 8;
	tx_buf[2] = address & 0x00FF;
	for( i = 0; i < size; i++ ){    
        tx_buf[i+3] = buffer[i];
    }
    ql_spi_write(QL_SPI_PORT1,tx_buf,size+3);
	// flag = LLCC68SetNss(1);
    // if(flag == -1){
    //         LOGI("LLCC68SetNss 1 fail\n");
    //         return flag;
    //     }

	flag = LLCC68WaitOnBusy( );
    if(flag == -1){
            LOGI("LLCC68WaitOnBusy fail\n");
            return flag;
        }

    return flag;
}

void LLCC68ReadRegisters( uint16_t address, unsigned char *rx_buf, uint16_t size )
{
	//uint16_t i = 0;
	
	// LLCC68CheckDeviceReady( );
	// LLCC68SetNss(0);
	
	// LLCC68SpiInOut(RADIO_READ_REGISTER);
	// LLCC68SpiInOut(( address & 0xFF00 ) >> 8);
	// LLCC68SpiInOut(address & 0x00FF);
	// LLCC68SpiInOut(0);
	
	// for( i = 0; i < size; i++ ){
	// 	buffer[i] = LLCC68SpiInOut(0);
	// }
    tx_buf[0] = RADIO_READ_REGISTER;
    tx_buf[1] = ( address & 0xFF00 ) >> 8;
    tx_buf[2] = address & 0x00FF;
    tx_buf[3] = 0;
    ql_spi_write(QL_SPI_PORT1,tx_buf,4);
    ql_spi_read(QL_SPI_PORT1,rx_buf,size);
	// LLCC68SetNss(1);
    rx_buf[0] = 0x12;
    rx_buf[1] = 0x34;
	LLCC68WaitOnBusy( );
}

void LLCC68WriteBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
	uint16_t i = 0;
	
	//LLCC68CheckDeviceReady( );

	//LLCC68SetNss(0);

	// LLCC68SpiInOut(RADIO_WRITE_BUFFER );
	// LLCC68SpiInOut(offset );
	// for( i = 0; i < size; i++ ){
	// 	LLCC68SpiInOut(buffer[i] );
	// }
    tx_buf[0] = RADIO_WRITE_BUFFER;
    tx_buf[1] = offset;
    for( i = 0; i < size; i++ ){
        tx_buf[2+i] = buffer[i];
    }
    ql_spi_write(QL_SPI_PORT1,tx_buf,2+size);
	//LLCC68SetNss(1);

	LLCC68WaitOnBusy( );
}

void LLCC68ReadBuffer( uint8_t offset, uint8_t *buffer, uint8_t size )
{
	//uint16_t i = 0;
	
	//LLCC68CheckDeviceReady( );

	//LLCC68SetNss(0);

	// LLCC68SpiInOut(RADIO_READ_BUFFER );
	// LLCC68SpiInOut(offset );
	// LLCC68SpiInOut(0 );
	// for( i = 0; i < size; i++ ){
	// 	buffer[i] = LLCC68SpiInOut(0);
	// }
    tx_buf[0] = RADIO_READ_BUFFER;
    tx_buf[1] = offset;
    tx_buf[2] = 0;
    ql_spi_write(QL_SPI_PORT1,tx_buf,3);
    ql_spi_read(QL_SPI_PORT1,rx_buf,size);
	//LLCC68SetNss(1);

	LLCC68WaitOnBusy( );
}
//------------------------ board code end -----------------

/*!
 * Returns the known FSK bandwidth registers value
 *
 * \param [IN] bandwidth Bandwidth value in Hz
 * \retval regValue Bandwidth register value.
 */
static uint8_t RadioGetFskBandwidthRegValue( uint32_t bandwidth )
{
    uint8_t i;

    if( bandwidth == 0 )
    {
        return( 0x1F );
    }

    for( i = 0; i < ( sizeof( FskBandwidths ) / sizeof( FskBandwidth_t ) ) - 1; i++ )
    {
        if( ( bandwidth >= FskBandwidths[i].bandwidth ) && ( bandwidth < FskBandwidths[i + 1].bandwidth ) )
        {
            return FskBandwidths[i+1].RegValue;
        }
    }
    // ERROR: Value not found
    while( 1 );
}

int RadioInit( RadioEvents_t *events )
{
    //LOGI("enter RadioInit\n");
    int flag = -1;
    RadioEvents = events;

    LLCC68IoInit();

#if 1
    //LLCC68Init( RadioOnDioIrq );//??
    RadioOnDioIrq();
    LLCC68Init();
    //LOGI("LLCC68Init end\n");

    flag = LLCC68SetStandby( STDBY_RC ); //ERROR
    if(flag == -1){
        //LOGI("LLCC68SetStandby fail\n");
        QL_LOG(QL_LOG_LEVEL_INFO,"inRadioInit","LLCC68SetStandby fail");
    }else{
        //LOGI("LLCC68SetStandby ok\n");
        QL_LOG(QL_LOG_LEVEL_INFO,"inRadioInit","LLCC68SetStandby ok");
    }
    

    flag = LLCC68SetRegulatorMode( USE_DCDC );
    if(flag == -1){
        LOGI("LLCC68SetRegulatorMode USE_DCDC fail\n");
    }

    flag = LLCC68SetBufferBaseAddress( 0x00, 0x00 );
    if(flag == -1){
        LOGI("LLCC68SetBufferBaseAddress 0x00, 0x00 fail\n");
    }

    flag = LLCC68SetTxParams( 0, RADIO_RAMP_200_US );
    if(flag == -1){
        LOGI("LLCC68SetTxParams 0, RADIO_RAMP_200_US fail\n");
    }

    flag = LLCC68SetDioIrqParams( IRQ_RADIO_ALL, IRQ_RADIO_ALL, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
    if(flag == -1){
        LOGI("LLCC68SetDioIrqParams  fail\n");
    }

    LLCC68TimerInit();
#endif
    IrqFired = 0;
    return 1;
}

RadioState_t RadioGetStatus( void )
{
    switch( LLCC68GetOperatingMode( ) )
    {
        case MODE_TX:
            return RF_TX_RUNNING;
        case MODE_RX:
            return RF_RX_RUNNING;
        case MODE_CAD:
            return RF_CAD;
        default:
            return RF_IDLE;
    }
}

void RadioSetModem( RadioModems_t modem )
{
    switch( modem )
    {
    default:
    case MODEM_FSK:
        LLCC68SetPacketType( PACKET_TYPE_GFSK );
        // When switching to GFSK mode the LoRa SyncWord register value is reset
        // Thus, we also reset the RadioPublicNetwork variable
        RadioPublicNetwork.Current = false;
        break;
    case MODEM_LORA:
        LLCC68SetPacketType( PACKET_TYPE_LORA );
        // Public/Private network register is reset when switching modems
        if( RadioPublicNetwork.Current != RadioPublicNetwork.Previous )
        {
            RadioPublicNetwork.Current = RadioPublicNetwork.Previous;
            RadioSetPublicNetwork( RadioPublicNetwork.Current );
        }
        break;
    }
}

void RadioSetChannel( uint32_t freq )
{
    LLCC68SetRfFrequency( freq );
}

bool RadioIsChannelFree( uint32_t freq, uint32_t rxBandwidth, int16_t rssiThresh, uint32_t maxCarrierSenseTime )
{
    bool     status           = true;
    int16_t  rssi             = 0;
    uint32_t count = 0;

    RadioSetModem( MODEM_FSK );

    RadioSetChannel( freq );

    // Set Rx bandwidth. Other parameters are not used.
    RadioSetRxConfig( MODEM_FSK, rxBandwidth, 600, 0, rxBandwidth, 3, 0, false,
                      0, false, 0, 0, false, true );
    RadioRx( 0 );

    LLCC68DelayMs( 1 );

    // Perform carrier sense for maxCarrierSenseTime
		count = 0;
    while( count < maxCarrierSenseTime )
    {
        rssi = RadioRssi( MODEM_FSK );

        if( rssi > rssiThresh )
        {
            status = false;
            break;
        }
				LLCC68DelayMs( 1 );
				count++;
    }
    RadioSleep( );
    return status;
}

uint32_t RadioRandom( void )
{
    uint32_t rnd = 0;

    /*
     * Radio setup for random number generation
     */
    // Set LoRa modem ON
    RadioSetModem( MODEM_LORA );

    // Disable LoRa modem interrupts
    LLCC68SetDioIrqParams( IRQ_RADIO_NONE, IRQ_RADIO_NONE, IRQ_RADIO_NONE, IRQ_RADIO_NONE );

    rnd = LLCC68GetRandom( );

    return rnd;
}

void RadioSetRxConfig( RadioModems_t modem, uint32_t bandwidth,
                         uint32_t datarate, uint8_t coderate,
                         uint32_t bandwidthAfc, uint16_t preambleLen,
                         uint16_t symbTimeout, bool fixLen,
                         uint8_t payloadLen,
                         bool crcOn, bool freqHopOn, uint8_t hopPeriod,
                         bool iqInverted, bool rxContinuous )
{
    uint8_t syncWordBuf[]={ 0xC1, 0x94, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00 };
		
    RxContinuous = rxContinuous;
    if( rxContinuous == true )
    {
        symbTimeout = 0;
    }
    if( fixLen == true )
    {
        MaxPayloadLength = payloadLen;
    }
    else
    {
        MaxPayloadLength = 0xFF;
    }

    switch( modem )
    {
        case MODEM_FSK:
            LLCC68SetStopRxTimerOnPreambleDetect( false );
            LLCC68.ModulationParams.PacketType = PACKET_TYPE_GFSK;

            LLCC68.ModulationParams.Params.Gfsk.BitRate = datarate;
            LLCC68.ModulationParams.Params.Gfsk.ModulationShaping = MOD_SHAPING_G_BT_1;
            LLCC68.ModulationParams.Params.Gfsk.Bandwidth = RadioGetFskBandwidthRegValue( bandwidth << 1 ); // LLCC68 badwidth is double sided

            LLCC68.PacketParams.PacketType = PACKET_TYPE_GFSK;
            LLCC68.PacketParams.Params.Gfsk.PreambleLength = ( preambleLen << 3 ); // convert byte into bit
            LLCC68.PacketParams.Params.Gfsk.PreambleMinDetect = RADIO_PREAMBLE_DETECTOR_08_BITS;
            LLCC68.PacketParams.Params.Gfsk.SyncWordLength = 3 << 3; // convert byte into bit
            LLCC68.PacketParams.Params.Gfsk.AddrComp = RADIO_ADDRESSCOMP_FILT_OFF;
            LLCC68.PacketParams.Params.Gfsk.HeaderType = ( fixLen == true ) ? RADIO_PACKET_FIXED_LENGTH : RADIO_PACKET_VARIABLE_LENGTH;
            LLCC68.PacketParams.Params.Gfsk.PayloadLength = MaxPayloadLength;
            if( crcOn == true )
            {
                LLCC68.PacketParams.Params.Gfsk.CrcLength = RADIO_CRC_2_BYTES_CCIT;
            }
            else
            {
                LLCC68.PacketParams.Params.Gfsk.CrcLength = RADIO_CRC_OFF;
            }
            LLCC68.PacketParams.Params.Gfsk.DcFree = RADIO_DC_FREEWHITENING;

            RadioStandby( );
            RadioSetModem( ( LLCC68.ModulationParams.PacketType == PACKET_TYPE_GFSK ) ? MODEM_FSK : MODEM_LORA );
            LLCC68SetModulationParams( &LLCC68.ModulationParams );
            LLCC68SetPacketParams( &LLCC68.PacketParams );
            LLCC68SetSyncWord( syncWordBuf );
            LLCC68SetWhiteningSeed( 0x01FF );

            RxTimeout = ( uint32_t )symbTimeout * 8000UL / datarate;
            break;

        case MODEM_LORA:
            LLCC68SetStopRxTimerOnPreambleDetect( false );
            LLCC68.ModulationParams.PacketType = PACKET_TYPE_LORA;
            LLCC68.ModulationParams.Params.LoRa.SpreadingFactor = ( RadioLoRaSpreadingFactors_t )datarate;
            LLCC68.ModulationParams.Params.LoRa.Bandwidth = Bandwidths[bandwidth];
            LLCC68.ModulationParams.Params.LoRa.CodingRate = ( RadioLoRaCodingRates_t )coderate;

            if( ( ( bandwidth == 0 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
            ( ( bandwidth == 1 ) && ( datarate == 12 ) ) )
            {
                LLCC68.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x01;
            }
            else
            {
                LLCC68.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x00;
            }

            LLCC68.PacketParams.PacketType = PACKET_TYPE_LORA;

            if( ( LLCC68.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF5 ) ||
                ( LLCC68.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF6 ) )
            {
                if( preambleLen < 12 )
                {
                    LLCC68.PacketParams.Params.LoRa.PreambleLength = 12;
                }
                else
                {
                    LLCC68.PacketParams.Params.LoRa.PreambleLength = preambleLen;
                }
            }
            else
            {
                LLCC68.PacketParams.Params.LoRa.PreambleLength = preambleLen;
            }

            LLCC68.PacketParams.Params.LoRa.HeaderType = ( RadioLoRaPacketLengthsMode_t )fixLen;

            LLCC68.PacketParams.Params.LoRa.PayloadLength = MaxPayloadLength;
            LLCC68.PacketParams.Params.LoRa.CrcMode = ( RadioLoRaCrcModes_t )crcOn;
            LLCC68.PacketParams.Params.LoRa.InvertIQ = ( RadioLoRaIQModes_t )iqInverted;

            RadioStandby( );
            RadioSetModem( ( LLCC68.ModulationParams.PacketType == PACKET_TYPE_GFSK ) ? MODEM_FSK : MODEM_LORA );
            LLCC68SetModulationParams( &LLCC68.ModulationParams );
            LLCC68SetPacketParams( &LLCC68.PacketParams );
            LLCC68SetLoRaSymbNumTimeout( symbTimeout );

            // WORKAROUND - Optimizing the Inverted IQ Operation, see DS_SX1261-2_V1.2 datasheet chapter 15.4
            if( LLCC68.PacketParams.Params.LoRa.InvertIQ == LORA_IQ_INVERTED )
            {
                // RegIqPolaritySetup = @address 0x0736
                LLCC68WriteRegister( 0x0736, LLCC68ReadRegister( 0x0736 ) & ~( 1 << 2 ) );
            }
            else
            {
                // RegIqPolaritySetup @address 0x0736
                LLCC68WriteRegister( 0x0736, LLCC68ReadRegister( 0x0736 ) | ( 1 << 2 ) );
            }
            // WORKAROUND END

            // Timeout Max, Timeout handled directly in SetRx function
            RxTimeout = 0xFFFF;

            break;
    }
}

void RadioSetTxConfig( RadioModems_t modem, int8_t power, uint32_t fdev,
                        uint32_t bandwidth, uint32_t datarate,
                        uint8_t coderate, uint16_t preambleLen,
                        bool fixLen, bool crcOn, bool freqHopOn,
                        uint8_t hopPeriod, bool iqInverted, uint32_t timeout )
{
    LOGI("enter RadioSetTxConfig\n");
    uint8_t syncWordBuf[] = { 0xC1, 0x94, 0xC1, 0x00, 0x00, 0x00, 0x00, 0x00 };
		
    switch( modem )
    {
        case MODEM_FSK:
            {LLCC68.ModulationParams.PacketType = PACKET_TYPE_GFSK;
            LLCC68.ModulationParams.Params.Gfsk.BitRate = datarate;

            LLCC68.ModulationParams.Params.Gfsk.ModulationShaping = MOD_SHAPING_G_BT_1;
            LLCC68.ModulationParams.Params.Gfsk.Bandwidth = RadioGetFskBandwidthRegValue( bandwidth << 1 ); // LLCC68 badwidth is double sided
            LLCC68.ModulationParams.Params.Gfsk.Fdev = fdev;

            LLCC68.PacketParams.PacketType = PACKET_TYPE_GFSK;
            LLCC68.PacketParams.Params.Gfsk.PreambleLength = ( preambleLen << 3 ); // convert byte into bit
            LLCC68.PacketParams.Params.Gfsk.PreambleMinDetect = RADIO_PREAMBLE_DETECTOR_08_BITS;
            LLCC68.PacketParams.Params.Gfsk.SyncWordLength = 3 << 3 ; // convert byte into bit
            LLCC68.PacketParams.Params.Gfsk.AddrComp = RADIO_ADDRESSCOMP_FILT_OFF;
            LLCC68.PacketParams.Params.Gfsk.HeaderType = ( fixLen == true ) ? RADIO_PACKET_FIXED_LENGTH : RADIO_PACKET_VARIABLE_LENGTH;

            if( crcOn == true )
            {
                LLCC68.PacketParams.Params.Gfsk.CrcLength = RADIO_CRC_2_BYTES_CCIT;
            }
            else
            {
                LLCC68.PacketParams.Params.Gfsk.CrcLength = RADIO_CRC_OFF;
            }
            LLCC68.PacketParams.Params.Gfsk.DcFree = RADIO_DC_FREEWHITENING;

            RadioStandby();
            LOGI("RadioStandby end");
            RadioSetModem( ( LLCC68.ModulationParams.PacketType == PACKET_TYPE_GFSK ) ? MODEM_FSK : MODEM_LORA );
            LOGI("RadioSetModem end");
            LLCC68SetModulationParams( &LLCC68.ModulationParams );
            LOGI("LLCC68SetModulationParams end");
            LLCC68SetPacketParams( &LLCC68.PacketParams );
            LOGI("LLCC68SetPacketParams end");
            LLCC68SetSyncWord( syncWordBuf );
            LOGI("LLCC68SetSyncWord end");
            LLCC68SetWhiteningSeed( 0x01FF );}
            LOGI("LLCC68SetWhiteningSeed end");
            break;

        case MODEM_LORA:
            {
            LLCC68.ModulationParams.PacketType = PACKET_TYPE_LORA;
            LLCC68.ModulationParams.Params.LoRa.SpreadingFactor = ( RadioLoRaSpreadingFactors_t ) datarate;
            LLCC68.ModulationParams.Params.LoRa.Bandwidth =  Bandwidths[bandwidth];
            LLCC68.ModulationParams.Params.LoRa.CodingRate= ( RadioLoRaCodingRates_t )coderate;

            if( 
                ( ( bandwidth == 0 ) && ( ( datarate == 11 ) || 
                    ( datarate == 12 ) ) 
                ) || ( 
                    ( bandwidth == 1 ) && ( datarate == 12 ) 
                ) 
            ){
                LLCC68.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x01;
            }
            else
            {
                LLCC68.ModulationParams.Params.LoRa.LowDatarateOptimize = 0x00;
            }

            LLCC68.PacketParams.PacketType = PACKET_TYPE_LORA;

            if( ( LLCC68.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF5 ) ||
                ( LLCC68.ModulationParams.Params.LoRa.SpreadingFactor == LORA_SF6 ) )
            {
                if( preambleLen < 12 )
                {
                    LLCC68.PacketParams.Params.LoRa.PreambleLength = 12;
                }
                else
                {
                    LLCC68.PacketParams.Params.LoRa.PreambleLength = preambleLen;
                }
            }
            else
            {
                LLCC68.PacketParams.Params.LoRa.PreambleLength = preambleLen;
            }

            LLCC68.PacketParams.Params.LoRa.HeaderType = ( RadioLoRaPacketLengthsMode_t )fixLen;
            LLCC68.PacketParams.Params.LoRa.PayloadLength = MaxPayloadLength;
            LLCC68.PacketParams.Params.LoRa.CrcMode = ( RadioLoRaCrcModes_t )crcOn;
            LLCC68.PacketParams.Params.LoRa.InvertIQ = ( RadioLoRaIQModes_t )iqInverted;

            RadioStandby();
            LOGI("RadioStandby end");
            RadioSetModem( ( LLCC68.ModulationParams.PacketType == PACKET_TYPE_GFSK ) ? MODEM_FSK : MODEM_LORA );
            LOGI("RadioSetModem end");
            LLCC68SetModulationParams( &LLCC68.ModulationParams );
            LOGI("LLCC68SetModulationParams end");
            LLCC68SetPacketParams( &LLCC68.PacketParams );
            LOGI("LLCC68SetPacketParams end");
            }
            break;
    }

    // WORKAROUND - Modulation Quality with 500 kHz LoRa Bandwidth, see DS_SX1261-2_V1.2 datasheet chapter 15.1
    if( ( modem == MODEM_LORA ) && ( LLCC68.ModulationParams.Params.LoRa.Bandwidth == LORA_BW_500 ) )
    {
        // RegTxModulation = @address 0x0889
        LLCC68WriteRegister( 0x0889, LLCC68ReadRegister( 0x0889 ) & ~( 1 << 2 ) );
    }
    else
    {
        // RegTxModulation = @address 0x0889
        LLCC68WriteRegister( 0x0889, LLCC68ReadRegister( 0x0889 ) | ( 1 << 2 ) );
    }
    // WORKAROUND END

    LLCC68SetRfTxPower( power );
    TxTimeout = timeout;
}

bool RadioCheckRfFrequency( uint32_t frequency )
{
    return true;
}

static uint32_t RadioGetLoRaBandwidthInHz( RadioLoRaBandwidths_t bw )
{
    uint32_t bandwidthInHz = 0;

    switch( bw )
    {
    case LORA_BW_007:
        bandwidthInHz = 7812UL;
        break;
    case LORA_BW_010:
        bandwidthInHz = 10417UL;
        break;
    case LORA_BW_015:
        bandwidthInHz = 15625UL;
        break;
    case LORA_BW_020:
        bandwidthInHz = 20833UL;
        break;
    case LORA_BW_031:
        bandwidthInHz = 31250UL;
        break;
    case LORA_BW_041:
        bandwidthInHz = 41667UL;
        break;
    case LORA_BW_062:
        bandwidthInHz = 62500UL;
        break;
    case LORA_BW_125:
        bandwidthInHz = 125000UL;
        break;
    case LORA_BW_250:
        bandwidthInHz = 250000UL;
        break;
    case LORA_BW_500:
        bandwidthInHz = 500000UL;
        break;
    }

    return bandwidthInHz;
}

static uint32_t RadioGetGfskTimeOnAirNumerator( uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn )
{
    const RadioAddressComp_t addrComp = RADIO_ADDRESSCOMP_FILT_OFF;
    const uint8_t syncWordLength = 3;

    return ( preambleLen << 3 ) +
           ( ( fixLen == false ) ? 8 : 0 ) +
             ( syncWordLength << 3 ) +
             ( ( payloadLen +
               ( addrComp == RADIO_ADDRESSCOMP_FILT_OFF ? 0 : 1 ) +
               ( ( crcOn == true ) ? 2 : 0 ) 
               ) << 3 
             );
}

static uint32_t RadioGetLoRaTimeOnAirNumerator( uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn )
{
    int32_t crDenom           = coderate + 4;
    bool    lowDatareOptimize = false;
    int32_t ceilDenominator;
    int32_t ceilNumerator = 0;
    int32_t intermediate = 0;

    // Ensure that the preamble length is at least 12 symbols when using SF5 or
    // SF6
    if( ( datarate == 5 ) || ( datarate == 6 ) )
    {
        if( preambleLen < 12 )
        {
            preambleLen = 12;
        }
    }

    if( ( ( bandwidth == 0 ) && ( ( datarate == 11 ) || ( datarate == 12 ) ) ) ||
        ( ( bandwidth == 1 ) && ( datarate == 12 ) ) )
    {
        lowDatareOptimize = true;
    }

    
    ceilNumerator = ( payloadLen << 3 ) +
                            ( crcOn ? 16 : 0 ) -
                            ( 4 * datarate ) +
                            ( fixLen ? 0 : 20 );

    if( datarate <= 6 )
    {
        ceilDenominator = 4 * datarate;
    }
    else
    {
        ceilNumerator += 8;

        if( lowDatareOptimize == true )
        {
            ceilDenominator = 4 * ( datarate - 2 );
        }
        else
        {
            ceilDenominator = 4 * datarate;
        }
    }

    if( ceilNumerator < 0 )
    {
        ceilNumerator = 0;
    }

    // Perform integral ceil()
    intermediate =
        ( ( ceilNumerator + ceilDenominator - 1 ) / ceilDenominator ) * crDenom + preambleLen + 12;

    if( datarate <= 6 )
    {
        intermediate += 2;
    }

    return ( uint32_t )( ( 4 * intermediate + 1 ) * ( 1 << ( datarate - 2 ) ) );
}

uint32_t RadioTimeOnAir( RadioModems_t modem, uint32_t bandwidth,
                              uint32_t datarate, uint8_t coderate,
                              uint16_t preambleLen, bool fixLen, uint8_t payloadLen,
                              bool crcOn )
{
    uint32_t numerator = 0;
    uint32_t denominator = 1;

    switch( modem )
    {
    case MODEM_FSK:
        {
            numerator   = 1000U * RadioGetGfskTimeOnAirNumerator( datarate, coderate,
                                                                  preambleLen, fixLen,
                                                                  payloadLen, crcOn );
            denominator = datarate;
        }
        break;
    case MODEM_LORA:
        {
            numerator   = 1000U * RadioGetLoRaTimeOnAirNumerator( bandwidth, datarate,
                                                                  coderate, preambleLen,
                                                                  fixLen, payloadLen, crcOn );
            denominator = RadioGetLoRaBandwidthInHz( Bandwidths[bandwidth] );
        }
        break;
    }
    // Perform integral ceil()
    return ( numerator + denominator - 1 ) / denominator;
}

void RadioSend( uint8_t *buffer, uint8_t size )
{
    LLCC68SetDioIrqParams( IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                           IRQ_TX_DONE | IRQ_RX_TX_TIMEOUT,
                           IRQ_RADIO_NONE,
                           IRQ_RADIO_NONE );

    if( LLCC68GetPacketType( ) == PACKET_TYPE_LORA )
    {
        LLCC68.PacketParams.Params.LoRa.PayloadLength = size;
    }
    else
    {
        LLCC68.PacketParams.Params.Gfsk.PayloadLength = size;
    }
    LLCC68SetPacketParams( &LLCC68.PacketParams );

    LLCC68SendPayload( buffer, size, 0 );
    LLCC68SetTxTimerValue(TxTimeout);
    LLCC68TxTimerStart();
}

void RadioSleep( void )
{
    SleepParams_t params = { 0 };

    params.Fields.WarmStart = 1;
    LLCC68SetSleep( params );

    LLCC68DelayMs( 2 );
}

void RadioStandby( void )
{
    LLCC68SetStandby( STDBY_RC );
}

void RadioRx( uint32_t timeout )
{
    LLCC68SetDioIrqParams( IRQ_RADIO_ALL, //IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT,
                           IRQ_RADIO_ALL, //IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT,
                           IRQ_RADIO_NONE,
                           IRQ_RADIO_NONE );

    if( timeout != 0 )
    {
        LLCC68SetRxTimerValue(timeout);
        LLCC68RxTimerStart();
    }

    if( RxContinuous == true )
    {
        LLCC68SetRx( 0xFFFFFF ); // Rx Continuous
    }
    else
    {
        LLCC68SetRx( RxTimeout << 6 );
    }
}

void RadioRxBoosted( uint32_t timeout )
{
    LLCC68SetDioIrqParams( IRQ_RADIO_ALL, //IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT,
                           IRQ_RADIO_ALL, //IRQ_RX_DONE | IRQ_RX_TX_TIMEOUT,
                           IRQ_RADIO_NONE,
                           IRQ_RADIO_NONE );

    if( timeout != 0 )
    {
        LLCC68SetRxTimerValue(timeout);
        LLCC68RxTimerStart();
    }

    if( RxContinuous == true )
    {
        LLCC68SetRxBoosted( 0xFFFFFF ); // Rx Continuous
    }
    else
    {
        LLCC68SetRxBoosted( RxTimeout << 6 );
    }
}

void RadioSetRxDutyCycle( uint32_t rxTime, uint32_t sleepTime )
{
    LLCC68SetRxDutyCycle( rxTime, sleepTime );
}

void RadioStartCad( void )
{
    LLCC68SetDioIrqParams( IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED, IRQ_CAD_DONE | IRQ_CAD_ACTIVITY_DETECTED, IRQ_RADIO_NONE, IRQ_RADIO_NONE );
    LLCC68SetCad( );
}

void RadioSetTxContinuousWave( uint32_t freq, int8_t power, uint16_t time )
{
    uint32_t timeout = ( uint32_t )time * 1000;

    LLCC68SetRfFrequency( freq );
    LLCC68SetRfTxPower( power );
    LLCC68SetTxContinuousWave( );

    LLCC68SetTxTimerValue(timeout);
    LLCC68TxTimerStart();
}

int16_t RadioRssi( RadioModems_t modem )
{
    return LLCC68GetRssiInst( );
}

void RadioWrite( uint32_t addr, uint8_t data )
{
    LLCC68WriteRegister( addr, data );
}

uint8_t RadioRead( uint32_t addr )
{
    return LLCC68ReadRegister( addr );
}

void RadioWriteBuffer( uint32_t addr, uint8_t *buffer, uint8_t size )
{
    LLCC68WriteRegisters( addr, buffer, size );
}

void RadioReadBuffer( uint32_t addr, uint8_t *buffer, uint8_t size )
{
    LLCC68ReadRegisters( addr, buffer, size );
}

void RadioSetMaxPayloadLength( RadioModems_t modem, uint8_t max )
{
    if( modem == MODEM_LORA )
    {
        LLCC68.PacketParams.Params.LoRa.PayloadLength = MaxPayloadLength = max;
        LLCC68SetPacketParams( &LLCC68.PacketParams );
    }
    else
    {
        if( LLCC68.PacketParams.Params.Gfsk.HeaderType == RADIO_PACKET_VARIABLE_LENGTH )
        {
            LLCC68.PacketParams.Params.Gfsk.PayloadLength = MaxPayloadLength = max;
            LLCC68SetPacketParams( &LLCC68.PacketParams );
        }
    }
}

void RadioSetPublicNetwork( bool enable )
{
    RadioPublicNetwork.Current = RadioPublicNetwork.Previous = enable;

    RadioSetModem( MODEM_LORA );
    if( enable == true )
    {
        // Change LoRa modem SyncWord
        LLCC68WriteRegister( REG_LR_SYNCWORD, ( LORA_MAC_PUBLIC_SYNCWORD >> 8 ) & 0xFF );
        LLCC68WriteRegister( REG_LR_SYNCWORD + 1, LORA_MAC_PUBLIC_SYNCWORD & 0xFF );
    }
    else
    {
        // Change LoRa modem SyncWord
        LLCC68WriteRegister( REG_LR_SYNCWORD, ( LORA_MAC_PRIVATE_SYNCWORD >> 8 ) & 0xFF );
        LLCC68WriteRegister( REG_LR_SYNCWORD + 1, LORA_MAC_PRIVATE_SYNCWORD & 0xFF );
    }
}

uint32_t RadioGetWakeupTime( void )
{
    return LLCC68GetBoardTcxoWakeupTime( ) + RADIO_WAKEUP_TIME;
}

void RadioOnTxTimeoutIrq( void* context )
{
    if( ( RadioEvents != NULL ) && ( RadioEvents->TxTimeout != NULL ) )
    {
        RadioEvents->TxTimeout( );
    }
}

void RadioOnRxTimeoutIrq( void* context )
{
    if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
    {
        RadioEvents->RxTimeout( );
    }
}

//void RadioOnDioIrq( void* context )
void RadioOnDioIrq( void )
{
    IrqFired = 1;
}

void RadioIrqProcess( void )
{
    QL_LOG(QL_LOG_LEVEL_INFO,"RadioIrqProcess","RadioIrqProcess start\n");
    uint16_t irqRegs = 0x0000;
    QL_LOG(QL_LOG_LEVEL_INFO,"RadioIrqProcess","IrqFired is before %d\n",IrqFired);

    IrqFired = 1;
    QL_LOG(QL_LOG_LEVEL_INFO,"RadioIrqProcess","IrqFired is after %d\n",IrqFired);

    if( 1 /*IrqFired == true*/ )
    {
        //CRITICAL_SECTION_BEGIN( );
        // Clear IRQ flag
        IrqFired = 0;
        QL_LOG(QL_LOG_LEVEL_INFO,"RadioIrqProcess","IrqFired is %d\n",IrqFired);
        //CRITICAL_SECTION_END( );

        /*获取中断状态*/
        QL_LOG(QL_LOG_LEVEL_INFO,"RadioIrqProcess","irqRegs before is 0x%04X\n",irqRegs);
        irqRegs = LLCC68GetIrqStatus();//read cmd  0x12
        LLCC68ClearIrqStatus(irqRegs);//write cmd  0x02
        QL_LOG(QL_LOG_LEVEL_INFO,"RadioIrqProcess","irqRegs after is 0x%04X\n",irqRegs);

        /*根据中断状态执行对应操作*/
        if( ( irqRegs & IRQ_TX_DONE ) == IRQ_TX_DONE )
        {
            LLCC68TxTimerStop();
            //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
            LLCC68SetOperatingMode( MODE_STDBY_RC );
            if( ( RadioEvents != NULL ) && ( RadioEvents->TxDone != NULL ) )
            {
                RadioEvents->TxDone( );
            }
        }

        if( ( irqRegs & IRQ_RX_DONE ) == IRQ_RX_DONE )
        {
            if( ( irqRegs & IRQ_CRC_ERROR ) == IRQ_CRC_ERROR )
            {
                if( RxContinuous == false )
                {
                    //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
                    LLCC68SetOperatingMode( MODE_STDBY_RC );
                }
                if( ( RadioEvents != NULL ) && ( RadioEvents->RxError ) )
                {
                    RadioEvents->RxError( );
                }
            }
            else
            {
                uint8_t size;

                LLCC68RxTimerStop();
                if( RxContinuous == false )
                {
                    //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
                    LLCC68SetOperatingMode( MODE_STDBY_RC );

                    // WORKAROUND - Implicit Header Mode Timeout Behavior, see DS_SX1261-2_V1.2 datasheet chapter 15.3
                    // RegRtcControl = @address 0x0902
                    LLCC68WriteRegister( 0x0902, 0x00 );
                    // RegEventMask = @address 0x0944
                    LLCC68WriteRegister( 0x0944, LLCC68ReadRegister( 0x0944 ) | ( 1 << 1 ) );
                    // WORKAROUND END
                }
                LLCC68GetPayload( RadioRxPayload, &size , 255 );
                LLCC68GetPacketStatus( &RadioPktStatus );
                if( ( RadioEvents != NULL ) && ( RadioEvents->RxDone != NULL ) )
                {
                    RadioEvents->RxDone( RadioRxPayload, size, RadioPktStatus.Params.LoRa.RssiPkt, RadioPktStatus.Params.LoRa.SnrPkt );
                }
            }
        }

        if( ( irqRegs & IRQ_CAD_DONE ) == IRQ_CAD_DONE )
        {
            //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
            LLCC68SetOperatingMode( MODE_STDBY_RC );
            if( ( RadioEvents != NULL ) && ( RadioEvents->CadDone != NULL ) )
            {
                RadioEvents->CadDone( ( ( irqRegs & IRQ_CAD_ACTIVITY_DETECTED ) == IRQ_CAD_ACTIVITY_DETECTED ) );
            }
        }

        if( ( irqRegs & IRQ_RX_TX_TIMEOUT ) == IRQ_RX_TX_TIMEOUT )
        {
            if( LLCC68GetOperatingMode( ) == MODE_TX )
            {
                LLCC68TxTimerStop();
                //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
                LLCC68SetOperatingMode( MODE_STDBY_RC );
                if( ( RadioEvents != NULL ) && ( RadioEvents->TxTimeout != NULL ) )
                {
                    RadioEvents->TxTimeout( );
                }
            }
            else if( LLCC68GetOperatingMode( ) == MODE_RX )
            {
                LLCC68RxTimerStop();
                //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
                LLCC68SetOperatingMode( MODE_STDBY_RC );
                if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
                {
                    RadioEvents->RxTimeout( );
                }
            }
        }

        if( ( irqRegs & IRQ_PREAMBLE_DETECTED ) == IRQ_PREAMBLE_DETECTED )
        {
            //__NOP( );
        }

        if( ( irqRegs & IRQ_SYNCWORD_VALID ) == IRQ_SYNCWORD_VALID )
        {
            //__NOP( );
        }

        if( ( irqRegs & IRQ_HEADER_VALID ) == IRQ_HEADER_VALID )
        {
            //__NOP( );
        }

        if( ( irqRegs & IRQ_HEADER_ERROR ) == IRQ_HEADER_ERROR )
        {
            LLCC68RxTimerStop();
            if( RxContinuous == false )
            {
                //!< Update operating mode state to a value lower than \ref MODE_STDBY_XOSC
                LLCC68SetOperatingMode( MODE_STDBY_RC );
            }
            if( ( RadioEvents != NULL ) && ( RadioEvents->RxTimeout != NULL ) )
            {
                RadioEvents->RxTimeout( );
            }
        }
    }
}
