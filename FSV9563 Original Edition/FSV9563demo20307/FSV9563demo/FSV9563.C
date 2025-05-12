#include "FSV9563.h"
#include "nfc.h"
#include <stdio.h>
#include <string.h>

static u8  fac_us=72;
static u16 fac_ms=65530;
void TestMode_FSV9563(void)
{
/**************************************************************************************/	      
//			FSV9563_WriteReg(0x66,0xC0);
	
//		  FSV9563_WriteReg(0x6C,0x00);
//      FSV9563_WriteReg(0x6A,0x00);
//      FSV9563_WriteReg(0x6D,0x15); //VLDO   AUX2
		
//			FSV9563_WriteReg(0x6C,0x11);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0x00); //VREF   AUX1
		
//			FSV9563_WriteReg(0x6C,0x0A);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0x00); //VBG    AUX1
//		
//			FSV9563_WriteReg(0x6C,0xC0);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0xCA); //ITST    AUX2
		
//			FSV9563_WriteReg(0x6C,0xC0);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0xD4); //IB13    AUX2

//			FSV9563_WriteReg(0x6C,0xC0);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0xD3); //VREF_OK  AUX2

//			FSV9563_WriteReg(0x6C,0xC0);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0xD0); //VPOR   AUX1

//			FSV9563_WriteReg(0x6C,0x00);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0x12); //VTSTI   AUX2
	/*********************************************************************************/
			FSV9563_WriteReg(0x66,0xc0);
			FSV9563_WriteReg(0x6A,0x00); 

//			FSV9563_WriteReg(0x6C,0x01);    //Q路第二级IA P端输出 AUX2
//			FSV9563_WriteReg(0x6D,0x03);    //I路第二级IA P端输出 AUX1
			
	//		FSV9563_WriteReg(0x6C,0x05);		//Q路第一级IA P端输出 AUX2
	//		FSV9563_WriteReg(0x6D,0x07);    //I路第一级IA P端输出 AUX1
			
	//		FSV9563_WriteReg(0x6C,0x0B);		//Q路解调P端输出 AUX2
	//		FSV9563_WriteReg(0x6D,0x0D);    //I路解调P端输出 AUX1
			
			FSV9563_WriteReg(0x6C,0x02);    //Q路第二级IA N端输出 AUX2
			FSV9563_WriteReg(0x6D,0x04);    //I路第二级IA N端输出 AUX1
			
	//		FSV9563_WriteReg(0x6C,0x06);		//Q路第一级IA N端输出 AUX2
	//		FSV9563_WriteReg(0x6D,0x08);    //I路第一级IA N端输出 AUX1
			
	//		FSV9563_WriteReg(0x6C,0x0C);		//Q路解调N端输出 AUX2
	//		FSV9563_WriteReg(0x6D,0x0E);    //I路解调N端输出 AUX1
	
//			FSV9563_WriteReg(0x6C,0xC9);
//			FSV9563_WriteReg(0x6A,0x00);
//			FSV9563_WriteReg(0x6D,0xC9);    //DAC输出，根据数字测试要求

	
}
//////////////////////////////////////////////////////////////////////////////delay
void delay_ns(u32 ns)
{
  u32 i;
  for(i=0;i<ns;i++)
  {
    __NOP();
    __NOP();
    __NOP();
  }
}

void delay_init(u8 SYSCLK)		//unit:MHz
{
	SysTick->CTRL &= 0xfffffffb;//select internal clk: HCLK/8
	fac_us = SYSCLK/8;      
	fac_ms = (u16)fac_us*1000;
}            

void delay_us(u32 Nus)
{ 
	SysTick->LOAD=Nus*fac_us;       //load time      
	SysTick->CTRL|=0x01;            //start count   
	while(!(SysTick->CTRL&(1<<16)));//wait time out 
	SysTick->CTRL=0X00000000;       //close counter
	SysTick->VAL=0X00000000;        //clear counter    
}

void delay_ms(u16 nms)	//nms <= 0xffffff*8/SYSCLK; for 72M, Nms<=1864 
{    
	SysTick->LOAD=(u32)nms*fac_ms; 
	SysTick->CTRL|=0x01;               
	while(!(SysTick->CTRL&(1<<16)));  
	SysTick->CTRL&=0XFFFFFFFE;         
	SysTick->VAL=0X00000000;           
}

void printStatus(const char *message,s8 status,u8 *pbuf,u8 len)
{
#ifdef UART_PRINT
	u8 i;
	printf("%s: %d_",message,status);
	if(status==MI_OK )// || status==MI_FRAMINGERR
	{
		for(i=0;i<len;i++)
			printf(" %02X",*(pbuf+i));
	}
	printf("\n");
#endif
}
/////////////////////////////////////////////////////////////////////
void FSV9563_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	ADC_InitTypeDef ADC_InitStructure;
	u8 temp;

	GPIO_InitStructure.GPIO_Pin = PDOWN;	 					//PA8
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	PDOWN_1;
	
	GPIO_InitStructure.GPIO_Pin = FSV9563_IRQ;  					//PB11	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource11);	 //IRQ

	EXTI_InitStructure.EXTI_Line = EXTI_Line11;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  	//falling edge of IRQ result interrupt
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	
	
	//IFSEL0,IFSEL1:0 1
	GPIO_InitStructure.GPIO_Pin = FSV9563_NSS;	 			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 
	FSV9563_NSS_1;			
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;	 			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	FSV9563_IFSEL0_0;
	FSV9563_IFSEL1_1;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;//SPI_BaudRatePrescaler_64;	
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	
	SPI_InitStructure.SPI_CRCPolynomial = 7;	
	SPI_Init(SPI2, &SPI_InitStructure);  
	SPI_Cmd(SPI2, ENABLE); 
	
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_7;	//PA7 ChipSelAdc  PA7 REFOUT 
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);
	
//  ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_239Cycles5);	 	
	ADC_Cmd(ADC1, ENABLE);  
//	ADC_ResetCalibration(ADC1);
//	while(ADC_GetResetCalibrationStatus(ADC1));
//	ADC_StartCalibration(ADC1);
//	while(ADC_GetCalibrationStatus(ADC1));
//	ADC_Cmd(ADC1, DISABLE);
	
	PDOWN_0;	//	->RESET
	delay_ms(30);
	temp = FSV9563_ReadReg(rRegVersion);
#ifdef UART_PRINT
	printf("version: %X\n",temp);
#endif
}
u8 ReadChipSel(void)
{
	u16 temp;
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_239Cycles5);	 	//PA7 
	temp = ReadADC1();
	if(temp<0x400)							//V
		return 3;
	else if(temp<0x700)					//F
		return 2;
	else if(temp<0xA00)					//B
		return 1;
	else 											
		return 0;									//A
}
u16 Get_Adc(void) 
{
//设置指定 ADC 的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 1, ADC_SampleTime_239Cycles5);
  ADC_SoftwareStartConvCmd(ADC1, ENABLE); //使能指定的 ADC1 的软件转换启动功能
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束
	return ADC_GetConversionValue(ADC1); //返回最近一次 ADC1 规则组的转换结果
}
u16 ReadADC1(void)
{
	u32 temp_val=0; u8 t;
	for(t=0;t<4;t++)
{
	temp_val+=Get_Adc();
	delay_ms(5);
}
return temp_val/4;
}
//////////////////////////////////////////////
u8 FSV9563_SPIWriteByte(u8 Byte)
{
	while((SPI2->SR&0X02)==0);		 
	SPI2->DR=Byte;	 	            
	while((SPI2->SR&0X01)==0);     
	return SPI2->DR;        	  
}

void FSV9563_WriteReg(u8   Address, u8   value)
{  
	FSV9563_NSS_0;
	FSV9563_SPIWriteByte(Address<<1);
	FSV9563_SPIWriteByte(value);
	FSV9563_NSS_1;
	delay_ns(20);
}

u8 FSV9563_ReadReg(u8   Address)
{
	u8  ucResult=0;
	FSV9563_NSS_0;
	FSV9563_SPIWriteByte((Address<<1)|0x01);
	ucResult = FSV9563_SPIWriteByte(0);
	FSV9563_NSS_1;
	delay_ns(20);
	return ucResult;
}

void FSV9563_SetBitMask(u8   reg,u8   mask)  
{
    u8  tmp = FSV9563_ReadReg(reg);
    FSV9563_WriteReg(reg,tmp | mask);  
}

void FSV9563_ClearBitMask(u8   reg,u8   mask)  
{
    u8  tmp = FSV9563_ReadReg(reg);
    FSV9563_WriteReg(reg, tmp & ~mask);  
}

void FSV9563_SetRawRC(u8   reg,u8 mask,u8 set)
{
	u8 temp = FSV9563_ReadReg(reg);
	temp = (temp&mask)|set;
	FSV9563_WriteReg(reg,temp);
}
///////////////////////////////////////////////////////////
void FSV9563_FlushFifo()
{
	FSV9563_SetBitMask(rRegFIFOControl,0x10);
}

void FSV9563_FieldOn()
{
	FSV9563_SetBitMask(rRegDrvMod,0x08);
}

void FSV9563_FieldOff()
{
	FSV9563_ClearBitMask(rRegDrvMod,0x08);
}

void FSV9563_FieldReset()
{
	FSV9563_FieldOff();
	delay_ms(20);
	FSV9563_FieldOn();
	delay_ms(20);
}

extern u8 Status_INT;
u8 mode;
s8 FSV9563_Command_Int(struct TranSciveBuffer *pi)
{
	u16 i;
	u8 j,n;
	FSV9563_WriteReg(rRegCommand,FSV9563_Idle);
	FSV9563_SetBitMask(rRegFIFOControl,0x10);		//FlushFifo
	FSV9563_WriteReg(rRegIRQ0,0x7F);	
	FSV9563_WriteReg(rRegIRQ1,0x7F);
	
	for(n=0;n<pi->Length;n++)
		FSV9563_WriteReg(rRegFIFOData, pi->Data[n]);
	if(pi->Command&0x80)
	{
		FSV9563_WriteReg(rRegIRQ0En,0x10);		
		if(mode)
			FSV9563_WriteReg(rRegIRQ1En,0xE0);		
		else
			FSV9563_WriteReg(rRegIRQ1En,0xE8);		//T3定时器中断请求
		
		Status_INT=0;
		FSV9563_WriteReg(rRegCommand, pi->Command);
		while(Status_INT==0);				//wait for IRQ ,using LPCD
		Status_INT=0;
		
		FSV9563_WriteReg(rRegIRQ0En,0x10);		
		if(mode)
			FSV9563_WriteReg(rRegIRQ1En,0x20);		
		else
			FSV9563_WriteReg(rRegIRQ1En,0x28);		
	}
	else
		FSV9563_WriteReg(rRegCommand, pi->Command);
	
	for(i=2000;i>0;i--)
	{
		n = FSV9563_ReadReg(rRegIRQ0);	
		if(n&0x10) break;		//IDLEIRQ  等待命令自行终止
	}
	if(i==0)
		return MI_ERR;
	n = FSV9563_ReadReg(rRegFIFOLength);
	for(j=0;j<n;j++)
		pi->Data[j]= FSV9563_ReadReg(rRegFIFOData);
	return MI_OK;
}
	
s8 FSV9563_CMD_LoadProtocol(u8 rx,u8 tx)
{
	struct TranSciveBuffer ComData;
	
	ComData.Command = FSV9563_LoadProtocol;
	ComData.Length = 2;
	ComData.Data[0] = rx;
	ComData.Data[1] = tx;

	return FSV9563_Command_Int(&ComData);
}

s8 FSV9563_CMD_LoadKey(u8* pkey)
{
	struct TranSciveBuffer ComData;
	
	ComData.Command = FSV9563_LoadKey;
	ComData.Length = 6;
	memcpy(ComData.Data,pkey,6);

	return FSV9563_Command_Int(&ComData);
}

s8 FSV9563_CMD_MfcAuthenticate(u8 auth_mode,u8 block,u8 *pSnr)
{
	s8 status;
	u8 reg;
	struct TranSciveBuffer ComData;
	
	ComData.Command = FSV9563_MFAuthent;
	ComData.Length = 6;
	ComData.Data[0] = auth_mode;
	ComData.Data[1] = block;
	memcpy(&ComData.Data[2],pSnr,4);

	status= FSV9563_Command_Int(&ComData);
	if(status==MI_OK)
	{
		reg = FSV9563_ReadReg(rRegStatus);
		if(!(reg&0x20))
			status=MI_AUTHERR;
	}
	return status;
}
s8 FSV9563_PcdAuthState(u8 auth_mode,u8 block,u8 *pKey,u8 *pSnr)
{
  s8 status;
	u8 reg;
 	struct TranSciveBuffer ComData;

	ComData.Command  = FSV9563_MFAuthent;
  ComData.Length  = 12;
  ComData.Data[0] = auth_mode;
  ComData.Data[1] = block;
  memcpy(&ComData.Data[2], pKey, 6); 
  memcpy(&ComData.Data[8], pSnr, 4); 
  status = FSV9563_Command_Int(&ComData);
  if (status == MI_OK) 
	{  
		reg = FSV9563_ReadReg(rRegStatus);
		if(!(reg&0x08))
			status = MI_AUTHERR;
  }
  return status;
}
//s8 FSV9563_CMD_ReadE2(u16 addr,u8 len,u8 *pdat)
//{
//	struct TranSciveBuffer ComData;
//	s8 status;
//	
//	ComData.Command = FSV9563_ReadE2;
//	ComData.Length = 3;
//	ComData.Data[0] = addr>>8;
//	ComData.Data[1] = addr&0xff;
//	ComData.Data[2] = len;

//	status = FSV9563_Command_Int(&ComData);
//	if(status== MI_OK)
//		memcpy(pdat,ComData.Data,len);
//	return status;
//}

//s8 FSV9563_CMD_WriteE2(u16 addr,u8 dat)
//{
//	struct TranSciveBuffer ComData;
//	s8 status;
//	
//	ComData.Command = FSV9563_ReadE2;
//	ComData.Length = 3;
//	ComData.Data[0] = addr>>8;
//	ComData.Data[1] = addr&0xff;
//	ComData.Data[2] = dat;

//	status = FSV9563_Command_Int(&ComData);
//	return status;
//}

