#ifndef __FSV9563_H
#define __FSV9563_H

//#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx.h"
#include "stm32f0xx_hal.h"

#define UART_PRINT

#define	 FSV9563_NSS	 	GPIO_PIN_4				//PA4
#define	 FSV9563_NSS_0	 HAL_GPIO_WritePin(GPIOA, FSV9563_NSS, GPIO_PIN_RESET) 
#define	 FSV9563_NSS_1	 HAL_GPIO_WritePin(GPIOA, FSV9563_NSS, GPIO_PIN_SET)

#define  FSV9563_IFSEL0_0    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_RESET)
#define  FSV9563_IFSEL0_1    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0, GPIO_PIN_SET)

#define  FSV9563_IFSEL1_0   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET)
#define  FSV9563_IFSEL1_1   HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET)

#define	 FSV9563_IRQ	 	GPIO_PIN_1			//PB1
#define	 FSV9563_IRQ_0	 	HAL_GPIO_WritePin(GPIOA, FSV9563_IRQ, GPIO_PIN_RESET)
#define	 FSV9563_IRQ_1	 	HAL_GPIO_WritePin(GPIOA, FSV9563_IRQ, GPIO_PIN_SET)

#define	 PDOWN	 		GPIO_PIN_1			//PA1
#define	 PDOWN_0	 	HAL_GPIO_WritePin(GPIOA, PDOWN, GPIO_PIN_RESET)
#define	 PDOWN_1	 	HAL_GPIO_WritePin(GPIOA, PDOWN, GPIO_PIN_SET)

#define	 FLAG_0	 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_RESET)
#define	 FLAG_1	 	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET)

#define  LED      GPIO_PIN_3
#define	 LED_0	 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET)
#define	 LED_1	 	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET)
//#define  LED      GPIO_Pin_5
//#define	 LED_0	 	GPIO_ResetBits(GPIOB,GPIO_Pin_5)
//#define	 LED_1	 	GPIO_SetBits(GPIOB,GPIO_Pin_5)

///////////
#define  LED_Toggle  {GPIOC->ODR ^= GPIO_Pin_13;}

