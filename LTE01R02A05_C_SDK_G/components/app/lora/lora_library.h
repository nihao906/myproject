
// #include "llcc68.h"
// #include "radio.h"
// typedef enum
// {
//     STDBY_RC                                = 0x00,
//     STDBY_XOSC                              = 0x01,
// }RadioStandbyModes_t;

// typedef enum
// {
//     USE_LDO                                 = 0x00, // default
//     USE_DCDC                                = 0x01,
// }RadioRegulatorMode_t;

// typedef enum RadioCommands_e
// {
//     RADIO_GET_STATUS                        = 0xC0,
//     RADIO_WRITE_REGISTER                    = 0x0D,
//     RADIO_READ_REGISTER                     = 0x1D,
//     RADIO_WRITE_BUFFER                      = 0x0E,
//     RADIO_READ_BUFFER                       = 0x1E,
//     RADIO_SET_SLEEP                         = 0x84,
//     RADIO_SET_STANDBY                       = 0x80,
//     RADIO_SET_FS                            = 0xC1,
//     RADIO_SET_TX                            = 0x83,
//     RADIO_SET_RX                            = 0x82,
//     RADIO_SET_RXDUTYCYCLE                   = 0x94,
//     RADIO_SET_CAD                           = 0xC5,
//     RADIO_SET_TXCONTINUOUSWAVE              = 0xD1,
//     RADIO_SET_TXCONTINUOUSPREAMBLE          = 0xD2,
//     RADIO_SET_PACKETTYPE                    = 0x8A,
//     RADIO_GET_PACKETTYPE                    = 0x11,
//     RADIO_SET_RFFREQUENCY                   = 0x86,
//     RADIO_SET_TXPARAMS                      = 0x8E,
//     RADIO_SET_PACONFIG                      = 0x95,
//     RADIO_SET_CADPARAMS                     = 0x88,
//     RADIO_SET_BUFFERBASEADDRESS             = 0x8F,
//     RADIO_SET_MODULATIONPARAMS              = 0x8B,
//     RADIO_SET_PACKETPARAMS                  = 0x8C,
//     RADIO_GET_RXBUFFERSTATUS                = 0x13,
//     RADIO_GET_PACKETSTATUS                  = 0x14,
//     RADIO_GET_RSSIINST                      = 0x15,
//     RADIO_GET_STATS                         = 0x10,
//     RADIO_RESET_STATS                       = 0x00,
//     RADIO_CFG_DIOIRQ                        = 0x08,
//     RADIO_GET_IRQSTATUS                     = 0x12,
//     RADIO_CLR_IRQSTATUS                     = 0x02,
//     RADIO_CALIBRATE                         = 0x89,
//     RADIO_CALIBRATEIMAGE                    = 0x98,
//     RADIO_SET_REGULATORMODE                 = 0x96,
//     RADIO_GET_ERROR                         = 0x17,
//     RADIO_CLR_ERROR                         = 0x07,
//     RADIO_SET_TCXOMODE                      = 0x97,
//     RADIO_SET_TXFALLBACKMODE                = 0x93,
//     RADIO_SET_RFSWITCHMODE                  = 0x9D,
//     RADIO_SET_STOPRXTIMERONPREAMBLE         = 0x9F,
//     RADIO_SET_LORASYMBTIMEOUT               = 0xA0,
// }RadioCommands_t;

// typedef enum
// {
//     RADIO_RAMP_10_US                        = 0x00,
//     RADIO_RAMP_20_US                        = 0x01,
//     RADIO_RAMP_40_US                        = 0x02,
//     RADIO_RAMP_80_US                        = 0x03,
//     RADIO_RAMP_200_US                       = 0x04,
//     RADIO_RAMP_800_US                       = 0x05,
//     RADIO_RAMP_1700_US                      = 0x06,
//     RADIO_RAMP_3400_US                      = 0x07,
// }RadioRampTimes_t;