s8 FSV9563_PcdConfigISOType(u8 type)
{
	FSV9563_WriteReg(rRegT0Control,0x98); //Starts at the end of Tx. Stops after Rx of first data. Auto-reloaded. 13.56 MHz input clock.
	FSV9563_WriteReg(rRegT1Control,0x92); //Starts at the end of Tx. Stops after Rx of first data. Input clock - cascaded with Timer-0.
	FSV9563_WriteReg(rRegT2Control,0x20); //Timer used for LFO trimming
	FSV9563_WriteReg(rRegT2ReloadHi,0x03);	//
	FSV9563_WriteReg(rRegT2ReloadLo,0xFF);	//
	FSV9563_WriteReg(rRegT3Control,0x00);	//Not started automatically. Not reloaded. Input clock 13.56 MHz	
	if(type=='A')
	{
		FSV9563_WriteReg(rRegWaterLevel,0x10);	//Set WaterLevel =(FIFO length -1)
										 
		FSV9563_WriteReg(rRegRxBitCtrl,0x80);	//Received bit after collision are replaced with 1.
		FSV9563_WriteReg(rRegDrvMod,0x80);	//Tx2Inv=1
		FSV9563_WriteReg(rRegTxAmp,0xC0);	// 0xc0
		FSV9563_WriteReg(rRegDrvCon,0x09);	//01
		FSV9563_WriteReg(rRegTxl,0x05);	//
		FSV9563_WriteReg(rRegRxSofD,0x00);	//
		
		FSV9563_CMD_LoadProtocol(0,0);

		FSV9563_WriteReg(rRegIRQ0En,0);
		FSV9563_WriteReg(rRegIRQ1En,0);
		
		FSV9563_WriteReg(rRegFIFOControl,0xB0);

		FSV9563_WriteReg(rRegTxModWidth,0x20); //2fh, Length of the pulse modulation in carrier clks+1  
		FSV9563_WriteReg(rRegTxSym10BurstLen,0); // Symbol 1 and 0 burst lengths = 8 bits.
		FSV9563_WriteReg(rRegFrameCon,0xCF); // Start symbol=Symbol2, Stop symbol=Symbol3
		
		FSV9563_WriteReg(rRegRxCtrl,0x04); //35h, Set Rx Baudrate 106 kBaud 
		FSV9563_WriteReg(rRegRxThreshold,0x55); // Set min-levels for Rx and phase shift  //32 
		FSV9563_WriteReg(rRegRcv,0x12);	//
		FSV9563_WriteReg(rRegRxAna,0x0A); //0
		
		FSV9563_WriteReg(rRegDrvMod,0x81);
		//> MIFARE Crypto1 state is further disabled.
		FSV9563_WriteReg(rRegStatus,0);
		//>  FieldOn
		FSV9563_WriteReg(rRegDrvMod,0x89);
	}
	else if(type=='B')
	{
		FSV9563_WriteReg(rRegWaterLevel,0x10);	//Set WaterLevel =(FIFO length -1)
										 
		FSV9563_WriteReg(rRegRxBitCtrl,0x80);	//Received bit after collision are replaced with 1.
		FSV9563_WriteReg(rRegDrvMod,0x8F);	//Tx2Inv=1
		FSV9563_WriteReg(rRegTxAmp,0x0C);	// 0xCC	//0x0C
		FSV9563_WriteReg(rRegDrvCon,0x01);	//0x01 //0X09
		FSV9563_WriteReg(rRegTxl,0x05);	//
		FSV9563_WriteReg(rRegRxSofD,0x00);	//
		
		FSV9563_CMD_LoadProtocol(4,4);

		// Disable Irq 0,1 sources
		FSV9563_WriteReg(rRegIRQ0En,0);
		FSV9563_WriteReg(rRegIRQ1En,0);
		
		FSV9563_WriteReg(rRegFIFOControl,0xB0);

		FSV9563_WriteReg(rRegTxModWidth,0x0A); // Length of the pulse modulation in carrier clks+1  
		FSV9563_WriteReg(rRegTxSym10BurstLen,0); // Symbol 1 and 0 burst lengths = 8 bits.
		FSV9563_WriteReg(rRegTXWaitCtrl,1);	
		FSV9563_WriteReg(rRegFrameCon,0x05); 
		FSV9563_WriteReg(rRegRxSofD,0xB2);
		
		FSV9563_WriteReg(rRegRxCtrl,0x34); // Set Rx Baudrate 106 kBaud 
		FSV9563_WriteReg(rRegRxThreshold,0x9f); // Set min-levels for  Rx and phase shift  0x7F 
		FSV9563_WriteReg(rRegRcv,0x12);//12
		FSV9563_WriteReg(rRegRxAna,0x0e); //0xa 0X0e
		
		FSV9563_WriteReg(rRegDrvMod,0x87);
		FSV9563_WriteReg(rRegStatus,0);
		//>  FieldOn
		FSV9563_WriteReg(rRegDrvMod,0x8F);
	}
	else if(type=='F')
	{
		FSV9563_WriteReg(rRegWaterLevel,0x10);	//Set WaterLevel =(FIFO length -1)
										 
		FSV9563_WriteReg(rRegRxBitCtrl,0x80);	//Received bit after collision are replaced with 1.
		FSV9563_WriteReg(rRegDrvMod,0x88);	//Tx2Inv=1
		FSV9563_WriteReg(rRegTxAmp,0x04);	//
		FSV9563_WriteReg(rRegDrvCon,0x01);	//
		FSV9563_WriteReg(rRegTxl,0x05);	//
		FSV9563_WriteReg(rRegRxSofD,0x00);	//
		
		FSV9563_CMD_LoadProtocol(8,8);

		// Disable Irq 0,1 sources
		FSV9563_WriteReg(rRegIRQ0En,0);
		FSV9563_WriteReg(rRegIRQ1En,0);
		
		FSV9563_WriteReg(rRegFIFOControl,0xB0);
		
		FSV9563_WriteReg(rRegTxModWidth,0x00); // Length of the pulse modulation in carrier clks+1  
		FSV9563_WriteReg(rRegTxSym10BurstLen,0x03); // Symbol 1 and 0 burst lengths = 8 bits.

		FSV9563_WriteReg(rRegFrameCon,0x01);

		FSV9563_WriteReg(rRegRxCtrl,0x05); // Set Rx Baudrate 212 kBaud 
		FSV9563_WriteReg(rRegRxThreshold,0x5c); // Set min-levels for Rx and phase shift  0x3C //5c
		FSV9563_WriteReg(rRegRcv,0x12);
		FSV9563_WriteReg(rRegRxAna,0x0f); //0x0a
		
		FSV9563_WriteReg(rRegRxWait,0x86);
		
		FSV9563_WriteReg(rRegDrvMod,0x87);
		FSV9563_WriteReg(rRegStatus,0);
		//>  FieldOn
		FSV9563_WriteReg(rRegDrvMod,0x8F);
	}
	else if(type=='V')
	{
		FSV9563_WriteReg(rRegWaterLevel,0x10);	//Set WaterLevel =(FIFO length -1)
										 
		FSV9563_WriteReg(rRegRxBitCtrl,0x80);	//Received bit after collision are replaced with 1.
		FSV9563_WriteReg(rRegDrvMod,0x89);	//Tx2Inv=1 0x80   //0x89
		FSV9563_WriteReg(rRegTxAmp,0x04);	//0	//0x04  //0x10
		FSV9563_WriteReg(rRegDrvCon,0x09);	//0x09
		FSV9563_WriteReg(rRegTxl,0x0A);	//0x05
		FSV9563_WriteReg(rRegRxSofD,0x00);	//
		
		FSV9563_CMD_LoadProtocol(0x0a,0x0A);	//A/B/C,A/C

		FSV9563_WriteReg(rRegIRQ0En,0);
		FSV9563_WriteReg(rRegIRQ1En,0);
		
		FSV9563_WriteReg(rRegFIFOControl,0xB0);
		
		FSV9563_WriteReg(rRegTxModWidth,0x00); // Length of the pulse modulation in carrier clks+1  
		FSV9563_WriteReg(rRegTxSym10BurstLen,0); // Symbol 1 and 0 burst lengths = 8 bits.

		FSV9563_WriteReg(rRegFrameCon,0x0F);

		FSV9563_WriteReg(rRegRxCtrl,0x02); // Set Rx Baudrate 26 kBaud 
		FSV9563_WriteReg(rRegRxThreshold,0x74); // Set min-levels for Rx and phase shift  0x4e  0x5C
		FSV9563_WriteReg(rRegRcv,0x12);
		FSV9563_WriteReg(rRegRxAna,0x07); //0xa 0x04
		
		FSV9563_WriteReg(rRegRxWait,0x9C);	//0x8C
		
		FSV9563_WriteReg(rRegDrvMod,0x81);   //0x81
		FSV9563_WriteReg(rRegStatus,0);
		FSV9563_WriteReg(rRegDrvMod,0x89);   //0x89
	}
	FSV9563_WriteReg(rRegTestMode,0xC0);
//	FSV9563_WriteReg(rRegTestInSel,0x66);	//rx
	FSV9563_WriteReg(rRegAUX1Sel,0x03);		//I(0x03),Q(0x01);
	FSV9563_WriteReg(rRegAUX2Sel,0x01);
//	FSV9563_WriteReg(rReg_6E,0x23);	//VIR:SCL; 3,4,6:SDA //0x23
//	FSV9563_WriteReg(rRegAUX1Sel,0x0A); //vref aux1
	FSV9563_WriteReg(rRegSigOut,0x09);	//RX-bit signal
	
	return MI_OK;
}

extern u8 Status_INT;
extern u8 Tx_Hander;
s8 FSV9563_PcdComTransceive(struct TranSciveBuffer *pi)
{
	s8 status= MI_ERR;
	u16 i;
	u8 reg1,temp,lastBits;	//reg0,
	u8 errReg;
	// Terminate any running command. 
	FSV9563_WriteReg(rRegCommand,FSV9563_Idle);
	FSV9563_SetBitMask(rRegFIFOControl,0x10);		//Flush_FiFo
	// Clear all IRQ 0,1 flags
	FSV9563_WriteReg(rRegIRQ0,0x7F);		
	FSV9563_WriteReg(rRegIRQ1,0x7F);

	for(i=0;i<pi->Length;i++)
		FSV9563_WriteReg(rRegFIFOData,pi->Data[i]);
	// Idle interrupt(Command terminated), FSV9563_BIT_IDLEIRQ=0x10
	FSV9563_WriteReg(rRegIRQ0En,0x08);	//IdleIRQEn,TxIRQEn
	//FSV9563_WriteReg(rRegIRQ1En,0x42);	//Global IRQ,Timer1IRQEn
	FSV9563_WriteReg(rRegIRQ1En,0x40);	//Global IRQ,Timer3IRQEn
	//>  Start FSV9563 command "Transcieve"=0x07. Activate Rx after Tx finishes.
	FSV9563_WriteReg(rRegCommand,pi->Command);	//FSV9563_Transceive
	
//	while(Tx_Hander == 0)
//	{
//		__NOP();
//	}
	do	
  {
      reg1 = FSV9563_ReadReg(rRegIRQ1);	  //07h //wait for TxIRQ
  }while((reg1&0x40)==0); //GlobalIRQ   //
	
	if(pi->Command == FSV9563_Transceive)
	{
		FSV9563_WriteReg(rRegIRQ0,0x7F);		
	    FSV9563_WriteReg(rRegIRQ1,0x7F);
		
		FSV9563_WriteReg(rRegIRQ0En,0x04);	//IdleIRQEn,RxIRQEn
		//FSV9563_WriteReg(rRegIRQ1En,0x42);	//Global IRQ,Timer1IRQEn
		FSV9563_WriteReg(rRegIRQ1En,0x40);	//Global IRQ,Timer3IRQEn
		for(i=8000;i>0;i--)
		{
			reg1 = FSV9563_ReadReg(rRegIRQ1);	  //07h  //wait for RxIRQ
			if(reg1&0x40) break;	//GlobalIRQ
		}
	}
	FSV9563_WriteReg(rRegIRQ0En,0);		
	FSV9563_WriteReg(rRegIRQ1En,0);

	errReg = FSV9563_ReadReg(rRegError);
	if(i==0)
	{
		status = MI_QUIT;
	}
	//else if(reg1&0x02)		//Timer1IRQ
//	else if(reg1&0x08)   //Timer3IRQ
//		status = MI_NOTAGERR;
	else if( errReg)	  //0Bh
	{
		if(errReg&0x04)
			status = MI_COLLERR;
		//读rxcoll 寄存器 看是在第几位发生碰撞
		else if(errReg&0x01)			//是错误的奇偶校验或错误 CRC，仍能接受数据
		{
			status = MI_FRAMINGERR;
			temp = FSV9563_ReadReg(rRegFIFOLength);		 //04h
			for (i=0; i<temp; i++)
					pi->Data[i] = FSV9563_ReadReg(rRegFIFOData);  //05h
			pi->Length = temp*8;
		}
		else
			status = MI_ERR;
	}
	else
	{
		status = MI_OK;
		Status_INT=0;
		if (pi->Command == FSV9563_Transceive)
		{
				temp = FSV9563_ReadReg(rRegFIFOLength);		 //04h
				lastBits = FSV9563_ReadReg(rRegRxBitCtrl) & 0x07;	 //0ch  
				if (lastBits)
					pi->Length = (temp-1)*8 + lastBits;
				else
					pi->Length = temp*8;
				if (temp == 0)	temp = 1;
				if (temp > 250) temp = 250; //maxlen ...
				for (i=0; i<temp; i++)
					pi->Data[i] = FSV9563_ReadReg(rRegFIFOData);  //05h
		}
	}
	return status;
}
///////////////////////////////////////////////////////////////////////////////////////
s8 FSV9563_PcdRequestA(u8 *pTagType)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;

	FSV9563_WriteReg(rRegTxCrcPreset,0x18);
	FSV9563_WriteReg(rRegRxCrcPreset,0x18);
	FSV9563_WriteReg(rRegFrameCon,0xCF);
	FSV9563_WriteReg(rRegStatus,0);
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0xC0);	//  TxWaitStart at the end of Rx data
	FSV9563_WriteReg(rRegTxWaitLo,0x0B);	// Set min.time between Rx and Tx or between two Tx   

	FSV9563_WriteReg(rRegT0ReloadHi,0x08); 	//2196/fc
	FSV9563_WriteReg(rRegT0ReloadLo,0x94); 
	FSV9563_WriteReg(rRegT1ReloadHi,0); 
	FSV9563_WriteReg(rRegT1ReloadLo,0x40);	//timerout ~= 10ms
	
	FSV9563_WriteReg(rRegIRQ0,0x08);
	FSV9563_WriteReg(rRegRxWait,0x90);
	FSV9563_WriteReg(rRegTxDataNum,0x0F);  //7bit
	
	//> Send the ReqA command
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 1;
	ComData.Data[0] = PICC_REQIDL;
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if (status == MI_OK)
	{
		if(ComData.Length == 0x10)
		{
			*pTagType     = ComData.Data[0];
			*(pTagType+1) = ComData.Data[1];
		}
		else
			status = MI_VALERR;
	}
	//FSV9563_WriteReg(rRegTxDataNum,0x08);
	return status;
}