#define  typeA    0x03
#define  typeB    0x02
#define  Felica   0x01
#define  V15693   0x00
/////////////////////////////////////////////
#define rRegCommand 					0x00 // Starts and stops command execution
#define rRegHostCtrl 				0x01 // Host control register
#define rRegFIFOControl 			0x02 // Control register of the FIFO
#define rRegWaterLevel 			0x03 // Level of the FIFO underflow and overflow warning
#define rRegFIFOLength 			0x04 // Length of the FIFO
#define rRegFIFOData 				0x05 // Data In/Out exchange register of FIFO buffer
#define rRegIRQ0 						0x06 // Interrupt register 0
#define rRegIRQ1 						0x07 // Interrupt register 1
#define rRegIRQ0En 					0x08 // Interrupt enable register 0
#define rRegIRQ1En 					0x09 // Interrupt enable register 1
#define rRegError 						0x0A // Error bits showing the error status of the last command execution
#define rRegStatus 					0x0B // Contains status of the communication
#define rRegRxBitCtrl 				0x0C // Control register for anticollision adjustments for bit oriented protocols
#define rRegRxColl 					0x0D // Collision position register
#define rRegTControl 				0x0E // Control of Timer 0..3
#define rRegT0Control 				0x0F // Control of Timer0
#define rRegT0ReloadHi 			0x10 // High register of the reload value of Timer0
#define rRegT0ReloadLo 			0x11 // Low register of the reload value of Timer0
#define rRegT0CounterValHi 	0x12 // Counter value high register of Timer0
#define rRegT0CounterValLo 	0x13 // Counter value low register of Timer0
#define rRegT1Control 				0x14 // Control of Timer1
#define rRegT1ReloadHi 			0x15 // High register of the reload value of Timer1
#define rRegT1ReloadLo 			0x16 // Low register of the reload value of Timer1
#define rRegT1CounterValHi 	0x17 // Counter value high register of Timer1
#define rRegT1CounterValLo 	0x18 // Counter value low register of Timer1
#define rRegT2Control 				0x19 // Control of Timer2
#define rRegT2ReloadHi 			0x1A // High byte of the reload value of Timer2
#define rRegT2ReloadLo 			0x1B // Low byte of the reload value of Timer2
#define rRegT2CounterValHi 	0x1C // Counter value high byte of Timer2
#define rRegT2CounterValLo 	0x1D // Counter value low byte of Timer2
#define rRegT3Control 				0x1E // Control of Timer3
#define rRegT3ReloadHi 			0x1F // High byte of the reload value of Timer3
#define rRegT3ReloadLo 			0x20 // Low byte of the reload value of Timer3
#define rRegT3CounterValHi 	0x21 // Counter value high byte of Timer3
#define rRegT3CounterValLo 	0x22 // Counter value low byte of Timer3
#define rRegT4Control 				0x23 // Control of Timer4
#define rRegT4ReloadHi 			0x24 // High byte of the reload value of Timer4
#define rRegT4ReloadLo 			0x25 // Low byte of the reload value of Timer4
#define rRegT4CounterValHi 	0x26 // Counter value high byte of Timer4
#define rRegT4CounterValLo 	0x27 // Counter value low byte of Timer4
#define rRegDrvMod 					0x28 // Driver mode register
#define rRegTxAmp 						0x29 // Transmitter amplifier register
#define rRegDrvCon 					0x2A // Driver configuration register
#define rRegTxl 							0x2B // Transmitter register
#define rRegTxCrcPreset 			0x2C // Transmitter CRC control register, preset value
#define rRegRxCrcPreset 			0x2D // Receiver CRC control register, preset value
#define rRegTxDataNum 				0x2E // Transmitter data number register
#define rRegTxModWidth 			0x2F // Transmitter modulation width register
#define rRegTxSym10BurstLen 	0x30 // Transmitter symbol 1 + symbol 0 burst length register
#define rRegTXWaitCtrl 			0x31 // Transmitter wait control
#define rRegTxWaitLo 				0x32 // Transmitter wait low
#define rRegFrameCon 				0x33 // Transmitter frame control
#define rRegRxSofD 					0x34 // Receiver start of frame detection
#define rRegRxCtrl 					0x35 // Receiver control register
#define rRegRxWait 					0x36 // Receiver wait register
#define rRegRxThreshold 			0x37 // Receiver threshold register
#define rRegRcv 							0x38 // Receiver register
#define rRegRxAna 						0x39 // Receiver analog register
#define rRegRFU_3A						0x3A // -
#define rRegSerialSpeed 			0x3B // Serial speed register
#define rRegLFO_Trimm 				0x3C // Low-power oscillator trimming register
#define rRegPLL_Ctrl 				0x3D // IntegerN PLL control register, for microcontroller clock output adjustment
#define rRegPLL_DivOut 			0x3E // IntegerN PLL control register, for microcontroller clock output adjustment
#define rRegLPCD_QMin 				0x3F // Low-power card detection Q channel minimum threshold
#define rRegLPCD_QMax 				0x40 // Low-power card detection Q channel maximum threshold
#define rRegLPCD_IMin 				0x41 // Low-power card detection I channel minimum threshold
#define rRegLPCD_I_Result 		0x42 // Low-power card detection I channel result register
#define rRegLPCD_Q_Result 		0x43 // Low-power card detection Q channel result register
#define rRegPadEn 						0x44 // PIN enable register
#define rRegPadOut 					0x45 // PIN out register
#define rRegPadIn 						0x46 // PIN in register
#define rRegSigOut 					0x47 // Enables and controls the SIGOUT Pin
#define rRegTxBitMod 				0x48 // Transmitter bit mode register
#define rRegRFU_49						0x49 // -
#define rRegTxDataCon 				0x4A // Transmitter data configuration register
#define rRegTxDataMod 				0x4B // Transmitter data modulation register
#define rRegTxSymFreq 				0x4C // Transmitter symbol frequency
#define rRegTxSym0H 					0x4D // Transmitter symbol 0 high register
#define rRegTxSym0L 					0x4E // Transmitter symbol 0 low register
#define rRegTxSym1H 					0x4F // Transmitter symbol 1 high register
#define rRegTxSym1L 					0x50 // Transmitter symbol 1 low register
#define rRegTxSym2 					0x51 // Transmitter symbol 2 register
#define rRegTxSym3 					0x52 // Transmitter symbol 3 register
#define rRegTxSym10Len 			0x53 // Transmitter symbol 1 + symbol 0 length register
#define rRegTxSym32Len 			0x54 // Transmitter symbol 3 + symbol 2 length register
#define rRegTxSym10BurstCtrl 0x55 // Transmitter symbol 1 + symbol 0 burst control register
#define rRegTxSym10Mod 			0x56 // Transmitter symbol 1 + symbol 0 modulation register
#define rRegTxSym32Mod 			0x57 // Transmitter symbol 3 + symbol 2 modulation register
#define rRegRxBitMod 				0x58 // Receiver bit modulation register
#define rRegRxEofSym 				0x59 // Receiver end of frame symbol register
#define rRegRxSyncValH 			0x5A // Receiver synchronisation value high register
#define rRegRxSyncValL 			0x5B // Receiver synchronisation value low register
#define rRegRxSyncMod 				0x5C // Receiver synchronisation mode register
#define rRegRxMod 						0x5D // Receiver modulation register
#define rRegRxCorr 					0x5E // Receiver correlation register
#define rRegFabCal 					0x5F // Calibration register of the receiver, calibration performed at production
#define rReg_60 							0x60 //
#define rReg_61 							0x61 //
#define rRegTestMode 					0x66 //
#define rReg_6A 							0x6A //
#define rRegTestInSel 				0x6B //
#define rRegAUX1Sel 					0x6C //
#define rRegAUX2Sel 					0x6D //
#define rReg_6E 							0x6E //
#define rReg_6F 							0x6F //
#define rRegVersion 					0x7F // Version and subversion register
//////////////////////////////////////////////////////////
//		Command 			No. Parameter (bytes) 	Short description
#define FSV9563_Idle 				0x00 //- 					no action, cancels current command execution
#define FSV9563_LPCD 				0x01 //- 					low-power card detection
#define FSV9563_LoadKey 		0x02 //(keybyte1..6); 		reads a MIFARE key (size of 6 bytes) from FIFO buffer and puts it into Key buffer
#define FSV9563_MFAuthent 	0x03 //60h or 61h,(block address),(card serial number byte0..3) 	performs the MIFARE standard authentication in MIFARE read/write mode only
#define FSV9563_AckReq 			0x04 //- 					performs a query, an Ack and a Req-Rn for ISO/IEC 18000-3 mode 3/ EPC Class-1 HF
#define FSV9563_Receive 		0x05 //- 					activates the receive circuit
#define FSV9563_Transmit 		0x06 //- 					transmits data from the FIFO buffer
#define FSV9563_Transceive 	0x07 //- 					transmits data from the FIFO buffer and automatically activates the receiver after transmission finished
#define FSV9563_WriteE2 		0x08 //addressH, addressL, data; 	gets one byte from FIFO buffer and writes it to the internal EEPROM, 
#define FSV9563_WriteE2Page 0x09 //(page Address), data0, [data1..data63]; 	gets up to 64 bytes (one EEPROM page) from the FIFO buffer and writes it to the EEPROM
#define FSV9563_ReadE2 			0x0A // address H, addressL,length; 	reads data from the EEPROM and copies it into the FIFO buffer
#define FSV9563_LoadReg 		0x0C //(EEPROM addressL), (EEPROM addressH), RegAdr, (number of Register to be copied);  reads data from the internal EEPROM and initializes the CLFSV9563 registers. EEPROM address needs to be within EEPROM sector 2
#define FSV9563_LoadProtocol 0x0D //(Protocol number RX), (Protocol number TX);		reads data from the internal EEPROM and initializes the CLFSV9563 registers needed for a Protocol change
#define FSV9563_LoadKeyE2 	0x0E //KeyNr; 				copies a key of the EEPROM into the key buffer
#define FSV9563_StoreKeyE2 	0x0F //KeyNr, byte1..6;	stores a MIFARE key (size of 6 bytes) into the EEPROM
#define FSV9563_ReadRNR 		0x1C //- 					Copies bytes from the Random Number generator into the FIFO until the FiFo is full
#define FSV9563_Soft_Reset 	0x1F //- 					resets the CLFSV9563