// typedef enum
// {
//     IRQ_RADIO_NONE                          = 0x0000,
//     IRQ_TX_DONE                             = 0x0001,
//     IRQ_RX_DONE                             = 0x0002,
//     IRQ_PREAMBLE_DETECTED                   = 0x0004,
//     IRQ_SYNCWORD_VALID                      = 0x0008,
//     IRQ_HEADER_VALID                        = 0x0010,
//     IRQ_HEADER_ERROR                        = 0x0020,
//     IRQ_CRC_ERROR                           = 0x0040,
//     IRQ_CAD_DONE                            = 0x0080,
//     IRQ_CAD_ACTIVITY_DETECTED               = 0x0100,
//     IRQ_RX_TX_TIMEOUT                       = 0x0200,
//     IRQ_RADIO_ALL                           = 0xFFFF,
// }RadioIrqMasks_t;

// typedef enum
// {
//     MODE_SLEEP                              = 0x00,         //! The radio is in sleep mode
//     MODE_STDBY_RC,                                          //! The radio is in standby mode with RC oscillator
//     MODE_STDBY_XOSC,                                        //! The radio is in standby mode with XOSC oscillator
//     MODE_FS,                                                //! The radio is in frequency synthesis mode
//     MODE_TX,                                                //! The radio is in transmit mode
//     MODE_RX,                                                //! The radio is in receive mode
//     MODE_RX_DC,                                             //! The radio is in receive duty cycle mode
//     MODE_CAD                                                //! The radio is in channel activity detection mode
// }RadioOperatingModes_t;

// typedef enum
// {
//     MODEM_FSK = 0,
//     MODEM_LORA,
// }RadioModems_t;

// typedef struct
// {
//     uint32_t bandwidth;
//     uint8_t  RegValue;
// }FskBandwidth_t;

// typedef struct
// {
//     bool Previous;
//     bool Current;
// }RadioPublicNetwork_t;

// // typedef struct
// // {
// //     RadioPacketTypes_t                    PacketType;        //!< Packet to which the packet parameters are referring to.
// //     struct
// //     {
// //         /*!
// //          * \brief Holds the GFSK packet parameters
// //          */
// //         struct
// //         {
// //             uint16_t                     PreambleLength;    //!< The preamble Tx length for GFSK packet type in bit
// //             RadioPreambleDetection_t     PreambleMinDetect; //!< The preamble Rx length minimal for GFSK packet type
// //             uint8_t                      SyncWordLength;    //!< The synchronization word length for GFSK packet type
// //             RadioAddressComp_t           AddrComp;          //!< Activated SyncWord correlators
// //             RadioPacketLengthModes_t     HeaderType;        //!< If the header is explicit, it will be transmitted in the GFSK packet. If the header is implicit, it will not be transmitted
// //             uint8_t                      PayloadLength;     //!< Size of the payload in the GFSK packet
// //             RadioCrcTypes_t              CrcLength;         //!< Size of the CRC block in the GFSK packet
// //             RadioDcFree_t                DcFree;
// //         }Gfsk;
// //         /*!
// //          * \brief Holds the LoRa packet parameters
// //          */
// //         struct
// //         {
// //             uint16_t                     PreambleLength;    //!< The preamble length is the number of LoRa symbols in the preamble
// //             RadioLoRaPacketLengthsMode_t HeaderType;        //!< If the header is explicit, it will be transmitted in the LoRa packet. If the header is implicit, it will not be transmitted
// //             uint8_t                      PayloadLength;     //!< Size of the payload in the LoRa packet
// //             RadioLoRaCrcModes_t          CrcMode;           //!< Size of CRC block in LoRa packet
// //             RadioLoRaIQModes_t           InvertIQ;          //!< Allows to swap IQ for LoRa packet
// //         }LoRa;
// //     }Params;                                                //!< Holds the packet parameters structure
// // }PacketParams_t;