s8 FSV9563_PcdWakeUpA(u8 *pTagType)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;

	FSV9563_WriteReg(rRegTxCrcPreset,0x18);
	FSV9563_WriteReg(rRegRxCrcPreset,0x18);
	FSV9563_WriteReg(rRegFrameCon,0xCF);
	FSV9563_WriteReg(rRegStatus,0);
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0xC0);	//  TxWaitStart at the end of Rx data
	FSV9563_WriteReg(rRegTxWaitLo,0x0B);	// Set min.time between Rx and Tx or between two Tx   

	FSV9563_WriteReg(rRegT0ReloadHi,0x08); 	//2196/fc
	FSV9563_WriteReg(rRegT0ReloadLo,0x94); 
	FSV9563_WriteReg(rRegT1ReloadHi,0); 
	FSV9563_WriteReg(rRegT1ReloadLo,0x40);	//timerout ~= 10ms
	
	FSV9563_WriteReg(rRegIRQ0,0x08);
	FSV9563_WriteReg(rRegRxWait,0x90);
	FSV9563_WriteReg(rRegTxDataNum,0x0F);  //7bit
	
	//> Send the ReqA command
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 1;
	ComData.Data[0] = PICC_REQALL;
	
		FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
		FLAG_0;
		
	if (status == MI_OK)
	{
		if(ComData.Length == 0x10)
		{
			*pTagType     = ComData.Data[0];
			*(pTagType+1) = ComData.Data[1];
		}
		else
			status = MI_VALERR;
	}
	//FSV9563_WriteReg(rRegTxDataNum,0x08);
	return status;
}

s8 FSV9563_PcdAnticollA(u8 level,u8 *pSnr)
{
	s8 status ;
	u8 i;
	u8 ucBits,ucBytes;
	u8 snr_check = 0;
	u8 ucCollPosition = 0;
	u8 ucTemp;
	u8 ucSNR[5] = {0, 0, 0, 0 ,0};
	struct TranSciveBuffer ComData,*pi = &ComData;
	
	FSV9563_ClearBitMask(rRegTxCrcPreset,0x01);	//TxCRCEn,off 
	FSV9563_ClearBitMask(rRegRxCrcPreset,0x01);	//TxCRCEn,off
	FSV9563_WriteReg(rRegTxDataNum,0x08);
	do
	{
		ucBits = (ucCollPosition) % 8;		
		if (ucBits != 0)
		{
			ucBytes = ucCollPosition / 8 + 1;
			FSV9563_SetRawRC(rRegRxBitCtrl, 0x8f,ucBits<<4);		//把冲撞位写入RxBitCtrl[6:4]位
			FSV9563_SetRawRC(rRegTxDataNum, 0xf8,ucBits);       //发送最后一个字节位的数目
		}
		else
			 ucBytes = ucCollPosition / 8;

		ComData.Command = FSV9563_Transceive;
		ComData.Data[0] = level;	//PICC_ANTICOLL1;//0x93
		ComData.Data[1] = 0x20 + ((ucCollPosition / 8) << 4) + (ucBits & 0x0F);
		for (i=0; i<ucBytes; i++)
			ComData.Data[i + 2] = ucSNR[i];
		ComData.Length = ucBytes + 2;
		status = FSV9563_PcdComTransceive(pi);

		ucTemp = ucSNR[(ucCollPosition / 8)];
		if (status == MI_COLLERR)
		{
			for (i=0; i < 5 - (ucCollPosition / 8); i++)
				 ucSNR[i + (ucCollPosition / 8)] = ComData.Data[i+1];
			ucSNR[(ucCollPosition / 8)] |= ucTemp;
			ucCollPosition = ComData.Data[0];
		}
		else if (status == MI_OK)
		{
			for (i=0; i < (ComData.Length / 8); i++)
				 ucSNR[4 - i] = ComData.Data[ComData.Length/8 - i - 1];
			ucSNR[(ucCollPosition / 8)] |= ucTemp;
		}
	} while (status == MI_COLLERR);
		
	if (status == MI_OK)
	{
		for (i=0; i<4; i++)
		{   
			*(pSnr+i)  = ucSNR[i];
			snr_check ^= ucSNR[i];
		}
		if (snr_check != ucSNR[i])
			status = MI_COM_ERR;
	}
	return status;
}

s8 FSV9563_PcdSelectA(u8 level,u8 *pSnr,u8 *pSize)
{
	s8   status;
	u8   i,snr_check = 0;
	struct TranSciveBuffer ComData,*pi = &ComData;

	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//TxCRCEn On
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x01);	//RxCRCEn On
    
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 7;
	ComData.Data[0] = level;//PICC_ANTICOLL1;
	ComData.Data[1] = 0x70;
	for (i=0; i<4; i++)
	{
		snr_check ^= *(pSnr+i);
		ComData.Data[i+2] = *(pSnr+i);
	}
	ComData.Data[6] = snr_check;
	status = FSV9563_PcdComTransceive(pi);
	
	if (status == MI_OK)
	{
    if (ComData.Length != 0x8)
    	status = MI_BITCOUNTERR;
    else
    	*pSize = ComData.Data[0];
	}
	return status;
}
s8 FSV9563_PcdAuth(u8 *pSize)
{
	s8   status;
	u8   i,snr_check = 0;
	struct TranSciveBuffer ComData,*pi = &ComData;

	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//TxCRCEn On
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x01);	//RxCRCEn On
    
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = 0x0a;
	ComData.Data[1] = 0x00;

	status = FSV9563_PcdComTransceive(pi);
	
	if (status == MI_OK)
	{
    if (ComData.Length != 0x9)
    	status = MI_BITCOUNTERR;
    else
		{
//			for(i=0;i<ComData.Length;i++)
//			{
				*pSize = ComData.Data[0];
//			}
		}
	}
	return status;
}

s8 FSV9563_PcdMfcRead(u8   addr,u8 *pReaddata)
{
	s8 status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	
	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//on
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x00);	//off
	
  ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = PICC_READ;
	ComData.Data[1] = addr;
//	
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;	
	
  if (status == MI_OK)
	{
		if (ComData.Length != 0x90)
			status = MI_BITCOUNTERR;
		else
			memcpy(pReaddata, &ComData.Data[0], 16);
	}
	return status;
}

s8 FSV9563_PcdMfcWrite(u8   addr,u8 *pWritedata)
{
	s8 status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//on
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x00);	//off
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 6;
	ComData.Data[0] = PICC_WRITE;
	ComData.Data[1] = addr;
	status = FSV9563_PcdComTransceive(pi);
	if (status != MI_NOTAGERR)
	{
		if(ComData.Length != 4)
			status=MI_BITCOUNTERR;
		else
		{
			ComData.Data[0] &= 0x0F;
			switch (ComData.Data[0])
			{
				case 0x00:
					status = MI_NOTAUTHERR;
					break;
				case 0x0A:
					status = MI_OK;
					break;
				default:
					status = MI_CODEERR;
					break;
			}
		}
	}    
	if (status == MI_OK)
	{
		ComData.Command = FSV9563_Transceive;
		ComData.Length  = 16;
		memcpy(&ComData.Data[0], pWritedata, 16);
		status = FSV9563_PcdComTransceive(pi);
		if (status != MI_NOTAGERR)
		{
			ComData.Data[0] &= 0x0F;
			switch(ComData.Data[0])
			{
				case 0x00:
					status = MI_WRITEERR;
					break;
				case 0x0A:
					status = MI_OK;
					break;
				default:
					status = MI_CODEERR;
					break;
			}
		}
	}
	return status;
}

s8 FSV9563_PcdMfulRead(u8   addr,u8 *pReaddata)
{
	s8 status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	
	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//on
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x01);	//on
	
  ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = PICC_READ;
	ComData.Data[1] = addr;
	status = FSV9563_PcdComTransceive(pi);
  if(status == MI_OK)
	{
		if (ComData.Length != 0x80)
			status = MI_BITCOUNTERR;
		else
			memcpy(pReaddata, &ComData.Data[0], 16);
	}
//	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x00); // clear crc
	return status;
}

s8 FSV9563_PcdHaltA(void)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = PICC_HALT;
	ComData.Data[1] = 0;
	status = FSV9563_PcdComTransceive(pi);
	if(status == MI_NOTAGERR)		//halt command has no response
		status = MI_OK;
	else
		status = MI_ERR;
	return status;
}

////////////////////////////////////////T1T JEWEL
void UpdateCrc_B(u8 bCh, u16 *pLpwCrc)
{
    bCh = (bCh^(u8)((*pLpwCrc)&0x00FFU));
    bCh = (bCh ^ (bCh<<4U));
    *pLpwCrc = (*pLpwCrc >> 8U) ^ ((uint16_t)bCh << 8U) ^ ((u16)bCh << 3U) ^ ((u16)bCh>>4U);
}

void ComputeCrc_B( u8 *pData,u32 dwLength,u8 *pCrc)
{
    u8 bChBlock = 0;
    u16 wCrc = 0xFFFF;
    do
    {
        bChBlock = *pData++;
        UpdateCrc_B(bChBlock, &wCrc);
    } while (0u != (--dwLength));
    wCrc = ~wCrc;
    pCrc[0] = (u8) (wCrc & 0xFFU);
    pCrc[1] = (u8) ( (wCrc>>8U) & 0xFFU);
}
s8 FSV9563_PcdJewelCommand(u8* pdata,u8 len,u8* resp,u8* replen)
{
	s8 status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	u8 i;
	u8 crc[2];
	
	FSV9563_WriteReg(rRegFrameCon,0x4F); //TxParityEn off,RxParityEn on//4F
	FSV9563_WriteReg(rRegTxDataNum,0x0F);  //7bit
//	FLAG_1;
	for(i=0;i<len-1;i++)
	{
		ComData.Command = FSV9563_Transmit;
		ComData.Length  = 1;
		ComData.Data[0] = *(pdata+i);	
		status = FSV9563_PcdComTransceive(pi);
		if(status != MI_OK)
			return status;
		if(i==0)
			FSV9563_WriteReg(rRegTxDataNum,0x08);
	}
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 1;
	ComData.Data[0] = *(pdata+len-1);	
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if(status == MI_OK)
	{
		*replen = ComData.Length/8;
		if (*replen != 0)
		{
			memcpy(resp, &ComData.Data[0], *replen);
			ComputeCrc_B(resp,*replen-2,crc);
			if(crc[0]!=resp[*replen-2] || crc[1]!=resp[*replen-1])
				status = MI_CRCERR;
		}
	}
	return status;
}

s8 FSV9563_PcdRidA(u8 *pUid)		
{
	s8   status;
	u8 cmd_rid[9]={0x78,0,0,0,0,0,0};	//{0x78,0,0,0,0,0,0,0xd0,0x43};	
	u8 resp[8];
	u8 len;

	ComputeCrc_B(cmd_rid,7,&cmd_rid[7]);
	status = FSV9563_PcdJewelCommand(cmd_rid,sizeof(cmd_rid),resp,&len);
	printStatus("RID",status,resp,len);
	if(status==MI_OK)
	{
		memcpy(pUid, &resp[2], 4);
	}
	return status;
} 

s8 FSV9563_PcdReadA(u8 *pUid)		
{
	s8   status;
	u8 cmdbuf[9]={1,8,0};
	u8 resp[8];
	u8 len;

	memcpy(&cmdbuf[3],pUid,4);
	ComputeCrc_B(cmdbuf,7,&cmdbuf[7]);
	status = FSV9563_PcdJewelCommand(cmdbuf,sizeof(cmdbuf),resp,&len);
	printStatus("Read",status,resp,len);

	return status;
} 

s8 FSV9563_PcdJewel(void)
{
	s8   status;
	u8 uid[4];
	status = FSV9563_PcdRidA(uid);
	if(status!=MI_OK) return status;
	status = FSV9563_PcdReadA(uid);
	return status;
}
s8 FSV9563_PcdActivateA(u8 *patqa,u8 *puid,u8 *plen,u8 *psak)
{
	s8 status;

	FSV9563_PcdConfigISOType('A');
	delay_ms(10);
	
	status = FSV9563_PcdWakeUpA(patqa);
	
	
	printStatus("ATQA",status,patqa,2);
	if(status!=MI_OK) return status;

	if(*(patqa+1)==0x0C && *patqa==0 )	//T1T JEWEL TOPAZ512
	{
		*plen = 6;
		status = FSV9563_PcdJewel();
		if(status!=MI_OK) return status;
	}
	else	//other
	{
		*plen = 4;
		status = FSV9563_PcdAnticollA(PICC_ANTICOLL1,puid);
		printStatus("UID1",status,puid,4);
		if(status!=MI_OK) return status;

		status = FSV9563_PcdSelectA(PICC_ANTICOLL1,puid,psak);
		printStatus("SEL1",status,psak,1);
		if(status!=MI_OK) return status;
		
		if(*patqa&0xC0)	//level 2   判断bit7  bit8
		{
			*plen = 8;
			status = FSV9563_PcdAnticollA(PICC_ANTICOLL2,puid+4);
			printStatus("UID2",status,puid+4,4);
			if(status!=MI_OK) return status;

			status = FSV9563_PcdSelectA(PICC_ANTICOLL2,puid+4,psak);
			printStatus("SEL2",status,psak,1);
			if(status!=MI_OK) return status;
		}
		
		if(*patqa&0x80)	//level 3    判断bit7  bit8
		{
			*plen = 12;
			status = FSV9563_PcdAnticollA(PICC_ANTICOLL3,puid+8);
			printStatus("UID3",status,puid+8,4);
			if(status!=MI_OK) return status;

			status = FSV9563_PcdSelectA(PICC_ANTICOLL3,puid+8,psak);
			printStatus("SEL3",status,psak,1);
			if(status!=MI_OK) return status;
		}
	}
	return status;
}