#define ADC_FLAG_EOC                               ((uint8_t)0x02)
#define ADC_SampleTime_239Cycles5                  ((uint8_t)0x07)
#define ADC_Channel_7                               ((uint8_t)0x07)
//////////////////////////////////////////////////////////
void delay_init(u8 SYSCLK);
void delay_us(u32);
void delay_ms(u16);

void FSV9563_Init(void);
u8 FSV9563_ReadReg(u8);
void FSV9563_WriteReg(u8, u8);
void FSV9563_WriteReg32(u32, u32);
uint32_t FSV9563_ReadReg32(uint32_t Address);

s8 FSV9563_CMD_LoadProtocol(u8 rx,u8 tx);
//s8 FSV9563_CMD_WriteE2(u16 addr,u8 dat);
//s8 FSV9563_CMD_ReadE2(u16 addr,u8 len,u8 *pdat);

s8 FSV9563_PcdConfigISOType(u8 type);
void FSV9563_TypeA(void);
void FSV9563_TypeB(void);
void FSV9563_Felica(void);
void FSV9563_ISO15693(void);
void FSV9563_PcdPollingV(void);
void FSV9563_TAG(void);
void FSV9563_PassWordInit(void);
 void FSV9563_Trigger(int *Temp);
 
void FSV9563_NT3H2111(void); 
u8 FSV9563_NT3H2111Test(u8 *prbuf,u8 *prlen);
s8 FSV9563_NT3H2111Write(u8   addr,u8 *pWritedata);
u8 FSV9563_NT3H2111Read(u8 page,u8 *prbuf,u8 *prlen);
void FSV9563_NT3H2111_Transfer(void);

s8 FSV9563_Lpcd_Calib(u8 *pI,u8 *pQ);
s8 FSV9563_Lpcd_Det(u8 ValueI,u8 ValueQ);
void FSV9563_FieldOff(void);
void FSV9563_FieldOn(void);
u8 ReadChipSel(void);
u16 ReadADC1(void);
void TestMode_FSV9563(void);
s8 FSV9563_PcdAuth(u8 *pSize);
s8 FSV9563_PcdAuthState(u8 auth_mode,u8 block,u8 *pKey,u8 *pSnr);
#endif