// // typedef struct
// // {
// //     RadioPacketTypes_t                    packetType;      //!< Packet to which the packet status are referring to.
// //     struct
// //     {
// //         struct
// //         {
// //             uint8_t RxStatus;
// //             int8_t RssiAvg;                                //!< The averaged RSSI
// //             int8_t RssiSync;                               //!< The RSSI measured on last packet
// //             uint32_t FreqError;
// //         }Gfsk;
// //         struct
// //         {
// //             int8_t RssiPkt;                                //!< The RSSI of the last packet
// //             int8_t SnrPkt;                                 //!< The SNR of the last packet
// //             int8_t SignalRssiPkt;
// //             uint32_t FreqError;
// //         }LoRa;
// //     }Params;
// // }PacketStatus_t;

// // typedef struct
// // {
// //     RadioPacketTypes_t                   PacketType;        //!< Packet to which the modulation parameters are referring to.
// //     struct
// //     {
// //         struct
// //         {
// //             uint32_t                     BitRate;
// //             uint32_t                     Fdev;
// //             RadioModShapings_t           ModulationShaping;
// //             uint8_t                      Bandwidth;
// //         }Gfsk;
// //         struct
// //         {
// //             RadioLoRaSpreadingFactors_t  SpreadingFactor;   //!< Spreading Factor for the LoRa modulation
// //             RadioLoRaBandwidths_t        Bandwidth;         //!< Bandwidth for the LoRa modulation
// //             RadioLoRaCodingRates_t       CodingRate;        //!< Coding rate for the LoRa modulation
// //             uint8_t                      LowDatarateOptimize; //!< Indicates if the modem uses the low datarate optimization
// //         }LoRa;
// //     }Params;                                                //!< Holds the modulation parameters structure
// // }ModulationParams_t;

// // typedef struct LLCC68_s
// // {
// //     PacketParams_t PacketParams;
// //     PacketStatus_t PacketStatus;
// //     ModulationParams_t ModulationParams;
// // }LLCC68_t;

// #define LORA_FRE									868500000	// 收发频率
// #define LORA_TX_OUTPUT_POWER                        20        // 测试默认使用的发射功率，126x发射功率0~22dbm，127x发射功率2~20dbm
// #define LORA_BANDWIDTH                              1         // [0: 125 kHz,	测试默认使用的带宽，LLCC68：[0: 125 kHz,1: 250 kHz,2: 500 kHz,3: Reserved]
// #define LORA_SPREADING_FACTOR                       9         // 测试默认使用的扩频因子范围7~12
// #define LORA_CODINGRATE                             1         // 测试默认使用的纠错编码率[1: 4/5,2: 4/6,3: 4/7,4: 4/8]
// #define LORA_PREAMBLE_LENGTH                        8         // 前导码长度
// #define LORA_LLCC68_SYMBOL_TIMEOUT                  0         // Symbols(LLCC68用到的是0,127x用到的是5)
// #define LORA_FIX_LENGTH_PAYLOAD_ON                  false			// 是否为固定长度包(暂时只是LLCC68用到了)
// #define LORA_IQ_INVERSION_ON                        false			// 这个应该是设置是否翻转中断电平的(暂时只是LLCC68用到了)
// #define LORA_RX_TIMEOUT_VALUE                       5000

// #define LLCC68_XTAL_FREQ                            32000000UL
// #define LLCC68_PLL_STEP_SHIFT_AMOUNT                ( 14 )
// #define LLCC68_PLL_STEP_SCALED                      ( LLCC68_XTAL_FREQ >> ( 25 - LLCC68_PLL_STEP_SHIFT_AMOUNT ) )

// void dio1IrqCallback(void *param);

// //ql_radio_init
// void lora_gpio_init(void);
// void lora_spi_init(void);
// void lora_tx_rx_buf_init(void);

// void lora_set_standby_mode(RadioStandbyModes_t mode);
// void lora_set_regulator_mode(RadioRegulatorMode_t mode);
// void lora_set_buffer_base_address(uint8_t txBaseAddress, uint8_t rxBaseAddress);
// void ql_lora_set_TxParams(int8_t power, RadioRampTimes_t rampTime);
// void ql_lora_set_DioTrqParams(uint16_t irqMask, uint16_t dio1Mask, uint16_t dio2Mask, uint16_t dio3Mask);
// uint8_t RadioGetFskBandwidthRegValue( uint32_t bandwidth );