u8 FSV9563_RatsA(u8 *pbuf,u8 *plen)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = 0xE0;	
	ComData.Data[1] = 0x51;	//default=0x51	Fsdi,CID

	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		*plen = ComData.Length/8;
    memcpy(pbuf, &ComData.Data[0], *plen);	//D81:06 75 77 81 02 80
	}
	return status;
}

u8 FSV9563_PpsA(u8 param,u8 *pPpss)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 3;
	ComData.Data[0] = 0xD1;	//0xD0 | CID	
	ComData.Data[1] = 0x11;	//default=0x51	bFsdi,bCid
	ComData.Data[2] = param;	//0x0A(424,424);	0x0F(848,848)

	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
    if (ComData.Length != 8)
    	status = MI_BITCOUNTERR;
    else
    	*pPpss = ComData.Data[0];	//
	}
	return status;
}

void FSV9563_SetBaudrate(u8 txrate,u8 rrate)
{
	switch(txrate)
	{
		case 0:	//106k
			FSV9563_WriteReg(rRegTxModWidth,0x20); //2fh,
			break;
		case 1:	//212k
			FSV9563_WriteReg(rRegTxModWidth,0x10); //2fh,
			break;
		case 2:	//424k
			FSV9563_WriteReg(rRegTxModWidth,0x07); //2fh,
			break;
		case 3:	//848k
			FSV9563_WriteReg(rRegTxModWidth,0x02); //2fh,
			break;
		default://106k
			FSV9563_WriteReg(rRegTxModWidth,0x20); //2fh,
			break;
	}
	switch(rrate)
	{
		case 0:	//106k
			FSV9563_WriteReg(rRegRxCtrl,0x04); //35h,
			break;
		case 1:	//212k
			FSV9563_WriteReg(rRegRxCtrl,0x05); //35h,
			break;
		case 2:	//424k
			FSV9563_WriteReg(rRegRxCtrl,0x06); //35h,
			break;
		case 3:	//848k
			FSV9563_WriteReg(rRegRxCtrl,0x07); //35h,
			break;
		default://106k
			FSV9563_WriteReg(rRegRxCtrl,0x04); //35h,
			break;
	}
}

u8 FSV9563_CPU_I_Block(u8 *psbuf,u8 slen,u8 *prbuf,u8 *prlen)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	FSV9563_WriteReg(rRegT0ReloadHi,0x41); 	//2196/fc 0x08
	FSV9563_WriteReg(rRegT0ReloadLo,0x0B); 	//0x94
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = slen+2;
	ComData.Data[0] = 0x0A;	//PCB	
	ComData.Data[1] = 0x01;	//Cid
	memcpy(&ComData.Data[2], psbuf, slen);
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		*prlen= ComData.Length/8;
		memcpy(prbuf, &ComData.Data[0], *prlen);
	}
	return status;
}

u8 FSV9563_NT3H2111Read(u8 page,u8 *prbuf,u8 *prlen)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	FSV9563_WriteReg(rRegT0ReloadHi,0x41); 	//2196/fc 0x08;0x41
	FSV9563_WriteReg(rRegT0ReloadLo,0x94); 	//0x94;0x0B
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 2;
	ComData.Data[0] = 0x30;	//PCB	:0x0A;
	ComData.Data[1] = page;	//Cid
//	ComData.Data[2] = 0x09;	//Cid
//	memcpy(&ComData.Data[2], psbuf,slen);
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if (status == MI_OK)
	{
		*prlen= ComData.Length/8;
		memcpy(prbuf, &ComData.Data[0], *prlen);
	}
	return status;
}

s8 FSV9563_NT3H2111Write(u8   addr,u8 *pWritedata)
{
	s8 status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	
	FSV9563_SetRawRC(rRegTxCrcPreset,0xfe,0x01);	//on
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x00);	//off
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 6;  //2
	ComData.Data[0] = 0xA2;  //PICC_WRITE
	ComData.Data[1] = addr;
	memcpy(&ComData.Data[2], pWritedata, 4);
	status = FSV9563_PcdComTransceive(pi);
	if (status != MI_NOTAGERR)
	{
		if(ComData.Length != 4)
			status=MI_BITCOUNTERR;
		else
		{
			ComData.Data[0] &= 0x0F; //0X0F
			switch (ComData.Data[0])
			{
				case 0x00:
					status = MI_NOTAUTHERR;
					break;
				case 0x0A:
					status = MI_OK;
					break;
				default:
					status = MI_CODEERR;
					break;
			}
		}
	}
	FSV9563_SetRawRC(rRegRxCrcPreset,0xfe,0x01);	//on
	return status;
}
u8 FSV9563_NT3H2111Test(u8 *prbuf,u8 *prlen)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;
	FSV9563_WriteReg(rRegT0ReloadHi,0x41); 	//2196/fc 0x08;0x41
	FSV9563_WriteReg(rRegT0ReloadLo,0x94); 	//0x94;0x0B
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 5;
	ComData.Data[0] = 0x1B;	
	memcpy(&ComData.Data[1], prbuf,4);
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		*prlen= ComData.Length/8;
		memcpy(prbuf, &ComData.Data[0], *prlen);
	}
	return status;

}
void FSV9563_NT3H2111(void)
{
			s8 status;
			u8 atqa[2];
			u8 uid[12],ulen;
			u8 sak;
		  u8 E8[16],E9[16],E5[16],E3[16],E4[16],E7[16];
			u8 CNT[4],CNT3[4];
	
			status = FSV9563_PcdActivateA(atqa,uid,&ulen,&sak);
			if(status!=MI_OK) return;
/**************************初始化TAG******************************/
			CNT[0] = 0x01;
			CNT[1] = 0x00;
			CNT[2] = 0x01;
			CNT[3] = 0xff;
			status = FSV9563_NT3H2111Write(0xE8,CNT); //配置寄存器
			printStatus("Write_e8",status,CNT,0);
			if(status!=MI_OK) return;		    
			
			CNT[0] = 0xff;
			CNT[1] = 0x01;
			CNT[2] = 0x00;
			CNT[3] = 0x00;
			status = FSV9563_NT3H2111Write(0xE9,CNT);//配置寄存器
			printStatus("Write_e9",status,CNT,0);
			if(status!=MI_OK) return;		    

			CNT[0] = 0x00;
			CNT[1] = 0x00;
			CNT[2] = 0x00;
			CNT[3] = 0x00;
			status = FSV9563_NT3H2111Write(0xE5,CNT);  //设置密钥
			printStatus("Write_e5",status,CNT,0);
			if(status!=MI_OK) return;	
			
			CNT3[0] = 0x00;
			CNT3[1] = 0x00;
			CNT3[2] = 0x00;
			CNT3[3] = 0x00;
			status = FSV9563_NT3H2111Test(CNT3,&ulen); //认证 commond 1B
			printStatus("Pass",status,CNT3,ulen);
			if(status!=MI_OK) return;

			CNT[0] = 0x00;
			CNT[1] = 0x00;
			CNT[2] = 0x00;
			CNT[3] = 0x01;
			status = FSV9563_NT3H2111Write(0xE3,CNT);
			printStatus("Write_e3",status,CNT,0);
			if(status!=MI_OK) return;				
			
			CNT[0] = 0x00;
			CNT[1] = 0x00;
			CNT[2] = 0x00;
			CNT[3] = 0x00;
			status = FSV9563_NT3H2111Write(0xE4,CNT);
			printStatus("Write_e4",status,CNT,0);
			if(status!=MI_OK) return;	
			
			CNT[0] = 0x04;
			CNT[1] = 0x00;
			CNT[2] = 0x00;
			CNT[3] = 0x00;
			status = FSV9563_NT3H2111Write(0xE7,CNT);
			printStatus("Write_e7",status,CNT,0);
			if(status!=MI_OK) return;	
	
	
			status = FSV9563_NT3H2111Read(0xE8,E8,&ulen);
			printStatus("Read_e8",status,E8,ulen);
			if(status!=MI_OK) return;

			status = FSV9563_NT3H2111Read(0xE9,E9,&ulen);
			printStatus("Read_e9",status,E9,ulen);
			if(status!=MI_OK) return;
			
			status = FSV9563_NT3H2111Read(0xE5,E5,&ulen);
			printStatus("Read_e5",status,E5,ulen);
			if(status!=MI_OK) return;			
			
			status = FSV9563_NT3H2111Read(0xE3,E3,&ulen);
			printStatus("Read_e3",status,E3,ulen);
			if(status!=MI_OK) return;		

			status = FSV9563_NT3H2111Read(0xE4,E4,&ulen);
			printStatus("Read_e4",status,E4,ulen);
			if(status!=MI_OK) return;

			status = FSV9563_NT3H2111Read(0xE7,E7,&ulen);
			printStatus("Read_e7",status,E7,ulen);
			if(status!=MI_OK) return;
			delay_ms(100);
				 
			if( E8[0] == 0x01 && E8[1] == 0x00 && E8[2] == 0x01 && E8[3] == 0xff 
					&& E9[0] == 0xff && E9[1] == 0x01 && E9[2] == 0x00 && E9[3] == 0x00
					&&E5[0] == 0x00 && E5[1] == 0x00&& E5[2] == 0x00 && E5[3] == 0x00 
					&& E3[0] == 0x00 && E3[1] == 0x00&& E3[2] == 0x00&& E3[3] == 0x01
				 && E4[0] == 0x00 && E4[1] == 0x00&& E4[2] == 0x00&& E4[3] == 0x00
				&& E7[0] == 0x04 && E7[1] == 0x00&& E7[2] == 0x00&& E7[3] == 0x00	
				)
				{
							LED_0;
					    delay_ms(500);
				    	LED_1;
				}
				else
				{

							LED_0;
					    delay_ms(100);

				}
/***********************************************************************************************/
}

void FSV9563_NT3H2111_Transfer(void)
{
		s8 status;
		u8 atqa[2];
		u8 uid[12],ulen;
		u8 sak,l;
	  u8 CNT1[16];
		status = FSV9563_PcdActivateA(atqa,uid,&ulen,&sak);
		if(status!=MI_OK) return;
	  delay_ms(500);
	
	 //判断nfc 来操作(发送数据)		
		 for(l = 0;l<10;l++)
		 {
			 status = FSV9563_NT3H2111Read(0xEC,CNT1,&ulen);
			 printStatus("Read_EC",status,CNT1,ulen);
			 if(status!=MI_OK) return;	
			 
			 if(CNT1[6] == 0x21)
			 {				 
				 break;					 
			 }		 
			delay_ms(50); 
		 }	 
		 if(l==10) return;
	

}

void FSV9563_TypeA(void)
{
	s8 status;
	u8 atqa[2];
	u8 uid[12],ulen;
	u8 sak;
	u8 au;
	u8 RD_Data[16];		
	static u8 KEY[6]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
	u8 ver1 = 0x60;
	u8 random[]={0x00,0x84,0x00,0x00,0x08};	//cpu_random
	u8 temp = 0,n,m;	
	PDOWN_1;
	delay_ms(1);
	PDOWN_0;
	delay_ms(5);	
	status = FSV9563_PcdActivateA(atqa,uid,&ulen,&sak);
	if(status!=MI_OK) return;
	if(ulen!=6)		//not topaz512
	{
		if(sak&0x04)
		{
			printf("uid not complete");
			return;
		}
		if(sak&0x20)	//compliant with ISO-14443-4	
		{
			status = FSV9563_RatsA(RD_Data,&ulen);
			printStatus("Rats",status,RD_Data,ulen);
			if(status!=MI_OK) return;
			
			if((RD_Data[1]&0x10)&&(RD_Data[2]&0x44)==0x44)	//TA(1) 848k supported	//mifare desfire
			{
				//848k		
				status = FSV9563_PpsA(0x00,&sak);
				printStatus("Pps",status,&sak,1);
				if(status!=MI_OK) return;
				FSV9563_CMD_LoadProtocol(0,0);
				FSV9563_SetBaudrate(0,0);	
			
				status = FSV9563_CPU_I_Block(&ver1,1,uid,&ulen);
				printStatus("ver1",status,uid,ulen);
				if(status!=MI_OK) return;
				
				//106k
				FSV9563_CMD_LoadProtocol(0,0);
				FSV9563_SetBaudrate(0,0);	
			}
			else
			{
				status = FSV9563_CPU_I_Block(random,5,uid,&ulen);
				printStatus("random",status,uid,ulen);
				if(status!=MI_OK) return;
			}
		}
		else if(ulen==4)	//not compliant with ISO-14443-4	
		{
			if(sak==0x08 && atqa[0]==0x04)
			{
				printf("mifare S50 has detected!\n");
			}
			else if(sak==0x18 && atqa[0]==0x02)
			{
				printf("mifare S70 has detected!\n");
			}

			status = FSV9563_CMD_LoadKey(KEY);
			printStatus("LoadKey",status,NULL,0);
			if(status!=MI_OK) return;
			
			status = FSV9563_CMD_MfcAuthenticate(0x60,0,uid);
			printStatus("Auth",status,NULL,0);
			if(status!=MI_OK) return;
		
			status=FSV9563_PcdMfcRead(1,RD_Data);
			printStatus("MfcRead",status,RD_Data,16);
			if(status!=MI_OK) return;

		}
		else
		{
			status = FSV9563_PcdMfulRead(3,RD_Data);
			printStatus("MfulRead",status,RD_Data,16);
			if(status!=MI_OK) return;

		}
	}
		
	do
	{
		FSV9563_FieldOff();
		delay_ms(10);
		FSV9563_FieldOn();
		LED_1;
		delay_ms(10);
		status = FSV9563_PcdRequestA(atqa);
//	status = FSV9563_PcdWakeUpA(atqa);
		if(status==MI_OK)
		{
			LED_0;
		}
		else 
			break;
	}while(status==MI_OK);
}
//////////////////////////////////////////////////////ID2
s8 FSV9563_PcdRequestB(u8 *pTagType,u8 *pLen)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;

	FSV9563_WriteReg(rRegTxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0xC1);	
	FSV9563_WriteReg(rRegTxWaitLo,0x0B);	

	FSV9563_WriteReg(rRegT0ReloadHi,0x08); 	//2196/fc
	FSV9563_WriteReg(rRegT0ReloadLo,0x94); 
	FSV9563_WriteReg(rRegT1ReloadHi,0); 
	FSV9563_WriteReg(rRegT1ReloadLo,0x30);	//timerout ~= 10ms
	
	FSV9563_WriteReg(rRegIRQ0,0x08);
	FSV9563_WriteReg(rRegRxWait,0x90);
	//> Send the ReqB command
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 3;
	ComData.Data[0] = PICC_ANTI;
	ComData.Data[1] = 0;
	ComData.Data[2] = 0;
	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
	FLAG_0;
	if (status == MI_OK || status == MI_FRAMINGERR)
	{
//		if((ComData.Length == 0x60) || (ComData.Length == 0x10))
//		{
			*pLen = ComData.Length/8;
			memcpy(pTagType, &ComData.Data[0], *pLen);
//		}
//		else
//			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_PcdAttribB(u8 *pPupi,u8 *pCid)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 9;
	ComData.Data[0] = PICC_ATTRIB;
//	memset(&ComData.Data[1], 0x00, 4);	//pupi
	memcpy(&ComData.Data[1], pPupi, 4);
	ComData.Data[5] = 0;	//
	ComData.Data[6] = 8;
	ComData.Data[7] = 1;
	ComData.Data[8] = 0;	//0 8
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
    if (ComData.Length != 0x8)
    	status = MI_BITCOUNTERR;
    else
    	*pCid = ComData.Data[0];
	}
	return status;
}

s8 FSV9563_PcdGetUidB(u8 *pUID)
{
	s8   status;
	struct TranSciveBuffer ComData,*pi = &ComData;

	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 5;
	ComData.Data[0] = 0;
	ComData.Data[1] = 0x36;
	ComData.Data[2] = 0;
	ComData.Data[3] = 0;
	ComData.Data[4] = 8;
	
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if(status == MI_OK)
	{
		if(ComData.Length == 0x50)
			memcpy(pUID, &ComData.Data[0], 10);
		else
			status = MI_BITCOUNTERR;
	}
	return status;
}
//////////////////////////////////////////////////////
s8 FSV9563_SRT512_Initate(u8 *p)
{
	s8   status;  
	struct TranSciveBuffer ComData,*pi = &ComData; 

	ComData.Command = FSV9563_Transceive;
	ComData.Length  =2;
	ComData.Data[0] =0x6; 
	ComData.Data[1] =0;
	status  = FSV9563_PcdComTransceive(pi);
	if(status == MI_OK)
	{		
		if(ComData.Length == 8)	 
			*p = ComData.Data[0];
		else
			status = MI_BITCOUNTERR;
	}
   
	return status;
}

s8 FSV9563_SRT512_Select(u8 *p)
{
	s8   status;  
	struct TranSciveBuffer ComData,*pi = &ComData; 

	ComData.Command = FSV9563_Transceive;
	ComData.Length  =2;
	ComData.Data[0] =0x0E; 
	ComData.Data[1] =*p;
	status  = FSV9563_PcdComTransceive(pi);
	if(status == MI_OK)
	{		
		if(ComData.Length == 8)	 
			*p = ComData.Data[0];
		else
			status = MI_BITCOUNTERR;
	}
	return status;
}

s8 FSV9563_SRT512_GetUID(u8 *p)
{
	s8   status;  
	struct TranSciveBuffer ComData,*pi = &ComData; 

	ComData.Command = FSV9563_Transceive;
	ComData.Length  =1;
	ComData.Data[0] =0x0B; 
	status  = FSV9563_PcdComTransceive(pi);
	if(status == MI_OK)
	{
		if(ComData.Length == 0x40) 	
			memcpy(p, &ComData.Data[0], 8);
		else
			status = MI_BITCOUNTERR;
	}
   
	return status;
}

void FSV9563_TypeB(void)
{
	s8 status;
	u8 i;//,temp;
	u8 RD_Data[12],Len;	
	PDOWN_1;
	delay_ms(1);
	PDOWN_0;
	delay_ms(5);
	FSV9563_PcdConfigISOType('B');
	delay_ms(10);
	///////////////////////////////////2nd ID

	status = FSV9563_PcdRequestB(RD_Data,&Len);
	//FLAG_0;
#ifdef UART_PRINT
	printf("ATQB: %d_",status);
	if(status==MI_OK)
	{
		for(i=0;i<Len;i++)
			printf(" %02X",RD_Data[i]);
	}
	printf("\n");
#endif
	if(status!=MI_OK) return;
	
	if(Len==12)
	{
		status = FSV9563_PcdAttribB(&RD_Data[1],RD_Data);
#ifdef UART_PRINT
		printf("SELECT:%d_",status);
		if(status==MI_OK)
			printf(" %02X",RD_Data[0]);
		printf("\n");
#endif
		if(status!=MI_OK) return;
	}
	
	status = FSV9563_PcdGetUidB(RD_Data);
#ifdef UART_PRINT
	printf("UID: %d_",status);
	if(status==MI_OK)
	{
		for(i=0;i<10;i++)
			printf(" %02X",RD_Data[i]);
	}
	printf("\n");
#endif

	while(1)
	{
		FSV9563_FieldOff();
		delay_ms(10);
		FSV9563_FieldOn();
		LED_1;
		delay_ms(10);
		status = FSV9563_PcdRequestB(RD_Data,&Len);
		if(status==MI_OK)
		{
			LED_0;
		}
		else 
			break;
	}
}

//////////////////////////////////////////////////////Felica
s8 FSV9563_PcdRequestF(u8 *pIDm,u8 *pPMm)
{
	s8 status; 
	u8 i;
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegTxCrcPreset,0x09); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x09); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0xC0);	//  TxWaitStart at the end of Rx data
	FSV9563_WriteReg(rRegTxWaitLo,0x00);	// Set min.time between Rx and Tx or between two Tx   

	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x08
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x03); 
	FSV9563_WriteReg(rRegT1ReloadLo,0x22);	//timerout ~= 10ms
	
	//> Send the ReqF command
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 6;
	ComData.Data[0] = 0x06;
	ComData.Data[1] = 0;
	ComData.Data[2] = 0xFF;
	ComData.Data[3] = 0xFF;
	ComData.Data[4] = 0;
	ComData.Data[5] = 0;
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		if(ComData.Length == 0x90)
		{

			memcpy(pIDm, &ComData.Data[2], 8);
			memcpy(pPMm, &ComData.Data[10], 8);
		}
		else
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_PcdReadF(u8 *pIDm,u8 *pDat)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x41); 	//2196/fc 0x08
	FSV9563_WriteReg(rRegT0ReloadLo,0x0B); 	//0x94
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 16;
	ComData.Data[0] = 0x10;		//len
	ComData.Data[1] = 0x06;
	memcpy(&ComData.Data[2],pIDm, 8);
	ComData.Data[10] = 1;			//bNumServices
	ComData.Data[11] = 0x0B;	//pServiceList..
	ComData.Data[12] = 0;
	ComData.Data[13] = 1;			//bTxNumBlocks
	ComData.Data[14] = 0x80;	//pBlockList..
	ComData.Data[15] = 0x01;
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		if(ComData.Length == 0xE8)		//
		{
			//0x1d,0x07,IDm(8),0,0,1,data(16)
			//memcpy(pIDm, &ComData.Data[2], 8);
			memcpy(pDat, &ComData.Data[13], 16);
		}
		else
			status = MI_VALERR;
	}
	return status;
}
s8 FSV9563_PcdWriteF(u8 *pIDm,u8 *pDat)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x08
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x02); 	//2196/fc 0x08
	FSV9563_WriteReg(rRegT1ReloadLo,0xA1); 	//0x94
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 32;
	ComData.Data[0] = 0x20;		//len
	ComData.Data[1] = 0x08;
	memcpy(&ComData.Data[2],pIDm, 8);
	ComData.Data[10] = 1;			//bNumServices
	ComData.Data[11] = 0x09;	//pServiceList..
	ComData.Data[12] = 0;
	ComData.Data[13] = 1;			//bTxNumBlocks
	ComData.Data[14] = 0x80;	//pBlockList..
	ComData.Data[15] = 0x01;
	memcpy(&ComData.Data[16],pDat, 16);
	status = FSV9563_PcdComTransceive(pi);
	if(status == MI_OK)
	{
		if(ComData.Length != 0x60)		//12
		{
			//0x0C,0x09,IDm(8),0,0
			status = MI_VALERR;
		}
	}
	return status;
}
void FSV9563_Felica(void)
{
	s8 status;
	u8 IDm[8],PMm[8],buf[16];	
	PDOWN_1;
	delay_ms(1);
	PDOWN_0;
	delay_ms(5);
	FSV9563_PcdConfigISOType('F');
	delay_ms(10);		
	///////////////////////////////////
	status = FSV9563_PcdRequestF(IDm,PMm);
	printStatus("ATQF",status,IDm,8);
	if(status!=MI_OK) return;
	
	status = FSV9563_PcdReadF(IDm,buf);
	printStatus("READ",status,buf,16);
	if(status!=MI_OK) return;
	
	/*delay_ms(100);
	buf[15]++;
	status = FSV9563_PcdWriteF(IDm,buf);
	printStatus("READ",status,NULL,0);
	if(status!=MI_OK) return;
	*/
	do
	{
		FSV9563_FieldOff();
		delay_ms(10);
		FSV9563_FieldOn();
		LED_1;
		delay_ms(10);
		status = FSV9563_PcdRequestF(IDm,PMm);
		if(status==MI_OK)
		{
			LED_0;
		}
		else 
			break;
	}while(status==MI_OK);
}
//////////////////////////////////////////////////////ISO15693
s8 FSV9563_PcdInventoryV(u8 *pDsfId,u8 *pUID)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegTxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0x88);	//  TxWaitStart at the end of Rx data 0xC0
	FSV9563_WriteReg(rRegTxWaitLo,0xA9);	// Set min.time between Rx and Tx or between two Tx   0
	// Set timeout for this command cmd. Init reload values for timers-0,1 
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x0080
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x03); 	//0x0232
	FSV9563_WriteReg(rRegT1ReloadLo,0x22);	//timerout ~= 10ms

	//> Send the ReqF command
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 3;
	ComData.Data[0] = 0x26;	//flags:NBSLOTS,no AFI
	ComData.Data[1] = 0x01;	//invetory
	//ComData.Data[2] = 0;	//afi
	ComData.Data[2] = 0;	//masklen
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if (status == MI_OK)
	{
		if(ComData.Length == 0x50)
		{
			*pDsfId = ComData.Data[1];
			memcpy(pUID, &ComData.Data[2], 8);
		}
		else
			status = MI_VALERR;
	}
	return status;
}
s8 FSV9563_SelectV(u8 *pUid)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x24); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0xEB); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x00);	
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 10;
	ComData.Data[0] = 0x22;	//bflag
	ComData.Data[1] = 0x25;	//PHPAL_SLI15693_SW_CMD_SELECT
	memcpy(&ComData.Data[2],pUid, 8);
	status = FSV9563_PcdComTransceive(pi);	//~5.5ms
	if (status == MI_OK)
	{
		if(ComData.Length ==0 || ComData.Data[0]!=0)
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_readV(u8 *pUid)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x24); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0xEB); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x00);	
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 10;
	ComData.Data[0] = 0x22;	//bflag
	ComData.Data[1] = 0x26;	//PHPAL_SLI15693_SW_CMD_SELECT
	memcpy(&ComData.Data[2],pUid, 8);
	status = FSV9563_PcdComTransceive(pi);	//~5.5ms
	if (status == MI_OK)
	{
		if(ComData.Length ==0 || ComData.Data[0]!=0)
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_StayQuiet(u8 *pUid)
{
	s8 status; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x24); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0xEB); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x00);	
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 10;
	ComData.Data[0] = 0x22;	//bflag
	ComData.Data[1] = 0x02;	//StayQuiet
	memcpy(&ComData.Data[2],pUid, 8);
//	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	if (status == MI_OK)
	{	
		if(ComData.Length !=0)	//should no response
			status = MI_VALERR;
	}
	else if(status == MI_NOTAGERR)	//timeout
	{
		status = MI_OK;
	}
	return status;
}
s8 FSV9563_ReadSingleBlockV(const u8 *pUid,u8 block,u8 *pDat,u8 *pLen)
{
	s8 status; 
	u8 k;
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x24); 	//2196/fc 0x0080
	FSV9563_WriteReg(rRegT0ReloadLo,0xEB); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x00); 	//0x0232
	FSV9563_WriteReg(rRegT1ReloadLo,0x00);	//timerout ~= 10ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0x20;	//PHAL_ICODE_CMD_READ_SINGLE_BLOCK
//	for(k =0;k < 8;k++)
//	{
//		ComData.Data[2+k] = pUid[k];
//	}
	ComData.Data[2] = block;
	
//  FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
//	FLAG_0;
	
	if (status == MI_OK)
	{
		if(ComData.Length !=0)//== 0x50)
		{
			*pLen = ComData.Length/8-1;
			memcpy(pDat, &ComData.Data[1], *pLen);
		}
		else
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_WriteSingleBlockV(u8 block,u8 *pDat,u8 Len)
{
	s8 status; 
   Len = 4;
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = Len+3;  //Len+3
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0x21;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = block;
	
	
	memcpy(&ComData.Data[3],pDat,Len);

	
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		if(ComData.Length ==0 || ComData.Data[0]!=0)
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_GetRandomNumber(const u8 *pUid,u8 *pRandom,u8 *pLen)
{
	s8 status; 
  u8 k;
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 11;
	ComData.Data[0] = 0x22;	//bflag
	ComData.Data[1] = 0xb2;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
	for(k =0;k < 8;k++)
	{
		ComData.Data[3+k] = pUid[k];
	}
	
	status = FSV9563_PcdComTransceive(pi);
	
//	
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
//	return status;
	if (status == MI_OK)
	{
		if(ComData.Length !=0)//== 0x50)
		{
			*pLen = ComData.Length/8 ;
			memcpy(pRandom, &ComData.Data[0], *pLen);
		}
		else
			status = MI_VALERR;
	}
  return status;
}

 s8 FSV9563_SetPassWard(const u8 *pUid,u8 *pPwd,u8 *pRandom,u8 *pLen)
{
	s8 status; 
  u8 k;
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =8;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xb3;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//	for(k =0;k < 8;k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	
	ComData.Data[3] = 0x01;
	ComData.Data[4] = pPwd[0] ^ pRandom[0];
	ComData.Data[5] = pPwd[1] ^ pRandom[1];
	ComData.Data[6] = pPwd[2] ^ pRandom[0];
	ComData.Data[7] = pPwd[3] ^ pRandom[1];
	
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		if(ComData.Length !=8 || ComData.Data[0]!=0)
		status = MI_VALERR;
	}

		if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pRandom, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_WritePassWard(const u8 *pUid,u8 *Ppassword, u8 *pDat,u8 *pLen)
{
	s8 status,i; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =8;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xb4;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(i = 0;i < 8; i++)
//	{
//		ComData.Data[3+i] = pUid[i];
//	}
	
	ComData.Data[3] = 0x01;
	
	ComData.Data[4] = Ppassword[0];
	ComData.Data[5] = Ppassword[1];
	ComData.Data[6] = Ppassword[2];
	ComData.Data[7] = Ppassword[3];
	
	
	status = FSV9563_PcdComTransceive(pi);
	
		if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
//	return status;
}


//s8 FSV9563_PassWardProtect(const u8 *pUid)
//{
//	s8 status,i; 

//	struct TranSciveBuffer ComData,*pi= &ComData;
//	
//	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
//	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
//	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
//	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
//	
//	ComData.Command = FSV9563_Transceive;
//	ComData.Length  =11;
//	ComData.Data[0] = 0x12;	//bflag
//	ComData.Data[1] = 0xa6;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
//	ComData.Data[2] = 0x04;
//		for(i = 0;i < 8; i++)
//	{
//		ComData.Data[3+i] = pUid[i];
//	}
//	
//	status = FSV9563_PcdComTransceive(pi);
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
//	return status;
//}

s8 FSV9563_ProtectPage(const u8 *pUid,u8 Page,u8 ProtectionStatus,u8 *pDat,u8 *pLen)
{
	s8 status,k; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =5;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xc6;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	ComData.Data[3] = Page;
	ComData.Data[4] = ProtectionStatus;
	
	status = FSV9563_PcdComTransceive(pi);
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_GetProtectionStatus(const u8 *pUid,u8 *pDat,u8 *pLen)
{
	s8 status,k; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xc8;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//	for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
//	status = FSV9563_PcdComTransceive(pi);
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
  status = FSV9563_PcdComTransceive(pi);
	
	if (status == MI_OK)
	{
		if(ComData.Length !=0)//== 0x50)
		{
			*pLen = ComData.Length/8;
			memcpy(pDat, &ComData.Data[0], *pLen);
		}
		else
			status = MI_VALERR;
	}
	return status;
}

s8 FSV9563_LockPageProtectionCondition(const u8 *pUid,u8 *pDat,u8 *pLen)
{
	s8 status,k; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xc7;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}

	status = FSV9563_PcdComTransceive(pi);
//	if (status == MI_OK)
//	{
//		if(ComData.Length ==0 || ComData.Data[0]!=0)
//			status = MI_VALERR;
//	}
	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_SetEAS(const u8 *pUid,u8 *pDat,u8 *pLen)
{
	s8 status,k; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xa2;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	
	status = FSV9563_PcdComTransceive(pi);

	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_ResetEAS(const u8 *pUid,u8 *pDat,u8 *pLen)
{
	s8 status,k; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xa3;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	
	status = FSV9563_PcdComTransceive(pi);

	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_EasAlarm(const u8 *pUid,u8 *pEasSequence,u8 *pLen)
{
	s8 status,k; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =3;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xa5;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	
	status = FSV9563_PcdComTransceive(pi);

	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pEasSequence, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_FastInventoryRead(const u8 *pUid,u8 MaskLength,u8 MaskValue,u8 FirstBlockNumber,u8 NumberOfBlock,u8 *pDat,u8 *pLen)
{
	s8 status,k; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegTxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0x88);	//  TxWaitStart at the end of Rx data 0xC0
	FSV9563_WriteReg(rRegTxWaitLo,0xA9);	// Set min.time between Rx and Tx or between two Tx   0
	// Set timeout for this command cmd. Init reload values for timers-0,1 
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x0080
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x03); 	//0x0232
	FSV9563_WriteReg(rRegT1ReloadLo,0x22);

	
	
	FSV9563_WriteReg(rRegRxCtrl,0x03);
	FSV9563_CMD_LoadProtocol(0x0b,0x0A);
//	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
//	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
//	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
//	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 7;
	ComData.Data[0] = 0x66;	//bflag
	ComData.Data[1] = 0xa1;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
	ComData.Data[3] = MaskLength;
	ComData.Data[4] = MaskValue;
	ComData.Data[5] = FirstBlockNumber;
	ComData.Data[6] = NumberOfBlock;
	
	
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
  FLAG_0;
	
	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
		
		FSV9563_WriteReg(rRegRxCtrl,0x02);
		FSV9563_CMD_LoadProtocol(0x0a,0x0A);
	return status;
}

s8 FSV9563_InventoryRead(const u8 *pUid,u8 MaskLength,u8 MaskValue,u8 FirstBlockNumber,u8 NumberOfBlock,u8 *pDat,u8 *pLen)
{
	s8 status,k; 
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegTxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0x88);	//  TxWaitStart at the end of Rx data 0xC0
	FSV9563_WriteReg(rRegTxWaitLo,0xA9);	// Set min.time between Rx and Tx or between two Tx   0
	// Set timeout for this command cmd. Init reload values for timers-0,1 
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x0080
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x03); 	//0x0232
	FSV9563_WriteReg(rRegT1ReloadLo,0x22);

	
	
//	FSV9563_WriteReg(rRegRxCtrl,0x03);
//	FSV9563_CMD_LoadProtocol(0x0b,0x0A);
//	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
//	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
//	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
//	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 7;
	ComData.Data[0] = 0x66;	//bflag  16bit
	ComData.Data[1] = 0xa0;	//Inventory Read  commond code  16bit
	ComData.Data[2] = 0x04; // IC Mfg code  16bit
	ComData.Data[3] = MaskLength; // 0x08   16bit
	ComData.Data[4] = MaskValue;  // 0x12   16bit
	ComData.Data[5] = FirstBlockNumber;  // 0x05  16bit
	ComData.Data[6] = NumberOfBlock;    //  0x02  16bit  
	
	
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	FLAG_1;
	status = FSV9563_PcdComTransceive(pi);
  FLAG_0;
	
	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
		
//		FSV9563_WriteReg(rRegRxCtrl,0x02);
//		FSV9563_CMD_LoadProtocol(0x0a,0x0A);
	return status;
}
s8 FSV9563_WriteTriggerMode(const u8 *pUid,u8 Mode ,u8 *pDat,u8 *pLen)
{
	s8 status; 

	struct TranSciveBuffer ComData,*pi= &ComData;
	
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	
	FSV9563_WriteReg(rRegT1ReloadHi,0x08); 	
	FSV9563_WriteReg(rRegT1ReloadLo,0x6E);	//timerout ~= 20ms
	
	ComData.Command = FSV9563_Transceive;
	ComData.Length  =4;
	ComData.Data[0] = 0x12;	//bflag
	ComData.Data[1] = 0xc5;	//PHAL_ICODE_CMD_WRITE_SINGLE_BLOCK
	ComData.Data[2] = 0x04;
	ComData.Data[3] = Mode;
//		for(k = 0;k < 8; k++)
//	{
//		ComData.Data[3+k] = pUid[k];
//	}
	
	status = FSV9563_PcdComTransceive(pi);

	if (status == MI_OK)
		{
			if(ComData.Length !=0)//== 0x50)
			{
				*pLen = ComData.Length/8;
				memcpy(pDat, &ComData.Data[0], *pLen);
			}
			else
				status = MI_VALERR;
		}
	return status;
}

s8 FSV9563_Unlock(void)
{
	u8 dat[4],len ;
	s8 status; 
	
//	 status = FSV9563_ReadSingleBlockV(0x4a,dat,&len);
//   printStatus("ReadBlock_4B",status,dat,len);
//  	if(status!=MI_OK) return 2;
	
	 dat[0] = 0x35;		//open
	 dat[1] = 0xcb;
	 dat[2] = 0x00;
	 dat[3] = 0x00;
	
	 delay_ms(10);
	
	 status = FSV9563_WriteSingleBlockV(0x4A,dat,len);
	 printStatus("Write_4A",status,NULL,0);
	 if(status!=MI_OK) return 1;
	 
//	for(i=0;i<3;i++)
//	{
//	delay_ms(100);
//	 status = FSV9563_ReadSingleBlockV(0x52,dat,&len);
//	 printStatus("ReadBlock_52",status,dat,len);
//	 if(status!=MI_OK) return 2;
//	 if(dat[3]==0x00)
//		 return 0;
//	}

    return 0;
}
//s8 FSV9563_EnableLock(void)
//{
//	u8 dat[4],i,len;
//	s8 status;
//	
//	 dat[0] = 0x73;		//close 
//	 dat[1] = 0x8d;
//	 dat[2] = 0x00;
//	 dat[3] = 0x00;
//	 status = FSV9563_WriteSingleBlockV(0x4A,dat,len);
//	 printStatus("Write2",status,NULL,0);
//	 if(status!=MI_OK) return 1;
//		 
//	for(i=0;i<3;i++)
//	{
//	delay_ms(100);
//	 status = FSV9563_ReadSingleBlockV(0x4A,dat,&len);
//	 printStatus("ReadBlock2",status,dat,len);
//	 if(status!=MI_OK) return 2;
//	 if(dat[3]==0x00)
//		 return 0;
//	}
//	return 3;
//}

//s8 FSV9563_GetLockStatus(void)
//{
//	u8 dat[4],len;
//	s8 status;
//	
//	dat[0] = 0xa3;		//get status
//	dat[1] = 0x5d;
//	dat[2] = 0x00;
//	dat[3] = 0x00;
//	status = FSV9563_WriteSingleBlockV(0x4A,dat,len);
//	printStatus("Write_4A",status,NULL,0);
//	if(status!=MI_OK) return 1;
//	
//	status = FSV9563_WriteSingleBlockV(0x4F,dat,len);
//	printStatus("Write_4F",status,NULL,0);
//	if(status!=MI_OK) return 2;
//	
//	delay_ms(200);
//	
//	status = FSV9563_ReadSingleBlockV(uid,0x4B,dat,&len);
//	printStatus("ReadBlock_4B",status,dat,len);
//	if(status!=MI_OK) return 3;
//	
//	status = FSV9563_ReadSingleBlockV(uid,0x4F,dat,&len);
//	printStatus("ReadBlock_4F",status,dat,len);
//	if(status!=MI_OK) return 4;
	
	
//	for(i=0;i<3;i++)
//	{
//		delay_ms(150);
//		status = FSV9563_ReadSingleBlockV(0x4B,dat,&len);
//		printStatus("ReadBlock2",status,dat,len);
//		if(status!=MI_OK) return 2;
//		if(dat[0]==0x53&&dat[1]==0xad)		//open status
//		{
//			*pflag=0;
//			return 0;
//		}
//		if(dat[0]==0x63&&dat[1]==0x9d)		//lock status
//		{
//			*pflag=1;
//			return 0;
//		}
//	}
//    return 0;
//}
 void FSV9563_PassWordInit(void)
 {
	s8 status,writeflag = 0,i;
	u8 DsfId,uid[8],page=0x00,protectionstatus=0x00;
	u8 dat[16],len;
	u8 password[10]={0x01,0x00,0x00,0x00}, SetPassword[10]={0x00,0x00,0x00,0x00};
//	u8 EasSequence[40];
//  u8 MaskLength;
//	u8 MaskValue;
//	u8 FirstBlockNumber;
//	u8 NumberOfBlock;
			FSV9563_FieldOff();
			delay_ms(10);

			FSV9563_FieldOn();
			delay_ms(100);

		///////////////////////////////////Type V/ISO15693
			status = FSV9563_PcdInventoryV(&DsfId,uid);
			printStatus("Inventory",status,uid,8);
			if(status!=MI_OK) return;
			
			status = FSV9563_SelectV(uid);
			printStatus("Select",status,NULL,0);
			if(status!=MI_OK) return;
	
			status = FSV9563_GetRandomNumber(uid,dat, &len);
			printStatus("Random",status,dat,len);
			if(status!=MI_OK) return;
			
      memcpy(SetPassword,password,sizeof(password));
			
			status = FSV9563_SetPassWard(uid,SetPassword,&dat[1],&len);
			printStatus("SetPassWard",status,&dat[1],len);
			if(status!=MI_OK) return;	
	
	 if(writeflag == 0)
	 {
		 writeflag = 1;
			status =  FSV9563_WritePassWard(uid,password,dat,&len);
			printStatus("WritePassWard",status,dat,len);
			if(status!=MI_OK) return;	
		 
		 	memcpy(SetPassword,password,sizeof(password));
			
			status = FSV9563_GetRandomNumber(uid,dat, &len);
		  printStatus("Random",status,dat,len);
		  if(status!=MI_OK) return;
			
			status = FSV9563_SetPassWard(uid,SetPassword,&dat[1],&len);
			printStatus("SetPassWard",status,&dat[1],len);
			if(status!=MI_OK) return;
		 
		 	page = 0x05;
			protectionstatus = 0x01;
			status =  FSV9563_ProtectPage(uid,page,protectionstatus,dat,&len);
			printStatus("ProtectPage",status,dat,len);
			if(status!=MI_OK) return;	
//			
//			status = FSV9563_LockPageProtectionCondition(uid,dat,&len);
//			printStatus("LockPageProtectionCondition",status,dat,len);
//			if(status!=MI_OK) return;

			for(i = 0;i < 4;i++ )
			{
//				delay_ms(500);
				LED_Toggle;
				delay_ms(500);
			}
	 }
	 
	 delay_ms(500);
	 
		do
		{
			FSV9563_FieldOff();
			delay_ms(10);
			FSV9563_FieldOn();
			LED_1;
			delay_ms(10);
			status = FSV9563_PcdInventoryV(&DsfId,uid);
			if(status==MI_OK)
			LED_0;
			else 
				break;
		}while(status==MI_OK);
	
 }

 int i = 0,s =0;
 int  j = 0, c = 0,parameter1,parameter2,parameter3,parameter4;
 
 void FSV9563_Trigger(int *Temp)
 {
	s8 status;
	u8 DsfId,uid[8],page=0x00,protectionstatus=0x00;
	u8 dat[30],len,random[10];
	u8 password[5]={0x00,0x00,0x00,0x00}, SetPassword[5]={0x01,0x00,0x00,0x00};
	u8 EasSequence[40];
  u8 MaskLength;
	u8 MaskValue;
	u8 FirstBlockNumber;
	u8 NumberOfBlock,mode;
	int command,ret;
	
//	if(i == 0 )
//	 {
//		  i = 1;
//		  FSV9563_FieldOff();
//			delay_ms(10);

//			FSV9563_FieldOn();
//			delay_ms(100);

//		///////////////////////////////////Type V/ISO15693
//			status = FSV9563_PcdInventoryV(&DsfId,uid);
//			printStatus("Inventory",status,uid,8);
//			if(status!=MI_OK) return;
//			
//			status = FSV9563_SelectV(uid);
//			printStatus("Select",status,NULL,0);
//			if(status!=MI_OK) return;
//		 
//		  mode = 0x03;
//		  FSV9563_WriteTriggerMode(uid,mode,dat,&len);
//			printStatus("Select",status,NULL,0);
//			if(status!=MI_OK) return;
//		 
//		 	status = FSV9563_ReadSingleBlockV(uid,0x1f,dat,&len);
//			printStatus("ReadBlock_1f",status,dat,len);
//			if(status!=MI_OK) return;		
//	 }

//	 ret = scanf("%x",&i);
//	  printf("%x",0xa4);
//	 while(ret != 1)
//	 {
//		 ret = scanf("%d",&i);
//	 }
//   printf("%d",i);
	 
	 if(i == 0 && *Temp == 0)
	 {
		 i = 1;
		  FSV9563_FieldOff();
			delay_ms(10);

			FSV9563_FieldOn();
			delay_ms(100);

		///////////////////////////////////Type V/ISO15693
			status = FSV9563_PcdInventoryV(&DsfId,uid);
			printStatus("Inventory",status,uid,8);
			if(status!=MI_OK) return;
			
			status = FSV9563_SelectV(uid);
			printStatus("Select",status,NULL,0);
			if(status!=MI_OK) return;
		 
		  status = FSV9563_GetRandomNumber(uid,dat, &len);
		  printStatus("Random",status,dat,len);
		  if(status!=MI_OK) return;
			
			status = FSV9563_SetPassWard(uid,SetPassword,&dat[1],&len);
			printStatus("SetPassWard",status,&dat[1],len);
			if(status!=MI_OK) return;
		 
		  mode = 0x01;
		  status = FSV9563_WriteTriggerMode(uid,mode,dat,&len);
			printStatus("WriteTriggerMode",status,dat,len);
			if(status!=MI_OK) return;
		 *Temp = 1;

	 }
	 i = 1;
	 if(i==1 && *Temp == 1)
	 { 
		 i = 2;
		 FSV9563_FieldOff();
			delay_ms(10);

			FSV9563_FieldOn();
			delay_ms(100);

		///////////////////////////////////Type V/ISO15693
			status = FSV9563_PcdInventoryV(&DsfId,uid);
			printStatus("Inventory",status,uid,8);
			if(status!=MI_OK) return;
			
			status = FSV9563_SelectV(uid);
			printStatus("Select",status,dat,len);
			if(status!=MI_OK) return;
		 
		 		  delay_ms(200);
			
			status = FSV9563_readV(uid);
			printStatus("readV",status,dat,len);
			if(status!=MI_OK) return;
	 }
 }

void FSV9563_TAG(void)
 {
	s8 status;
	u8 DsfId,uid[8],page=0x00,protectionstatus=0x00;
	u8 dat[30],len,random[10];
	u8 password[5]={0x00,0x00,0x00,0x00}, SetPassword[5]={0x01,0x00,0x00,0x00};
	u8 EasSequence[40];
  u8 MaskLength;
	u8 MaskValue;
	u8 FirstBlockNumber;
	u8 NumberOfBlock;
	int command,ret;
	 if(i == 0 )
	 {
		  i = 2;
		  FSV9563_FieldOff();
			delay_ms(10);

			FSV9563_FieldOn();
			delay_ms(100);

		///////////////////////////////////Type V/ISO15693
//			status = FSV9563_PcdInventoryV(&DsfId,uid);
//			printStatus("Inventory",status,uid,8);
//			if(status!=MI_OK) return;
//			
//			status = FSV9563_SelectV(uid);
//			printStatus("Select",status,NULL,0);
//			if(status!=MI_OK) return;
//		 
//		 		 if(s == 1)
//		 {
//			 s = 0;
//      status = FSV9563_GetRandomNumber(uid,dat, &len);
//		  printStatus("Random",status,dat,len);
//		  if(status!=MI_OK) return;
//			
//			status = FSV9563_SetPassWard(uid,SetPassword,&dat[1],&len);
//			printStatus("SetPassWard",status,&dat[1],len);
//			if(status!=MI_OK) return;
//		 }
//		 
//		 	status = FSV9563_ReadSingleBlockV(uid,0x10,dat,&len);
//			printStatus("ReadBlock_10",status,dat,len);
//			if(status!=MI_OK) return;
//		 
//		 	 dat[0] = 0x10;		
//			 dat[1] = 0x00;
//			 dat[2] = 0x00;
//			 dat[3] = 0x00;
//			 status = FSV9563_WriteSingleBlockV(0x10,dat,len);
//			 printStatus("Write_10",status,NULL,0);
//			 if(status!=MI_OK) return ;		 
//			 
//	    status = FSV9563_ReadSingleBlockV(uid,0x10,dat,&len);
//			printStatus("ReadBlock_10",status,dat,len);
//			if(status!=MI_OK) return;

		  MaskLength = 0x08;
		  MaskValue  = 0x12;  //d4
			FirstBlockNumber =0x05;
			NumberOfBlock = 2;
      status = FSV9563_InventoryRead(uid,MaskLength,MaskValue,FirstBlockNumber,NumberOfBlock,dat,&len);
			printStatus("InventoryRead_",status,dat,len);
			if(status!=MI_OK) return;	
			
		  MaskLength = 0x08;
		  MaskValue  = 0x12;  //d4
			FirstBlockNumber =0x05;
			NumberOfBlock = 2;
		 	status = FSV9563_FastInventoryRead(uid,MaskLength,MaskValue,FirstBlockNumber,NumberOfBlock,dat,&len);
			printStatus("FastReadBlock_",status,dat,len);
			if(status!=MI_OK) return;		 
			

//				
//		 
//		 	 dat[0] = 0x40;		
//			 dat[1] = 0x00;
//			 dat[2] = 0x00;
//			 dat[3] = 0x00;
//			 status = FSV9563_WriteSingleBlockV(0x40,dat,len);
//			 printStatus("Write_40",status,NULL,0);
//			 if(status!=MI_OK) return ;
			  
//			 
//		  status = FSV9563_ReadSingleBlockV(uid,0x05,dat,&len);
//			printStatus("ReadBlock_05",status,dat,len);
//			if(status!=MI_OK) return;
//		 

		 

//		 

			

		if(c == 1)
	{		
      c = 0;
		
			status = FSV9563_GetRandomNumber(uid,dat, &len);
			printStatus("Random",status,dat,len);
			if(status!=MI_OK) return;
			
	//    memcpy(random, &dat[1], len);

			status = FSV9563_SetPassWard(uid,SetPassword,&dat[1],&len);
			printStatus("SetPassWard",status,&dat[1],len);
			if(status!=MI_OK) return;	
	////

//			status =  FSV9563_WritePassWard(uid,password,dat,&len);
//			printStatus("WritePassWard",status,dat,len);
//			if(status!=MI_OK) return;	
//			
//			status = FSV9563_GetRandomNumber(uid,dat, &len);
//		  printStatus("Random",status,dat,len);
//		  if(status!=MI_OK) return;
//			
//			status = FSV9563_SetPassWard(uid,SetPassword,&dat[1],&len);
//			printStatus("SetPassWard",status,&dat[1],len);
//			if(status!=MI_OK) return;
			
			page = 0x00;
			protectionstatus = 0x01;
			status =  FSV9563_ProtectPage(uid,page,protectionstatus,dat,&len);
			printStatus("ProtectPage",status,dat,len);
			if(status!=MI_OK) return;	
			
//			status = FSV9563_LockPageProtectionCondition(uid,dat,&len);
//			printStatus("LockPageProtectionCondition",status,dat,len);
//			if(status!=MI_OK) return;	
			
//			 dat[0] = 0x48;		
//			 dat[1] = 0x00;
//			 dat[2] = 0x04;
//			 dat[3] = 0x00;
//			 status = FSV9563_WriteSingleBlockV(0x33,dat,len);
//			 printStatus("Write_33",status,NULL,0);
//			 if(status!=MI_OK) return ;
		
//     status = FSV9563_GetProtectionStatus(uid,dat,&len);
//		 printStatus("GetProtectionStatus",status,dat,len);
//		 if(status!=MI_OK) return;
       
		}
	
//	    status = FSV9563_GetRandomNumber(uid,dat, &len);
//			printStatus("Random",status,dat,len);
//			if(status!=MI_OK) return;
//			
//			status = FSV9563_SetPassWard(uid,SetPassword,&dat[1],&len);
//			printStatus("SetPassWard",status,&dat[1],len);
//			if(status!=MI_OK) return;	
		
//			status = FSV9563_ReadSingleBlockV(uid,0x05,dat,&len);
//			printStatus("ReadBlock_05",status,dat,len);
//			if(status!=MI_OK) return;
		
////			 dat[0] = 0x05;		
////			 dat[1] = 0x05;
////			 dat[2] = 0x05;
////			 dat[3] = 0x05;
////			 status = FSV9563_WriteSingleBlockV(0x05,dat,len);
////			 printStatus("Write_05",status,NULL,0);
////			 if(status!=MI_OK) return ;
////		
////		  status = FSV9563_ReadSingleBlockV(uid,0x05,dat,&len);
////			printStatus("ReadBlock_05",status,dat,len);
////			if(status!=MI_OK) return;
//			
			
		 }

//     ret = scanf("%d",&i);
//		 if(ret != 1 )
//		 {
//			  while(getchar() !='\n' );
//			  printf("input i value\n");
//			  ret = scanf("%d",&i);			 
//		 }

	 
	 if(i == 1)
	 {

		 
		  i =4;
		 
		 if(j == 1)
		 {
			  j = 0;		 

//			 
				 
		    dat[0] = 0x35;		//open
	      dat[1] = 0xcb;
	      dat[2] = 0x00;
	      dat[3] = 0x00;
	      status = FSV9563_WriteSingleBlockV(0x4A,dat,len);
	      printStatus("Write-4a",status,NULL,0);
	      if(status!=MI_OK) return ;
				

		 
		    status = FSV9563_WriteSingleBlockV(0x4F,dat,len);
	      printStatus("Write-4f",status,NULL,0);
	      if(status!=MI_OK) return ;
			
			 
		 }
		 
		 if(j == 2)
		 { 
			  j = 0;
//      status = FSV9563_GetRandomNumber(uid,dat, &len);
//		  printStatus("Random",status,dat,len);
//		  if(status!=MI_OK) return;
//			
//			status = FSV9563_SetPassWard(uid,SetPassword,&dat[1],&len);
//			printStatus("SetPassWard",status,&dat[1],len);
//			if(status!=MI_OK) return;
//			 
		    dat[0] = 0x73;		//close
	      dat[1] = 0x8d;
	      dat[2] = 0x00;
	      dat[3] = 0x00;
	      status = FSV9563_WriteSingleBlockV(0x4A,dat,len);
	      printStatus("Write-4a",status,NULL,0);
	      if(status!=MI_OK) return ;
		 
		    status = FSV9563_WriteSingleBlockV(0x4F,dat,len);
	      printStatus("Write-4f",status,NULL,0);
	      if(status!=MI_OK) return ;
			 
		 } 
			  if(j == 3)        
	   {  
			  j = 0;
//        status = FSV9563_GetRandomNumber(uid,dat, &len);
//		    printStatus("Random",status,dat,len);
//        if(status!=MI_OK) return;
//			
//			  status = FSV9563_SetPassWard(uid,SetPassword,&dat[1],&len);
//			  printStatus("SetPassWard",status,&dat[1],len);
//			  if(status!=MI_OK) return;
	      dat[0] = 0xa3;	
	      dat[1] = 0x5D;
	      dat[2] = 0x00;
	      dat[3] = 0x00;
	      status = FSV9563_WriteSingleBlockV(0x4A,dat,len);
	      printStatus("Write_4a",status,NULL,0);
	      if(status!=MI_OK) return ;
		 
		    status = FSV9563_WriteSingleBlockV(0x4F,dat,len);
	      printStatus("Write_4f",status,NULL,0);
	      if(status!=MI_OK) return ;
		 
		    delay_ms(350);
		 
				status = FSV9563_ReadSingleBlockV(uid,0x4b,dat,&len);
				printStatus("ReadBlock_4b",status,dat,len);
				if(status!=MI_OK) return; 
				 
				status = FSV9563_ReadSingleBlockV(uid,0x4f,dat,&len);
				printStatus("ReadBlock_4f",status,dat,len);
				if(status!=MI_OK) return;
				j = 0;
	    }
		  if(j == 4)
			{
				j = 0;
//				status = FSV9563_GetRandomNumber(uid,dat, &len);
//		    printStatus("Random",status,dat,len);
//        if(status!=MI_OK) return;
//			
//			  status = FSV9563_SetPassWard(uid,SetPassword,&dat[1],&len);
//			  printStatus("SetPassWard",status,&dat[1],len);
//			  if(status!=MI_OK) return;
				
				dat[0] = 0xa6;	
				dat[1] = 0x5a;
				dat[2] = 0x00;
				dat[3] = 0x00;		 
				status = FSV9563_WriteSingleBlockV(0x4a,dat,len);
				printStatus("Write_4a",status,NULL,0);
				if(status!=MI_OK) return ;
				
				  delay_ms(300);
							
			  status = FSV9563_ReadSingleBlockV(uid,0x4b,dat,&len);
				printStatus("ReadBlock_4b",status,dat,len);
				if(status!=MI_OK) return;
				
			}
//			if(j == 5) 
//			{
//				j = 0;
//				status = FSV9563_ReadSingleBlockV(uid,0x4A,dat,&len);
//	      printStatus("ReadBlock_4A",status,dat,len);
////	      if(status!=MI_OK) return;
//				
//				status = FSV9563_ReadSingleBlockV(uid,0x4B,dat,&len);
//	      printStatus("ReadBlock_4B",status,dat,len);
////	      if(status!=MI_OK) return;
//				
//				status = FSV9563_ReadSingleBlockV(uid,0x30,dat,&len);
//	      printStatus("ReadBlock_30",status,dat,len);
//				
//								status = FSV9563_ReadSingleBlockV(uid,0x31,dat,&len);
//	      printStatus("ReadBlock_31",status,dat,len);
//				
//								status = FSV9563_ReadSingleBlockV(uid,0x32,dat,&len);
//	      printStatus("ReadBlock_32",status,dat,len);
//				
//								status = FSV9563_ReadSingleBlockV(uid,0x33,dat,&len);
//	      printStatus("ReadBlock_33",status,dat,len);
//				
//								status = FSV9563_ReadSingleBlockV(uid,0x34,dat,&len);
//	      printStatus("ReadBlock_34",status,dat,len);
//				
//								status = FSV9563_ReadSingleBlockV(uid,0x50,dat,&len);
//	      printStatus("ReadBlock_50",status,dat,len);
//				
//								status = FSV9563_ReadSingleBlockV(uid,0x51,dat,&len);
//	      printStatus("ReadBlock_51",status,dat,len);
//				
//				status = FSV9563_ReadSingleBlockV(uid,0x52,dat,&len);
//	      printStatus("ReadBlock_52",status,dat,len);
////	      if(status!=MI_OK) return;
//			
//		   	status = FSV9563_ReadSingleBlockV(uid,0x53,dat,&len);
//	      printStatus("ReadBlock_53",status,dat,len);
////	      if(status!=MI_OK) return;
//				
//				status = FSV9563_ReadSingleBlockV(uid,0x32,dat,&len);
//	      printStatus("ReadBlock_32",status,dat,len);
////	      if(status!=MI_OK) return;
//						
//			}
			if(j == 5) 
			{
				parameter1 = 0x4b;
				parameter2 = 0x64;
				parameter3 = 0x19;
				parameter4 = parameter1 + parameter2 + parameter3;
				dat[0] = parameter1;	
				dat[1] = parameter2;
				dat[2] = parameter3;
				dat[3] = parameter4;		 
				status = FSV9563_WriteSingleBlockV(0x4b,dat,len);
				printStatus("Write_4b",status,NULL,0);
				if(status!=MI_OK) return ;
				
				dat[0] = 0xa5;	
				dat[1] = 0x5b;
				dat[2] = 0x00;
				dat[3] = 0x00;		 
				status = FSV9563_WriteSingleBlockV(0x4a,dat,len);
				printStatus("Write_4a",status,NULL,0);
				if(status!=MI_OK) return ;	
			}
			if(j == 6)
			{
				dat[0] = 0xa4;	
				dat[1] = 0x5c;
				dat[2] = 0x00;
				dat[3] = 0x00;		 
				status = FSV9563_WriteSingleBlockV(0x4a,dat,len);
				printStatus("Write_4a",status,NULL,0);
				if(status!=MI_OK) return ;
				
				delay_ms(300);
				
			  status = FSV9563_ReadSingleBlockV(uid,0x4b,dat,&len);
				printStatus("ReadBlock_4b",status,dat,len);
				if(status!=MI_OK) return;
			}
		}
    if(i==3)
    {
					FSV9563_FieldOff();
			    delay_ms(10);
			    i=2;
		}			
//	 status = FSV9563_Unlock();
//	 printStatus("Unlock",status,NULL,0);
//	 if(status!=MI_OK) return;
//	 
//	 status = FSV9563_GetLockStatus();
//		printStatus("check",status,NULL,0);
//		if(status!=MI_OK) return;
	 
//  }while(1);
 

//	 delay_ms(1000);
//	 for(i=0;i<3;i++)
//	 {
//		 delay_ms(100);
//		status = FSV9563_GetLockStatus();
//		printStatus("check",status,&flag,1);
//		if(status!=MI_OK) return;
//		 if(flag==0)
//			 break;
//	 }
//	if(flag==0)
//	{
//		status = FSV9563_EnableLock();	
//		printStatus("open",status,NULL,0);
//		if(status!=MI_OK) return;
	
	
	
//	do
//	{
//		FSV9563_FieldOff();
//		delay_ms(10);
//		FSV9563_FieldOn();
//		LED_1;
//		delay_ms(10);
//		status = FSV9563_PcdInventoryV(&DsfId,uid);
//		if(status==MI_OK)
//			LED_0;
//		else 
//			break;
//	}while(status==MI_OK);
   
} 

void FSV9563_ISO15693(void)
 {
	s8 status;
	u8 DsfId,uid[8];
	u8 dat[16],len;
	u8 flag;
	FSV9563_FieldOff();
	delay_ms(10);
	FSV9563_FieldOn();
	PDOWN_1;
	delay_ms(1);
	PDOWN_0;
	delay_ms(5);
	FSV9563_PcdConfigISOType('V');
	delay_ms(10);

	///////////////////////////////////Type V/ISO15693
	status = FSV9563_PcdInventoryV(&DsfId,uid);
	printStatus("Inventory",status,uid,8);
	if(status!=MI_OK) return;
	
	delay_ms(1000);
	
	status = FSV9563_SelectV(uid);
	printStatus("Select",status,NULL,0);
	if(status!=MI_OK) return;
	
	status = FSV9563_ReadSingleBlockV(uid,05,dat,&len);
	printStatus("ReadBlock",status,dat,len);
	if(status!=MI_OK) return;
	


	do
	{
		FSV9563_FieldOff();
		delay_ms(10);
		FSV9563_FieldOn();
		LED_1;
		delay_ms(10);
		status = FSV9563_PcdInventoryV(&DsfId,uid);
		if(status==MI_OK)
			LED_1;
		else 
			break;
	}while(status==MI_OK);
}
//////////////////////////////////////////////////////
u16 nPollCount;
s8 FSV9563_InventoryExV(u8 *pmask,u8 maskBitLen,u8 *pDsfId,u8 *pUID)
{
	s8 status; 
	const u8 BITMASK[]={0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff};
	u8 maskByteLen = (maskBitLen%8)?((maskBitLen >> 3) + 1):(maskBitLen >> 3);
	struct TranSciveBuffer ComData,*pi= &ComData;
	
	nPollCount++;
	FSV9563_WriteReg(rRegT0ReloadHi,0x00); 	//2196/fc 0x0080
	FSV9563_WriteReg(rRegT0ReloadLo,0x80); 	//0x94
	FSV9563_WriteReg(rRegT1ReloadHi,0x02); 	//0x0232
	FSV9563_WriteReg(rRegT1ReloadLo,0x22);	//timerout ~= 10ms
	ComData.Command = FSV9563_Transceive;
	ComData.Length  = 3+maskByteLen;
	ComData.Data[0] = 0x26;	//flags:NBSLOTS,no AFI
	ComData.Data[1] = 0x01;	//invetory
	//ComData.Data[2] = 0;	//afi
	ComData.Data[2] = maskBitLen;	//masklen
	memcpy(&ComData.Data[3], pmask, maskByteLen);
	ComData.Data[2+maskByteLen] &= BITMASK[(maskBitLen-1)&0x07];
	status = FSV9563_PcdComTransceive(pi);
	if (status == MI_OK)
	{
		if(ComData.Length == 0x50)
		{
			*pDsfId = ComData.Data[1];
			memcpy(pUID, &ComData.Data[2], 8);
		}
		else
			status = MI_VALERR;
	}
	return status;
}
s8 FSV9563_InventoryV(u8 *pMask,u8 maskBitLen,u8 *pDsfId,u8 *pUID,u8 *pMore)
{
	s8 status; 
	u8 TimeOutAllowed=0,aMaskBuff[8];
	u8 maskByteLen = (maskBitLen&0x07)?((maskBitLen >> 3) + 1):(maskBitLen >> 3);
	//u8 maskByteLen  = ((maskBitLen + 7) & ~(7));
	*pMore=0;
	memset(aMaskBuff, 0, sizeof(aMaskBuff));  
  memcpy(aMaskBuff, pMask, maskByteLen);  

	FSV9563_WriteReg(rRegTxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegRxCrcPreset,0x7B); 
	FSV9563_WriteReg(rRegTxDataNum,0x08); 
	
	FSV9563_WriteReg(rRegTXWaitCtrl,0x88);	
	FSV9563_WriteReg(rRegTxWaitLo,0xA9);	
	
	status = FSV9563_InventoryExV(pMask,maskBitLen,pDsfId,pUID);
	while(maskBitLen<64)
	{
		switch(status)
		{
			case MI_OK:
				maskBitLen = 64;
				break;
			case MI_NOTAGERR:
				if(TimeOutAllowed)
				{
					aMaskBuff[(u8)(maskBitLen - 1) >> 3] = aMaskBuff[(u8)(maskBitLen - 1) >> 3] ^ (u8)(0x01 << (u8)((maskBitLen - 1) &0x07));
					TimeOutAllowed = 0;
					break;
				}
				else
					return status;
			case MI_COLLERR:
				
			case MI_FRAMINGERR:
				*pMore=1;
				maskBitLen++;
				TimeOutAllowed = 1;
				break;
			default:
				return status;
		}
		if(maskBitLen<64)
			status = FSV9563_InventoryExV(aMaskBuff,maskBitLen,pDsfId,pUID);
	}
	status = FSV9563_StayQuiet(pUID);
	return status;
}
//polling for multi ISO15693 card
void FSV9563_PcdPollingV(void)
{
	s8 status;
//	u8 i;
	u8 aTempUid[8],uid[8],DsfId,more,tags=0;;
	
	FSV9563_PcdConfigISOType('V');
	nPollCount=0;
	delay_ms(10);		
	memset(aTempUid, 0x00, 0x08);
	do{
		status = FSV9563_InventoryV(aTempUid,0,&DsfId,uid,&more);
		if(status==MI_OK)
		{
			tags++;
//			printf("uid:");
//			for(i=0;i<8;i++)
//				printf(" %02X",uid[i]);
//			printf("\n");
		}
		else
		{
			printf("err:%d\n",status);
			break;
		}
	}while(more);
	printf("tags:%d,%d\n",tags,nPollCount);
	FSV9563_FieldOff();
}
//////////////////////////////////////////////////////
//mode 0:calibration; 1:detection
s8 FSV9563_CMD_Lpcd()
{
	struct TranSciveBuffer ComData;
	s8 status = MI_OK;
	u8 backAna,temp;

	FSV9563_WriteReg(rRegT3ReloadHi,0x01);		//0x1f	//powerdown 20us
	FSV9563_WriteReg(rRegT3ReloadLo,0x0F);		//0x20
	FSV9563_WriteReg(rRegT4ReloadHi,0x14);		//0x24: 0x14 0	//poweron 300ms
	FSV9563_WriteReg(rRegT4ReloadLo,0xA0);		//0x25: 0xA0 5

	
	if(mode)
		FSV9563_WriteReg(rRegT4Control,0xDC);			//0x23	//start T4 ,0x1C dc
	else
		FSV9563_WriteReg(rRegT4Control,0xF4);			//0x23	//start T4 ,0x34 f4
	
	
	  FSV9563_WriteReg(rRegLPCD_Q_Result,0);		//0x43	//clear LPCD result

	FSV9563_SetBitMask(rRegRcv,0x40);					//0x38	//set Mix2Adc bit  //接收模式LPCD
	backAna = FSV9563_ReadReg(rRegRxAna);		//0x39	//Backup current RxAna setting
	FSV9563_WriteReg(rRegRxAna,0x03);

	while(1)
	{
		temp = FSV9563_ReadReg(rRegT4Control);		//0x23 //wait until T4 is started
		if(temp&0x80) break;
	}

	ComData.Command = 0x81;		//bStandby 0x81
	ComData.Length = 0;
	status = FSV9563_Command_Int(&ComData);

	FSV9563_WriteReg(rRegRxAna,backAna);	//0x39	//restore RxAna setting
	FSV9563_ClearBitMask(rRegRcv,0x40);		//0x38	//clear Mix2Adc bit
	FSV9563_SetRawRC(rRegT4Control,0x7f,0x40);			//0x23	//stop T4
	
	FSV9563_WriteReg(rRegCommand,FSV9563_Idle);
	FSV9563_WriteReg(rRegFIFOControl,0xB0);	//flush fifo
	if(mode)
	{
		temp = FSV9563_ReadReg(rRegIRQ1);
		if(temp&0x20)	//PHHAL_HW_FSV9563_BIT_LPCDIRQ
		{
			FSV9563_WriteReg(rRegLPCD_Q_Result,0x40);		//0x43	//clear LPCD interrupt
		}
		else
			status = MI_ERR;
	}
	return status;
}

s8 FSV9563_Lpcd_Calib(u8 *pI,u8 *pQ)
{
	s8 status;
	FSV9563_WriteReg(rRegLPCD_QMin,0xc0);
	FSV9563_WriteReg(rRegLPCD_QMax,0xff);
	FSV9563_WriteReg(rRegLPCD_IMin,0xc0);  //IMax 0x3f
	mode =0;
	status = FSV9563_CMD_Lpcd();
	*pI = FSV9563_ReadReg(rRegLPCD_I_Result);
	*pQ = FSV9563_ReadReg(rRegLPCD_Q_Result);
	return status;
}

s8 FSV9563_Lpcd_Det(u8 ValueI,u8 ValueQ)
{
	s8 status;
	u8 bQMin,bQMax,bIMin,bIMax;
	u8 reg,temp,n,m;
	bQMin= bQMax = ValueQ;
	bIMin= bIMax = ValueI;
	if(bQMax < 0x3FU) bQMax++;
  if(bIMax < 0x3FU) bIMax++;
  if(bQMin > 0U) bQMin--;
  if(bIMin > 0U) bIMin--;
	reg = bQMin | (uint8_t)((bIMax & 0x30U) << 2U);
	FSV9563_WriteReg(rRegLPCD_QMin,reg);
	reg = bQMax | (uint8_t)((bIMax & 0x0CU) << 4U);
	FSV9563_WriteReg(rRegLPCD_QMax,reg);
	reg = bIMin | (uint8_t)((bIMax & 0x03U) << 6U);
	FSV9563_WriteReg(rRegLPCD_IMin,reg);
	mode = 1;
		
//	printf(" rRegLPCD_QMin =%02X,rRegLPCD_QMax =%02X, rRegLPCD_IMin =%02X \n",temp,n,m);	
	
	status = FSV9563_CMD_Lpcd();
	return status;
}
